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
# @brief Representation of objects in memory.

import ctypes as c


class Object:
    """Immutable representation of an (array) object backed by a single continuous memory block.
    """
    def __init__(self, length: 'int', stride: 'int', alignment: 'int', tag: 'int' = None,
                 refs: 'map[Any, Object]' = None):
        """Initialize an object.
        """
        if not isinstance(length, int) \
                or not isinstance(stride, int) \
                or not isinstance(alignment, int):
            raise TypeError
        elif not isinstance(tag, (type(None), int)):
            raise TypeError
        elif not isinstance(refs, (type(None), map)):
            raise TypeError
        elif refs is not None and not all(
                isinstance(obj, (type(None), Object)) for obj in refs.values()):
            raise TypeError

        if length < 0:
            raise ValueError("Length is negative")
        elif stride <= 0:
            raise ValueError("Stride is non-positive")
        elif (alignment <= 0) or ((alignment & (alignment - 1)) != 0):
            raise ValueError("Alignment requirement is not a power of two")
        elif stride % alignment != 0:
            raise ValueError("Stride is not divisible by alignment requirement")
        elif tag is not None and tag < 0:
            raise ValueError("Opaque data tag is negative")

        self._length = length
        self._stride = stride
        self._alignment = alignment
        self._tag = tag

        self._refs = refs.copy() if refs is not None else {}

        self._reset_address()

    def __contains__(self, key) -> 'bool':
        """Check if there is a reference with the specified key.
        """
        return key in self._refs

    def __getitem__(self, key) -> 'Object':
        """Get a referenced object by key.
        """
        return self._refs[key]

    def ref_map(self) -> 'map[Any, Object]':
        """Get the map of referenced objects.
        """
        from types import MappingProxyType

        return MappingProxyType(self._refs)

    def ref_objects(self) -> 'set[Object]':
        """Get set of all referenced objects.
        """
        objects = set(self._refs.values())
        objects.discard(None)
        return objects

    def tag(self) -> 'int':
        """Get opaque data type tag.
        """
        return self._tag

    def length(self) -> 'int':
        """Get number of data elements.
        """
        return self._length

    def stride(self) -> 'int':
        """Get size of a data element in bytes.
        """
        return self._stride

    def alignment(self) -> 'int':
        """Get alignment requirement of a data element in bytes.
        """
        return self._alignment

    def total_size(self) -> 'int':
        """Get total size of data in bytes.

        This method may be redefined to implement support for additional trailing padding.
        """
        return self.length() * self.stride()

    def total_alignment(self) -> 'int':
        """Get total alignment requirement of data in bytes.

        This method may be redefined to implement support for overaligned data.
        """
        return self.alignment()

    def address(self) -> 'int':
        """Get nominal address of the object in memory.

        This address is to be assigned to pointers in C structures,
        and not to be used to access the real object.
        The real object is accessed through the effective address instead,
        which is only available as 'address' parameter in write() method.
        """
        return self._address

    def to_bytearray(self, address: 'int') -> 'bytearray':
        """Encode the object as a byte array to be mapped at the specified address.

        Parameter 'address' contains the nominal address of the object.
        """
        if not isinstance(address, int):
            raise TypeError
        elif address < 0:
            raise ValueError("Address cannot be negative")
        elif address % self.total_alignment() != 0:
            raise ValueError("Address is not divisible by the alignment requirement")

        # Create the byte array
        buffer = bytearray(self.total_size())

        if self.total_size() != 0:
            # Set nominal address
            self._set_address(address)

            # Write object contents to the buffer
            eff_address = c.addressof(c.c_char.from_buffer(buffer))
            self._write(eff_address)

            # Reset nominal address
            self._reset_address()

        return buffer

    def equivalent_to(self, other: 'Object', /) -> 'bool':
        """Check if the object is equivalent to (replaceable with) another object.
        """
        if not isinstance(other, (type(None), Object)):
            raise TypeError

        if self is other:
            return True
        elif other is None:
            return False

        if self.total_size() != other.total_size():
            return False
        elif self.total_alignment() != other.total_alignment():
            return False

        if self._refs.keys() != other._refs.keys():
            return False

        if isinstance(self, type(other)):
            if not self._equivalent_to(other):
                return False
        elif isinstance(other, type(self)):
            if not other._equivalent_to(self):
                return False
        else:
            return False

        return all(Object.equivalent(obj, other._refs[key])
                   for key, obj in self._refs.items())

    def _set_address(self, address: 'int', /):
        """Set nominal (written to memory) address of the object in memory.
        """
        if not isinstance(address, int):
            raise TypeError
        elif address < 0:
            raise ValueError

        self._address = address

    def _reset_address(self):
        """Reset nominal address of the object in memory.
        """
        self._address = None

    def _equivalent_to(self, other: 'Object', /) -> 'bool':
        """Check if the object is equivalent to (replaceable with) another object.

        This method is to be reimplemented in derived classes.
        """
        return True

    def _write(self, eff_address: 'int', /):
        """Write object to memory.

        This method is to be implemented in derived classes.
        It must write (no more than) `self.total_size()` bytes at the specified address.

        Parameter 'eff_address' contains the effective address of the real object,
        it is to be used to access the modified object.
        """
        raise NotImplementedError

    @staticmethod
    def equivalent(obj1: 'Object', obj2: 'Object', /) -> 'bool':
        """Check if the object is equivalent to (replaceable with) another object.
        """
        if obj1 is None and obj2 is None:
            return True
        elif obj1 is None or obj2 is None:
            return False

        return obj1.equivalent_to(obj2)

    @staticmethod
    def random_address(start: 'int' = 0x7f0000000000,
                       stop: 'int' = 0x800000000000,
                       alignment: 'int' = 0x1000) -> 'int':
        """Generate random address of the specified alignment
        in range [start, stop).
        """
        import random

        if start < 0:
            raise ValueError
        elif start >= stop:
            raise ValueError
        elif (alignment <= 0) or ((alignment & (alignment - 1)) != 0):
            raise ValueError
        elif start % alignment != 0:
            raise ValueError
        elif stop % alignment != 0:
            raise ValueError

        return random.randrange(start, stop) & ~(alignment - 1)

