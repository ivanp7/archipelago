 #############################################################################
 # Copyright (C) 2023-2025 by Ivan Podmazov                                  #
 #                                                                           #
 # This file is part of Archipelago.                                         #
 #                                                                           #
 #   Archipelago is free software: you can redistribute it and/or modify it  #
 #   under the terms of the GNU Lesser General Public License as published   #
 #   by the Free Software Foundation, either version 3 of the License, or    #
 #   (at your option) any later version.                                     #
 #                                                                           #
 #   Archipelago is distributed in the hope that it will be useful,          #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of          #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
 #   GNU Lesser General Public License for more details.                     #
 #                                                                           #
 #   You should have received a copy of the GNU Lesser General Public        #
 #   License along with Archipelago. If not, see                             #
 #   <http://www.gnu.org/licenses/>.                                         #
 #############################################################################

# @file
# @brief Representation of memory with pointer support consisting of multiple blocks.

import ctypes as c


class CValue:
    """Wrapper around a ctypes value and optional attributes.
    """
    def __init__(self, value, /, **attributes):
        """Initialize a ctypes value wrapper.
        """
        if isinstance(value, str):
            value = c.create_string_buffer(value.encode())
        elif isinstance(value, bytes):
            value = c.create_string_buffer(value, size=len(value))
        elif isinstance(value, bytearray):
            value = c.create_string_buffer(bytes(value), size=len(value))

        self._value = value
        self._attributes = attributes

        if isinstance(value, c.Array):
            if len(value) == 0:
                raise ValueError("Array of values must not be empty")

            self._num_of = len(value)
            self._size = c.sizeof(value._type_)
            self._alignment = c.alignment(value._type_)
        else:
            self._num_of = 1
            self._size = c.sizeof(value)
            self._alignment = c.alignment(value)

    def value(self):
        """Obtain the original value object.
        """
        return self._value

    def attributes(self) -> 'dict':
        """Obtain dictionary of additional attributes.
        """
        return self._attributes

    def num_elements(self) -> 'int':
        """Get the number of value elements.
        """
        return self._num_of

    def element_size(self) -> 'int':
        """Get the size of a value element in bytes.
        """
        return self._size

    def element_alignment(self) -> 'int':
        """Get the alignment requirement of a value element in bytes.
        """
        return self._alignment

###############################################################################

class MemoryBlock:
    """Representation of a continuous memory block with a callback function.
    """
    def __init__(self, initializer: 'CValue', /, callback: 'types.FunctionType' = None):
        """Initialize a memory block.
        """
        import types

        if not isinstance(initializer, CValue):
            raise TypeError
        elif not isinstance(callback, (type(None), types.FunctionType)):
            raise TypeError

        self._initializer = initializer
        self._callback = callback

        self._reset()

    def initializer(self) -> 'CValue':
        """Obtain the memory initializer object.
        """
        return self._initializer

    def callback(self) -> 'types.FunctionType':
        """Obtain the callback.

        This callback function is called on the copy when the value is copied to the memory,
        after all memory blocks have been assigned addresses.
        It is needed to write data structures containing pointers to other structures.
        """
        return self._callback

    def size(self) -> 'int':
        """Obtain block size in bytes.
        """
        return c.sizeof(self._initializer.value())

    def alignment(self) -> 'int':
        """Obtain block alignment requirement in bytes.
        """
        return c.alignment(self._initializer.value())

    def address(self) -> 'int':
        """Obtain block address.
        """
        return self._address

    def value(self):
        """Obtain actual block value in the memory.
        """
        return self._value

    def _set(self, address: 'int', temp_address: 'int'):
        """Initialize block memory and set the address.
        """
        self._address = address
        self._value = type(self._initializer.value()).from_address(temp_address)

        c.memmove(temp_address, c.addressof(self._initializer.value()), self.size())

    def _reset(self):
        """Copy contents into the buffer and set the memory address.
        """
        self._address = None
        self._value = None

    def _call(self):
        """Invoke the callback.
        """
        if self._callback is not None:
            self._callback(self._value)


