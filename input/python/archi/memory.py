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
# @brief Python module: Representation of memory consisting of multiple blocks with pointer support

import ctypes as c


class CValue:
    """Wrapper around a ctypes value with a callback function and optional attributes.
    """
    def __init__(self, value, /, callback: "Callable" = None, **attributes):
        """Initialize a ctypes value wrapper.
        """
        import types

        if callback is not None and not isinstance(callback, types.FunctionType):
            raise TypeError("Callback must be a function or None")

        if isinstance(value, str):
            value = c.create_string_buffer(value.encode())
        elif isinstance(value, bytes):
            value = c.create_string_buffer(value)

        self._object = value
        self._callback = callback
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

    def object(self):
        """Obtain the original value object.
        """
        return self._object

    def callback(self) -> "Callable":
        """Obtain the callback.

        This callback function is called on the copy when the value is copied to the memory,
        after all memory blocks have been assigned addresses.
        It is needed to write data structures containing pointers to other structures.
        """
        return self._callback

    def attributes(self) -> "dict":
        """Obtain dictionary of additional attributes.
        """
        return self._attributes

    def num_elements(self) -> "int":
        """Get the number of value elements.
        """
        return self._num_of

    def element_size(self) -> "int":
        """Get the size of a value element in bytes.
        """
        return self._size

    def element_alignment(self) -> "int":
        """Get the alignment requirement of a value element in bytes.
        """
        return self._alignment


class MemoryBlock:
    """Representation of a continuous memory block.
    """
    def __init__(self, value: "CValue", /):
        """Initialize a memory block.
        """
        if not isinstance(value, CValue):
            raise TypeError("Value object must be of type CValue")

        self._value = value

        self._owner = None
        self._address = None
        self._object = None

    def value(self) -> "CValue":
        """Obtain the original value.
        """
        return self._value

    def size(self) -> "int":
        """Obtain block size in bytes.
        """
        return c.sizeof(self._value.object())

    def alignment(self) -> "int":
        """Obtain block alignment requirement in bytes.
        """
        return c.alignment(self._value.object())

    def owner(self) -> "Memory":
        """Obtain memory object owning this block.
        """
        return self._owner

    def address(self) -> "int":
        """Obtain actual block address in the memory.
        """
        return self._address

    def object(self):
        """Obtain actual block object in the memory.
        """
        return self._object


class Memory:
    """Representation of memory consisting of relocatable memory blocks.
    """
    def __init__(self, header: "MemoryBlock" = None, blocks: "list[MemoryBlock]" = []):
        """Initialize a memory object.
        """
        self._header = None
        self._blocks = []

        self.set_header_block(header)
        self.set_blocks(blocks)

    def header_block(self) -> "MemoryBlock":
        """Obtain the header block of the memory.
        """
        return self._header

    def set_header_block(self, header: "MemoryBlock"):
        """Set the header block of the memory.
        """
        if header is self._header:
            return

        if header is not None and not isinstance(header, MemoryBlock):
            raise TypeError("Memory header object must be of type MemoryBlock")
        elif header is not None and header._owner is not None:
            raise RuntimeError("Memory block is already owned by another Memory object")
        elif header in self._blocks:
            raise RuntimeError("Header block cannot be in the list of regular blocks")

        if self._header is not None:
            self._header._owner = None

        self._header = header
        self._header._owner = self

        self._reset()

    def blocks(self) -> "list[MemoryBlock]":
        """Obtain the list of blocks of the memory.
        """
        return self._blocks.copy()

    def set_blocks(self, blocks: "list[MemoryBlock]"):
        """Set the list of blocks of the memory.
        """
        if not isinstance(blocks, list) \
                or not all(isinstance(block, MemoryBlock) for block in blocks):
            raise TypeError("List of blocks must be of type list[MemoryBlock]")
        elif not all(block._owner is None or block._owner is self for block in blocks):
            raise RuntimeError("Some of blocks in the list are owned by another Memory object")
        elif self._header in blocks:
            raise RuntimeError("Header block cannot be in the list of regular blocks")
        elif len(blocks) != len(set(blocks)):
            raise TypeError("List of blocks cannot contain duplicate elements")

        for block in self._blocks:
            block._owner = None

        self._blocks = blocks.copy()

        for block in self._blocks:
            block._owner = self

        self._reset()

    def size(self) -> "int":
        """Obtain the total size of the memory in bytes.
        """
        if self._size is None:
            self._calculate()

        return self._size

    def padding(self) -> "int":
        """Obtain the total number of padding bytes in the memory.
        """
        if self._padding is None:
            self._calculate()

        return self._padding

    def alignment(self) -> "int":
        """Obtain the alignment requirement the memory.
        """
        if self._alignment is None:
            self._calculate()

        return self._alignment

    def pack(self):
        """Reorder the blocks to decrease the memory size.
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

    def fossilize(self, address: "int" = None) -> "bytearray":
        """Serialize the memory to a new byte array.
        """
        if address is not None and not isinstance(address, int):
            raise TypeError("Memory address must be an integer")

        if self.size() == 0:
            return bytearray(0)

        # Create a byte array
        buffer = bytearray(self.size())
        buffer_address = c.addressof(c.c_char.from_buffer(buffer))

        if address is None:
            address = buffer_address

        blocks = [self._header] if self._header is not None else []
        blocks += self._blocks

        # Assign addresses to the blocks and copy their contents to the buffer
        offset = 0

        for block in blocks:
            alignment = block.alignment()
            offset = (offset + (alignment - 1)) & ~(alignment - 1)

            src_object = block.value().object()
            dst_object = type(src_object).from_address(buffer_address + offset)

            c.memmove(c.addressof(dst_object), c.addressof(src_object), block.size())

            block._address = address + offset
            block._object = dst_object

            offset += block.size()

        # Call the block callbacks
        for block in blocks:
            callback = block.value().callback()

            if callback is not None:
                callback(block._object)

        # Reset block addresses
        for block in blocks:
            block._address = None
            block._object = None

        return buffer

    def _reset(self):
        self._size = None
        self._padding = None

    def _calculate(self):
        size = self._header.size() if self._header is not None else 0
        padding = 0
        max_alignment = self._header.alignment() if self._header is not None else 1

        for block in self._blocks:
            alignment = block.alignment()
            max_alignment = max(alignment, max_alignment)

            padded_size = (size + (alignment - 1)) & ~(alignment - 1)

            padding += padded_size - size
            size = padded_size + block.size()

        self._size = size
        self._padding = padding
        self._alignment = max_alignment