##############################################################################

class ObjectPack(Object):
    """Immutable representation of an object packed
    together with all of it (nested) references.
    """
    def __init__(self, header: 'Object', /):
        """Initialize an object pack.
        """
        if not isinstance(header, (type(None), Object)):
            raise TypeError

        if header is None:
            self._header = None
            self._objects = frozenset()

            super().__init__(length=0, stride=1, alignment=1)
            return

        ############################################
        # Compute set of nested referenced objects #
        ############################################
        objects = set()

        refs = header.ref_objects()
        while refs:
            objects.update(refs)
            nested_refs = set()
            nested_refs.update(obj.ref_objects() for obj in refs)
            refs = nested_refs.difference(objects)
        del nested_refs
        del refs

        objects.discard(header)

        self._header = header
        self._objects = frozenset(objects)

        ################################
        # Find groups of equal objects #
        ################################
        if header.length() == 0:
            header = None # zero size objects are omitted

        self._object_equivalents = {header: set()} if header is not None else {}

        for obj in objects:
            if obj.length() == 0:
                continue # zero size objects are omitted

            for key_obj, eq_obj_set in self._object_equivalents.items():
                if Object.equivalent(obj, key_obj):
                    eq_obj_set.add(obj)
                    break
            else:
                self._object_equivalents[obj] = set()

        #################################################################################
        # Find (sub)optimal ordering of objects, and calculate size, alignment, padding #
        #################################################################################
        self._key_objects = {header: 0} if header is not None else {}

        total_size = header.total_size() if header is not None else 0
        total_alignment = header.total_alignment() if header is not None else 1

        key_objects = list(self._object_equivalents.keys())
        if header is not None:
            key_objects.remove(header) # header is not relocatable, treat it specially

        # Sort objects by descending alignment (for equal alignments, by descending size)
        def compare(obj1, obj2):
            return obj2.total_alignment() - obj1.total_alignment() \
                    if obj2.total_alignment() != obj1.total_alignment() \
                    else obj2.total_size() - obj1.total_size()

        from functools import cmp_to_key
        key_objects.sort(key=cmp_to_key(compare))

        # Pack sorted list of objects
        gaps = [] # free areas as tuples (offset, size)

        for obj in key_objects:
            current_alignment = obj.total_alignment()
            current_size = obj.total_size()

            # Update total alignment
            total_alignment = max(current_alignment, total_alignment)

            # Try to fit the object into one of gaps
            idx_gap_used = None
            new_gaps = []

            for idx, (gap_offset, gap_size) in enumerate(gaps):
                gap_offset_aligned = (gap_offset + (current_alignment - 1)) \
                        & ~(current_alignment - 1)
                gap_size_aligned = gap_size - (gap_offset_aligned - gap_offset)

                if current_size <= gap_size_aligned: # the current object fits
                    if gap_offset_aligned > gap_offset:
                        new_gaps.append((gap_offset, gap_offset_aligned - gap_offset))

                    if current_size < gap_size_aligned:
                        new_gaps.append((gap_offset_aligned + current_size,
                                         gap_size_aligned - current_size))

                    idx_gap_used = idx
                    break

            # Calculate offset, update gaps and total size
            if idx_gap_used is None:
                offset = (total_size + (current_alignment - 1)) & ~(current_alignment - 1)

                if offset > total_size:
                    gaps.append((total_size, offset - total_size))

                total_size = offset + current_size
            else:
                offset = gap_offset_aligned

                gaps[idx_gap_used:idx_gap_used+1] = new_gaps

            # Insert the object into the dictionary
            self._key_objects[obj] = offset

        # Add the final gap to the list of gaps
        current_size = total_size
        total_size = (current_size + (total_alignment - 1)) & ~(total_alignment - 1)

        if total_size > current_size:
            gaps.append((current_size, total_size - current_size))

        # Calculate total padding
        self._padding = sum(gap_size for _, gap_size in gaps)

        # Sort the dictionary of objects by offsets (optional, not strictly needed)
        # self._key_objects = {obj: offset for obj, offset
        #                      in sorted(self._key_objects.items(), key=lambda x: x[1])}

        ###########################
        # Call parent constructor #
        ###########################
        if total_size != 0:
            super().__init__(length=1, stride=total_size, alignment=total_alignment,
                             tag=header.tag())
        else:
            super().__init__(length=0, stride=1, alignment=total_alignment,
                             tag=header.tag())

    def header_object(self) -> 'Object':
        """Get the header object.
        """
        return self._header

    def packed_objects(self) -> 'frozenset[Object]':
        """Get the set of packed objects.
        """
        return self._objects

    def total_padding(self) -> 'int':
        """Get the total number of padding bytes in the object pack.
        """
        return self._padding

    def _equivalent_to(self, other: 'ObjectPack', /) -> 'bool':
        """Compare object packs for equivalence.
        """
        return isinstance(other, ObjectPack) \
                and Object.equivalent(self.header_object(), other.header_object())

    def _write(self, eff_address: 'int', /):
        """Write object pack to memory.
        """
        # Set nominal addresses of key objects and all objects equal to them
        for obj, offset in self._key_objects.items():
            address = self.address() + offset
            obj._set_address(address)

            for eq_obj in self._object_equivalents[obj]:
                eq_obj._set_address(address) # equivalent objects have the same address

        # Write key objects to memory at their respective offsets
        for obj, offset in self._key_objects.items():
            obj._write(eff_address + offset)

        # Reset nominal addresses of objects
        for obj in self._key_objects.keys():
            obj._reset_address()

            for eq_obj in self._object_equivalents[obj]:
                eq_obj._reset_address()