class MemoryBlockCache:
    """Storage of memory blocks with memoization support.
    """
    def __init__(self):
        """Initialize a cache.
        """
        self.reset()

    def __getitem__(self, key) -> 'MemoryBlock':
        """Get a memory block from the cache.
        """
        return self._cache[key] if key is not None else None

    def __setitem__(self, key, block: 'MemoryBlock'):
        """Insert a memory block into the cache.
        """
        if key is None:
            raise KeyError
        elif not isinstance(block, MemoryBlock):
            raise TypeError
        elif key in self._cache and self._cache[key] is not block:
            raise KeyError

        self._cache[key] = block

    def __delitem__(self, key):
        """Delete a memory block from the cache.
        """
        del self._cache[key]

    def __contains__(self, item) -> 'bool':
        """Check if an item is in the cache.
        """
        if item is None:
            return True
        elif isinstance(item, MemoryBlock):
            return item in self._cache.values()
        else:
            return item in self._cache

    def __iter__(self):
        """Return an iterator.
        """
        return iter(self._cache)

    def blocks(self) -> 'list[MemoryBlock]':
        """Obtain list of all stored memory blocks.
        """
        return list(set(self._cache.values()))

    def reset(self):
        """Reset the cache.
        """
        self._cache = {}

###############################################################################

class MemoryCluster:
    """Representation of memory cluster consisting of a list of relocatable memory blocks,
    along with a separate non-relocatable header block.
    """
    def __init__(self, header: 'MemoryBlock' = None, blocks: 'list[MemoryBlock]' = None):
        """Initialize a memory cluster.
        """
        self._header = None
        self._blocks = []

        self.set_header_block(header)
        if blocks is not None:
            self.set_blocks(blocks)

    def header_block(self) -> 'MemoryBlock':
        """Obtain the header block of the memory cluster.
        """
        return self._header

    def set_header_block(self, header: 'MemoryBlock', /):
        """Set the header block of the memory cluster.
        """
        if header is self._header:
            return

        if not isinstance(header, (type(None), MemoryBlock)):
            raise TypeError
        elif header in self._blocks:
            raise RuntimeError("Header block cannot be in the list of regular blocks")

        self._header = header

        self._reset()

    def blocks(self) -> 'list[MemoryBlock]':
        """Obtain the list of blocks of the memory cluster.
        """
        from types import MappingProxyType

        return MappingProxyType(self._blocks)

    def set_blocks(self, blocks: 'list[MemoryBlock]', /):
        """Set the list of blocks of the memory cluster.
        """
        if not isinstance(blocks, list):
            raise TypeError
        elif not all(isinstance(block, MemoryBlock) for block in blocks):
            raise TypeError
        elif self._header in blocks:
            raise ValueError("Header block cannot be in the list of regular blocks")
        elif len(blocks) != len(set(blocks)):
            raise ValueError("List of blocks cannot contain duplicates")

        self._blocks = blocks.copy()

        self._reset()

    def size(self) -> 'int':
        """Obtain the total size in bytes of the memory cluster.
        """
        if self._size is None:
            self._recalculate()

        return self._size

    def alignment(self) -> 'int':
        """Obtain the alignment requirement of the memory cluster.
        """
        if self._alignment is None:
            self._recalculate()

        return self._alignment

    def padding(self) -> 'int':
        """Obtain the total number of padding bytes in the memory cluster.
        """
        if self._padding is None:
            self._recalculate()

        return self._padding

    def pack(self):
        """Reorder the blocks to decrease the memory padding.
        """
        from functools import cmp_to_key

        # Sort blocks by descending alignment (for equal alignments, by descending size)
        def compare(block1, block2):
            return block2.alignment() - block1.alignment() \
                    if block2.alignment() != block1.alignment() \
                    else block2.size() - block1.size()

        self._blocks.sort(key=cmp_to_key(compare))

        # Pack sorted list of blocks
        blocks = {} # offset -> MemoryBlock

        gaps = []
        end = self._header.size() if self._header is not None else 0

        for block in self._blocks:
            alignment = block.alignment()
            size = block.size()

            # Try to fit the block into one of gaps
            gap_used = None
            new_gaps = []
            for idx, gap in enumerate(gaps):
                gap_addr = gap[0]
                gap_size = gap[1]

                gap_addr_align = (gap_addr + (alignment - 1)) & ~(alignment - 1)
                gap_size_align = gap_size - (gap_addr_align - gap_addr)

                if size <= gap_size_align: # succeed
                    if gap_addr_align > gap_addr:
                        new_gaps.append((gap_addr, gap_addr_align - gap_addr))

                    if size < gap_size_align:
                        new_gaps.append((gap_addr_align + size, gap_size_align - size))

                    gap_used = idx
                    break

            # Insert the block into the dictionary and update gaps
            if gap_used is None:
                address = (end + (alignment - 1)) & ~(alignment - 1)
                if address > end:
                    gaps.append((end, address - end))

                end = address + size
            else:
                address = gap_addr_align
                gaps[gap_used:gap_used+1] = new_gaps

            blocks[address] = block

        # Create the new list of blocks
        self._blocks = [block for _, block in sorted(blocks.items())]

        self._reset()

    def marshal(self, address: 'int', /) -> 'bytearray':
        """Marshal the memory cluster into a memory image
        which can be mapped at the specified address.
        """
        if not isinstance(address, int):
            raise TypeError
        elif address < 0:
            raise ValueError

        if self.size() == 0:
            return bytearray(0)

        # Create a byte array
        buffer = bytearray(self.size())
        temp_address = c.addressof(c.c_char.from_buffer(buffer))

        blocks = [self._header] if self._header is not None else []
        blocks += self._blocks

        # Assign addresses to the blocks and copy their contents to the buffer
        offset = 0

        for block in blocks:
            alignment = block.alignment()
            offset = (offset + (alignment - 1)) & ~(alignment - 1)

            block._set(address + offset, temp_address + offset)

            offset += block.size()

        # Invoke callbacks of the blocks
        for block in blocks:
            block._call()

        # Reset block addresses
        for block in blocks:
            block._reset()

        return buffer

    def _reset(self):
        self._size = None
        self._alignment = None
        self._padding = None

    def _recalculate(self):
        size = self._header.size() if self._header is not None else 0
        max_alignment = self._header.alignment() if self._header is not None else 1
        padding = 0

        for block in self._blocks:
            alignment = block.alignment()
            max_alignment = max(alignment, max_alignment)

            size_with_padding = (size + (alignment - 1)) & ~(alignment - 1)

            padding += size_with_padding - size
            size = size_with_padding + block.size()

        self._size = size
        self._alignment = max_alignment
        self._padding = padding