##############################################################################

class CObject(Object):
    """Wrapper for a ctypes object.
    """
    def __init__(self, cobject, /, tag: 'int' = None,
                 refs: 'map[Any, Object]' = None):
        """Initialize a ctypes object wrapper.
        """
        self._buffer = bytes(cobject)
        self._type = type(cobject)

        if refs is not None:
            for key in refs.keys():
                if isinstance(key, tuple):
                    if len(key) == 0:
                        raise ValueError
                    elif not all(isinstance(elt, (str, int)) for elt in key):
                        raise TypeError
                elif not isinstance(key, (str, int)):
                    raise TypeError

        if isinstance(cobject, c.Array):
            self._type1 = cobject._type_
            super().__init__(length=len(cobject), stride=c.sizeof(cobject._type_),
                             alignment=c.alignment(cobject._type_), tag=tag, refs=refs)
        else:
            self._type1 = self._type
            super().__init__(length=1, stride=c.sizeof(cobject),
                             alignment=c.alignment(cobject), tag=tag, refs=refs)

    def buffer(self) -> 'bytes':
        """Get managed bytes buffer.
        """
        return self._buffer

    def c_type(self):
        """Get type of the original ctypes object.
        """
        return self._type

    def c_element_type(self):
        """Get type of an element of the original ctypes object.
        """
        return self._type1

    def c_object(self):
        """Get copy of the original ctypes object.
        """
        return self.c_type().from_buffer_copy(self.buffer())

    def _equivalent_to(self, other: 'CObject', /) -> 'bool':
        """Check equivalence with another C object.
        """
        if not isinstance(other, CObject):
            return False

        return self.buffer() == other.buffer()

    def _write(self, eff_address: 'int', /):
        """Write object to memory.
        """
        from .ctypes import archi_pointer_t

        # Initialize the memory
        c.memmove(eff_address, self._buffer, len(self._buffer))

        # The memory is now a valid object
        if self._refs:
            c_object = self._type.from_address(eff_address)

        # Write pointers to referenced objects
        for ref_key, ref_obj in self._refs:
            if ref_obj is None:
                continue

            obj = c_object
            key = ref_key

            if isinstance(key, tuple):
                # Descend to the target subobject
                while len(key) > 1:
                    if isinstance(key[0], str): # structure field
                        obj = getattr(obj, key[0])
                    else: # array element
                        obj = obj[key[0]]

                    key = key[1:]

                key = key[0]

            if isinstance(key, str): # structure field
                target = getattr(obj, key)
            else: # array element
                target = obj[key]

            if isinstance(target, archi_pointer_t):
                target.assign(ref_obj)
            else:
                # Cast the address to the target type if necessary
                if isinstance(target, (c.c_void_p, c.c_char_p, c.c_wchar_p)):
                    address = ref_obj.address()
                else:
                    address = c.cast(ref_obj.address(), type(target))

                # Write the pointer address to the structure field / array element
                if isinstance(key, str): # structure field
                    setattr(obj, key, address)
                else: # array element
                    obj[key] = address


class PlainCObject(CObject):
    """Wrapper for a ctypes object without pointers.
    """
    def __init__(self, cobject, /, tag: 'int' = None):
        """Initialize a plain ctypes object wrapper.
        """
        super().__init__(cobject, tag=tag)

    @staticmethod
    def from_bytes(buf: 'bytes', /) -> 'PlainCObject':
        """Initialize a wrapper for a raw byte buffer.
        """
        if not isinstance(s, (type(None), bytes)):
            raise TypeError

        if buf is None:
            return None

        return PlainCObject(c.create_string_buffer(buf, len(buf)))


class String(PlainCObject):
    """Wrapper for a C string.
    """
    def __init__(self, string: 'str', /, encoding=None):
        """Initialize a C string wrapper.
        """
        if not isinstance(string, str):
            raise TypeError

        self._string = string

        super().__init__(c.create_string_buffer(string.encode() if encoding is None
                                                else string.encode(encoding=encoding)))

    def string(self) -> 'str':
        """Get the original string.
        """
        return self._string

    @staticmethod
    def nullable(string: 'str', /, encoding=None) -> 'String':
        """Construct a C string wrapper.
        """
        if string is None:
            return None

        return String(string, encoding=encoding)

##############################################################################

class KeyValueList(CObject):
    """Representation of a key-value list node.
    """
    def __init__(self, key: 'String', value: 'Object', next_node: 'KeyValueList' = None):
        """Initialize a key-value list node.
        """
        if not isinstance(key, (type(None), String)) \
                or not isinstance(value, (type(None), Object)) \
                or not isinstance(next_node, (type(None), KeyValueList)):
            raise TypeError

        from .ctypes import archi_kvlist_t
        super().__init__(archi_kvlist_t(),
                         refs={'key': key, 'value': value, 'next': next_node})

    def next_node(self) -> 'KeyValueList':
        """Get next node of the list.
        """
        return self['next']

    def key(self) -> 'String':
        """Get node key.
        """
        return self['key']

    def value(self) -> 'Object':
        """Get node value.
        """
        return self['value']

    @staticmethod
    def construct(key_object_tuples: 'list', /) -> 'KeyValueList':
        """Construct a key-value list object from a list of (key, value_obj) tuples.
        """
        if not key_object_tuples:
            return None

        key_obj = {key for key, _ in key_object_tuples}
        key_obj = {key: String.nullable(key) for key in key_obj}

        kvlist_node = None
        for key, obj in reversed(key_object_tuples):
            kvlist_node = KeyValueList(key_obj[key], obj, kvlist_node)

        return kvlist_node