###############################################################################

class Encoder:
    """The base class for encoders of objects of various types.

    Works as the CValue encoder class.
    """
    @classmethod
    def encode(cls, obj, cache: 'MemoryBlockCache', /) -> 'MemoryBlock':
        """Encode an object.
        """
        if not isinstance(cache, MemoryBlockCache):
            raise TypeError

        if obj is None:
            return None
        elif not cls.check_type(obj):
            raise TypeError

        key = cls._cache_key(obj)

        if key is not None and key in cache:
            return cache[key]

        block = cls._encode(obj, cache)

        if block is not None:
            cache[key if key is not None else block] = block

        return block

    @classmethod
    def check_type(cls, obj) -> 'bool':
        """Check type of an encoded object.
        """
        return isinstance(obj, CValue)

    @classmethod
    def _cache_key(cls, obj):
        """Get a cache key for an object.

        Returns a hashable object or None.
        """
        return obj

    @classmethod
    def _encode(cls, obj, cache: 'MemoryBlockCache', /) -> 'MemoryBlock':
        """Encode an object of a specific type.

        This method is to be implemented in derived classes for specific types.
        """
        return MemoryBlock(obj)

    def __init__(self, obj, /):
        """Initialize an encoder instance with an attached object.
        """
        if not type(self).check_type(obj):
            raise TypeError

        self._obj = obj

    def __call__(self, cache: 'MemoryBlockCache') -> 'MemoryBlock':
        """Encode an attached object.
        """
        return type(self).encode(self.obj(), cache)

    def obj(self):
        """Obtain the attached object.
        """
        return self._obj


class StringEncoder(Encoder):
    """Encoder class for strings.
    """
    @classmethod
    def check_type(cls, obj) -> 'bool':
        """Check type of an encoded object.
        """
        return isinstance(obj, str)

    @classmethod
    def _encode(cls, obj, cache: 'MemoryBlockCache', /) -> 'MemoryBlock':
        """Encode a string.
        """
        return MemoryBlock(CValue(obj))