##############################################################################

class ExecInputFileHeader(CObject):
    """Representation of header of an input file for the executable.
    """
    def __init__(self, contents: 'KeyValueList', /):
        """Initialize an input file header.
        """
        if not isinstance(contents, (type(None), KeyValueList)):
            raise TypeError

        from .ctypes import archi_exe_input_file_header_t
        super().__init__(archi_exe_input_file_header_t(),
                         refs={'contents': contents})

    def contents(self):
        """Get list of contents of the file.
        """
        return self['contents']


class ExecInputFile(ObjectPack):
    """Representation of an input file for the executable.
    """
    def __init__(self, header: 'ExecInputFileHeader', /):
        """Initialize an input file.
        """
        if not isinstance(header, ExecInputFileHeader):
            raise TypeError

        super().__init__(header)

    def _write(self, eff_address: 'int', /):
        """Write input file to memory.
        """
        super()._write(eff_address)

        from .ctypes import archi_exe_input_file_header_t

        header = archi_exe_input_file_header_t.from_address(eff_address)

        header.header.addr = self.address()
        header.header.size = self.total_size() - c.sizeof(header.header)

##############################################################################

class ContextRegistryOpData_delete(CObject):
    """Context registry operation data: delete a context.
    """
    def __init__(self, key: 'String'):
        if not isinstance(key, String):
            raise TypeError

        from .ctypes import archi_context_registry_op_data__delete_t
        super().__init__(archi_context_registry_op_data__delete_t(),
                         refs={'key': key})


class ContextRegistryOpData_alias(CObject):
    """Context registry operation data: create a context alias.
    """
    def __init__(self, alias_key: 'String', origin_key: 'String'):
        if not isinstance(alias_key, String) \
                or not isinstance(origin_key, String):
            raise TypeError

        from .ctypes import archi_context_registry_op_data__alias_t
        super().__init__(archi_context_registry_op_data__alias_t(),
                         refs={'alias_key': alias_key, 'origin_key': origin_key})


class ContextRegistryOpData_create_as(CObject):
    """Context registry operation data: create a context using interface of another context.
    """
    def __init__(self, context_key: 'String',
                 context_init_params_list_key: 'String',
                 context_init_params_list: 'KeyValueList',
                 instance_key: 'String'):
        if not isinstance(context_key, String) \
                or not isinstance(context_init_params_list_key, (type(None), String)) \
                or not isinstance(context_init_params_list, (type(None), KeyValueList)) \
                or not isinstance(instance_key, String):
            raise TypeError

        from .ctypes import archi_context_registry_op_data__create_as_t
        super().__init__(archi_context_registry_op_data__create_as_t(),
                         refs={('context', 'key'): context_key,
                               ('context', 'init_params', 'list_key'): context_init_params_list_key,
                               ('context', 'init_params', 'list'): context_init_params_list,
                               'instance_key': instance_key})


class ContextRegistryOpData_create_from(CObject):
    """Context registry operation data: create a context using interface obtained from another context slot.
    """
    def __init__(self, context_key: 'String',
                 context_init_params_list_key: 'String',
                 context_init_params_list: 'KeyValueList',
                 slot_name: 'String',
                 slot_indices: 'PlainCObject'):
        if not isinstance(context_key, String) \
                or not isinstance(context_init_params_list_key, (type(None), String)) \
                or not isinstance(context_init_params_list, (type(None), KeyValueList)) \
                or not isinstance(slot_name, (type(None), String)) \
                or not isinstance(slot_indices, (type(None), PlainCObject)):
            raise TypeError
        elif slot_indices is not None and slot_indices.c_element_type() != c.c_ssize_t:
            raise TypeError

        from .ctypes import archi_context_registry_op_data__create_from_t

        cobject = archi_context_registry_op_data__create_from_t()
        if slot_indices is not None:
            cobject.slot.num_indices = slot_indices.length()

        super().__init__(cobject,
                         refs={('context', 'key'): context_key,
                               ('context', 'init_params', 'list_key'): context_init_params_list_key,
                               ('context', 'init_params', 'list'): context_init_params_list,
                               ('slot', 'name'): slot_name,
                               ('slot', 'index'): slot_indices})


class ContextRegistryOpData_call(CObject):
    """Context registry operation data: invoke context call.
    """
    def __init__(self, target_key: 'String',
                 target_slot_name: 'String', target_slot_indices: 'PlainCObject',
                 call_params_list_key: 'String', call_params_list: 'KeyValueList'):
        if not isinstance(target_key, String) \
                or not isinstance(target_slot_name, (type(None), String)) \
                or not isinstance(target_slot_indices, (type(None), PlainCObject)) \
                or not isinstance(call_params_list_key, (type(None), String)) \
                or not isinstance(call_params_list, (type(None), KeyValueList)):
            raise TypeError
        elif target_slot_indices is not None \
                and target_slot_indices.c_element_type() != c.c_ssize_t:
            raise TypeError

        from .ctypes import archi_context_registry_op_data__call_t

        cobject = archi_context_registry_op_data__call_t()
        if target_slot_indices is not None:
            cobject.target.slot.num_indices = target_slot_indices.length()

        super().__init__(cobject,
                         refs={('target', 'key'): target_key,
                               ('target', 'slot', 'name'): target_slot_name,
                               ('target', 'slot', 'index'): target_slot_indices,
                               ('call_params', 'list_key'): call_params_list_key,
                               ('call_params', 'list'): call_params_list})


class ContextRegistryOpData_set(CObject):
    """Context registry operation data: set context slot to value.
    """
    def __init__(self, target_key: 'String',
                 target_slot_name: 'String', target_slot_indices: 'PlainCObject',
                 value: 'Object'):
        if not isinstance(target_key, String) \
                or not isinstance(target_slot_name, (type(None), String)) \
                or not isinstance(target_slot_indices, (type(None), PlainCObject)) \
                or not isinstance(value, (type(None), Object)):
            raise TypeError
        elif target_slot_indices is not None \
                and target_slot_indices.c_element_type() != c.c_ssize_t:
            raise TypeError

        from .ctypes import archi_context_registry_op_data__set_t

        cobject = archi_context_registry_op_data__set_t()
        if target_slot_indices is not None:
            cobject.target.slot.num_indices = target_slot_indices.length()

        super().__init__(cobject,
                         refs={('target', 'key'): target_key,
                               ('target', 'slot', 'name'): target_slot_name,
                               ('target', 'slot', 'index'): target_slot_indices,
                               'value': value})


class ContextRegistryOpData_assign(CObject):
    """Context registry operation data: set context slot to value of another context slot.
    """
    def __init__(self, target_key: 'String',
                 target_slot_name: 'String', target_slot_indices: 'PlainCObject',
                 source_key: 'String',
                 source_slot_name: 'String', source_slot_indices: 'PlainCObject'):
        if not isinstance(target_key, String) \
                or not isinstance(target_slot_name, (type(None), String)) \
                or not isinstance(target_slot_indices, (type(None), PlainCObject)) \
                or not isinstance(source_key, String) \
                or not isinstance(source_slot_name, (type(None), String)) \
                or not isinstance(source_slot_indices, (type(None), PlainCObject)):
            raise TypeError
        elif target_slot_indices is not None \
                and target_slot_indices.c_element_type() != c.c_ssize_t:
            raise TypeError
        elif source_slot_indices is not None \
                and source_slot_indices.c_element_type() != c.c_ssize_t:
            raise TypeError

        from .ctypes import archi_context_registry_op_data__assign_t

        cobject = archi_context_registry_op_data__assign_t()
        if target_slot_indices is not None:
            cobject.target.slot.num_indices = target_slot_indices.length()
        if source_slot_indices is not None:
            cobject.source.slot.num_indices = source_slot_indices.length()

        super().__init__(cobject,
                         refs={('target', 'key'): target_key,
                               ('target', 'slot', 'name'): target_slot_name,
                               ('target', 'slot', 'index'): target_slot_indices,
                               ('source', 'key'): source_key,
                               ('source', 'slot', 'name'): source_slot_name,
                               ('source', 'slot', 'index'): source_slot_indices})


class ContextRegistryOpData_assign_call(CObject):
    """Context registry operation data: set context slot to result of another context call.
    """
    def __init__(self, target_key: 'String',
                 target_slot_name: 'String', target_slot_indices: 'PlainCObject',
                 source_key: 'String',
                 source_slot_name: 'String', source_slot_indices: 'PlainCObject',
                 call_params_list_key: 'String', call_params_list: 'KeyValueList'):
        if not isinstance(target_key, String) \
                or not isinstance(target_slot_name, (type(None), String)) \
                or not isinstance(target_slot_indices, (type(None), PlainCObject)) \
                or not isinstance(source_key, String) \
                or not isinstance(source_slot_name, (type(None), String)) \
                or not isinstance(source_slot_indices, (type(None), PlainCObject)) \
                or not isinstance(call_params_list_key, (type(None), String)) \
                or not isinstance(call_params_list, (type(None), KeyValueList)):
            raise TypeError
        elif target_slot_indices is not None \
                and target_slot_indices.c_element_type() != c.c_ssize_t:
            raise TypeError
        elif source_slot_indices is not None \
                and source_slot_indices.c_element_type() != c.c_ssize_t:
            raise TypeError

        from .ctypes import archi_context_registry_op_data__assign_call_t

        cobject = archi_context_registry_op_data__assign_call_t()
        if target_slot_indices is not None:
            cobject.target.slot.num_indices = target_slot_indices.length()
        if source_slot_indices is not None:
            cobject.source.slot.num_indices = source_slot_indices.length()

        super().__init__(cobject,
                         refs={('target', 'key'): target_key,
                               ('target', 'slot', 'name'): target_slot_name,
                               ('target', 'slot', 'index'): target_slot_indices,
                               ('source', 'key'): source_key,
                               ('source', 'slot', 'name'): source_slot_name,
                               ('source', 'slot', 'index'): source_slot_indices,
                               ('call_params', 'list_key'): call_params_list_key,
                               ('call_params', 'list'): call_params_list})

##############################################################################

class ContextRegistryOpData_create_parameters(CObject):
    """Context registry operation data: create a parameters list context.
    """
    def __init__(self, key: 'String',
                 init_params_list_key: 'String', init_params_list: 'KeyValueList'):
        if not isinstance(key, String) \
                or not isinstance(init_params_list_key, (type(None), String)) \
                or not isinstance(init_params_list, (type(None), KeyValueList)):
            raise TypeError

        from .ctypes import archi_context_registry_op_data__create_parameters_t
        super().__init__(archi_context_registry_op_data__create_parameters_t(),
                         refs={'key': key,
                               ('init_params', 'list_key'): init_params_list_key,
                               ('init_params', 'list'): init_params_list})


class ContextRegistryOpData_create_pointer_to_value(CObject):
    """Context registry operation data: create a pointer context (to value).
    """
    def __init__(self, key: 'String', value: 'Object',
                 ptr_type: 'archi_context_registry_op_data_pointer_type_t',
                 init_params_list: 'KeyValueList'):
        from .ctypes import archi_context_registry_op_data_pointer_type_t

        if not isinstance(key, String) \
                or not isinstance(value, (type(None), Object)) \
                or not isinstance(ptr_type, archi_context_registry_op_data_pointer_type_t) \
                or not isinstance(init_params_list, (type(None), KeyValueList)):
            raise TypeError

        from .ctypes import archi_context_registry_op_data__create_pointer_to_value_t

        cobject = archi_context_registry_op_data__create_pointer_to_value_t()
        cobject.type = ptr_type.value

        super().__init__(cobject,
                         refs={'key': key, 'value': value,
                               'init_params_list': init_params_list})


class ContextRegistryOpData_create_pointer_to_context(CObject):
    """Context registry operation data: create a pointer context (to context slot).
    """
    def __init__(self, key: 'String', pointee_key: 'String',
                 pointee_slot_name: 'String', pointee_slot_indices: 'PlainCObject',
                 ptr_type: 'archi_context_registry_op_data_pointer_type_t',
                 init_params_list: 'KeyValueList'):
        from .ctypes import archi_context_registry_op_data_pointer_type_t

        if not isinstance(key, String) \
                or not isinstance(pointee_key, String) \
                or not isinstance(pointee_slot_name, (type(None), String)) \
                or not isinstance(pointee_slot_indices, (type(None), PlainCObject)) \
                or not isinstance(ptr_type, archi_context_registry_op_data_pointer_type_t) \
                or not isinstance(init_params_list, (type(None), KeyValueList)):
            raise TypeError
        elif pointee_slot_indices is not None \
                and pointee_slot_indices.c_element_type() != c.c_ssize_t:
            raise TypeError

        from .ctypes import archi_context_registry_op_data__create_pointer_to_context_t

        cobject = archi_context_registry_op_data__create_pointer_to_context_t()
        if pointee_slot_indices is not None:
            cobject.pointee.slot.num_indices = pointee_slot_indices.length()
        cobject.type = ptr_type.value

        super().__init__(cobject,
                         refs={'key': key,
                               ('pointee', 'key'): pointee_key,
                               ('pointee', 'slot', 'name'): pointee_slot_name,
                               ('pointee', 'slot', 'index'): pointee_slot_indices,
                               'init_params_list': init_params_list})


class ContextRegistryOpData_create_dptr_array(CObject):
    """Context registry operation data: create a data pointer array context.
    """
    def __init__(self, key: 'String', length: 'int'):
        if not isinstance(key, String) \
                or not isinstance(length, int):
            raise TypeError
        elif length < 0:
            raise ValueError

        from .ctypes import archi_context_registry_op_data__create_dptr_array_t

        cobject = archi_context_registry_op_data__create_dptr_array_t()
        cobject.length = length

        super().__init__(cobject,
                         refs={'key': key})

