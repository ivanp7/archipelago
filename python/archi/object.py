 #############################################################################
 # Copyright (C) 2023-2026 by Ivan Podmazov                                  #
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
from types import MappingProxyType

import archi.ctypes as ac


class _ObjectReferencesMixin:
    """Support for references in objects.
    """
    def __init__(self, owner, refs, /):
        if not isinstance(owner, Object):
            raise TypeError
        elif not isinstance(refs, (dict, MappingProxyType)):
            raise TypeError
        elif None in refs:
            raise KeyError
        elif not all(isinstance(obj, (type(None), Object)) for obj in refs.values()):
            raise TypeError

        if isinstance(refs, dict):
            refs = MappingProxyType(refs)

        self._owner = owner
        self._refs = refs.copy() if refs is not None else {}

    def __contains__(self, key):
        """Check if there is a reference with the specified key.
        """
        return key in self._refs

    def __getitem__(self, key):
        """Get a referenced object by key.
        """
        return self._refs[key]

    @property
    def ref_map(self):
        """Get the map of references.
        """
        return self._refs

    def ref_set(self, nested=False):
        """Get the set of references.
        """
        if not nested:
            ref_set = set(self._refs.values())
        else:
            ref_set = set()
            refs = self.ref_set()
            while refs:
                ref_set.update(refs)

                nested_refs = set()
                for obj in refs:
                    nested_refs.update(obj.ref_set())

                refs = nested_refs.difference(ref_set)

        ref_set.discard(None)
        return ref_set


class _ObjectEquivalentsMixin:
    """Support for object equivalence checks.
    """
    def __init__(self, owner, /):
        if not isinstance(owner, Object):
            raise TypeError

        self._owner = owner

    def equivalent_to(self, other, /):
        """Check if the object is equivalent to (replaceable with) another object.
        """
        if not isinstance(other, (type(None), Object)):
            raise TypeError

        if self._owner is other:
            return True
        elif other is None:
            return False

        if self._owner.total_size != other.total_size:
            return False
        elif self._owner.total_alignment != other.total_alignment:
            return False

        if self._owner.ref_map.keys() != other.ref_map.keys():
            return False

        if isinstance(self._owner, type(other)):
            if not self._owner._equivalent_to(other):
                return False
        elif isinstance(other, type(self._owner)):
            if not other._equivalent_to(self._owner):
                return False
        else:
            return False

        return all(_ObjectEquivalentsMixin.equivalent(obj, other[key])
                   for key, obj in self._owner.ref_map.items())

    @staticmethod
    def equivalent(obj1, obj2, /):
        """Check if the object is equivalent to (replaceable with) another object.
        """
        if obj1 is None and obj2 is None:
            return True
        elif obj1 is None or obj2 is None:
            return False
        else:
            return obj1.equivalent_to(obj2)


class Object:
    """Immutable representation of an (array) object backed by a single continuous memory block.
    """
    def __init__(self, length, stride, alignment, tag=None, refs=None):
        """Initialize an object.
        """
        self._attr = ac.archi_pointer_attr_t.primitive_data(length, stride, alignment)
        if tag is not None:
            self._attr = ac.archi_pointer_attr_t.complex_data(tag)

        self._refs = _ObjectReferencesMixin(self, refs if refs is not None else {})
        self._eqv = _ObjectEquivalentsMixin(self)

        self._length = length
        self._stride = stride
        self._alignment = alignment
        self._tag = tag

        self._reset_address()

    @property
    def attributes(self):
        """Get type attributes.
        """
        return self._attr

    @property
    def tag(self):
        """Get opaque data type tag.
        """
        return self._tag

    @property
    def length(self):
        """Get number of data elements.
        """
        return self._length

    @property
    def stride(self):
        """Get size of a data element in bytes.
        """
        return self._stride

    @property
    def alignment(self):
        """Get alignment requirement of a data element in bytes.
        """
        return self._alignment

    @property
    def total_size(self):
        """Get total size of data in bytes.

        This method may be redefined to implement support for additional trailing padding.
        """
        return self.length * self.stride

    @property
    def total_alignment(self):
        """Get total alignment requirement of data in bytes.

        This method may be redefined to implement support for over-aligned data.
        """
        return self.alignment

    def address_of(self, ref_key=None, /):
        """Get nominal address in memory of the object itself or a referenced object.

        This address is to be assigned to pointers in C structures,
        and not to be used to access the real object.
        The real object is accessed through the effective address instead,
        which is only available as 'address' parameter in write() method.
        """
        obj = self if ref_key is None else self[ref_key]
        return obj._address if obj is not None else None

    def _set_address(self, address, /):
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

    @staticmethod
    def random_address(start = 0x7f0000000000, stop = 0x800000000000, alignment = 0x1000):
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

    def to_bytearray(self, address):
        """Encode the object as a byte array to be mapped at the specified address.

        Parameter 'address' contains the nominal address of the object.
        """
        if not isinstance(address, int):
            raise TypeError
        elif address < 0:
            raise ValueError("Address cannot be negative")
        elif address % self.total_alignment != 0:
            raise ValueError("Address is not divisible by the alignment requirement")

        # Create the byte array
        buffer = bytearray(self.total_size)

        if self.total_size != 0:
            # Set nominal address
            self._set_address(address)

            # Write object contents to the buffer
            eff_address = c.addressof(c.c_char.from_buffer(buffer))
            self._write(eff_address)

            # Reset nominal address
            self._reset_address()

        return buffer

    def _write(self, eff_address, /):
        """Write object to memory.

        This method is to be implemented in derived classes.
        It must write (no more than) `self.total_size` bytes at the specified address.

        Parameter 'eff_address' contains the effective address of the real object,
        it is to be used to access the modified object.
        """
        raise NotImplementedError

    ### references ###

    def __contains__(self, key):
        """Check if there is a reference with the specified key.
        """
        return key in self._refs

    def __getitem__(self, key):
        """Get a referenced object by key.
        """
        return self._refs[key]

    @property
    def ref_map(self):
        """Get the map of referenced objects.
        """
        return self._refs.ref_map

    def ref_set(self, nested=False):
        """Get the set of referenced objects.
        """
        return self._refs.ref_set(nested)

    ### equivalents ###

    def equivalent_to(self, other, /):
        """Check if the object is equivalent to (replaceable with) another object.
        """
        return self._eqv.equivalent_to(other)

    def _equivalent_to(self, other, /):
        """Check if the object is equivalent to (replaceable with) another object.

        This method is to be reimplemented in derived classes.
        """
        return True

    @staticmethod
    def equivalent(obj1, obj2, /):
        """Check if the object is equivalent to (replaceable with) another object.
        """
        return type(self._eqv).equivalent(obj1, obj2)

##############################################################################

class ObjectEquivalentSet(Object):
    """Immutable representation of a set of equivalent objects.
    """
    def __init__(self, equivalents, /):
        """Initialize representation of a set of object equivalents.
        """
        if not isinstance(equivalents, frozenset):
            raise TypeError
        elif len(equivalents) == 0:
            raise ValueError
        elif not all(isinstance(obj, Object) for obj in equivalents):
            raise TypeError

        iterator = iter(equivalents)
        self._obj = next(iterator)
        self._equivalents = equivalents

        for obj in iterator:
            if not self._obj.equivalent_to(obj):
                raise ValueError

        if self._obj.total_size != 0:
            super().__init__(length=1, stride=self._obj.total_size,
                             alignment=self._obj.total_alignment,
                             refs=self._obj.ref_map)
        else:
            super().__init__(length=0, stride=1, alignment=1,
                             refs=self._obj.ref_map)

    @property
    def equivalents(self):
        """Get the set of equivalent objects.
        """
        return self._equivalents

    def _set_address(self, address, /):
        """Set nominal (written to memory) address of the object in memory.
        """
        super()._set_address(address)

        for obj in self._equivalents:
            obj._set_address(address)

    def _reset_address(self):
        """Reset nominal address of the object in memory.
        """
        super()._reset_address()

        for obj in self._equivalents:
            obj._reset_address()

    def _write(self, eff_address, /):
        """Write object to memory.
        """
        self._obj._write(eff_address)

    def _equivalent_to(self, other, /):
        """Check equivalence of objects.
        """
        return self._obj._equivalent_to(other)

##############################################################################

class ObjectSequence(Object):
    """Immutable representation of a sequence of objects.
    """
    def __init__(self, seq, /, tag=None):
        """Initialize an object sequence representation.
        """
        if not isinstance(seq, tuple):
            raise TypeError
        elif not all(isinstance(obj, Object) for obj in seq):
            raise TypeError

        self._sequence = seq

        self._objects = {}
        self._padding = 0

        if len(seq) == 0:
            super().__init__(length=0, stride=1, alignment=1, tag=tag)
            return

        # Compute total size, alignment, padding
        total_size = 0
        total_alignment = 1

        for obj in seq:
            size = obj.total_size
            alignment = obj.total_alignment

            total_alignment = max(total_alignment, alignment)

            offset = (total_size + (alignment - 1)) & ~(alignment - 1)
            self._objects[obj] = offset
            self._padding += offset - total_size

            total_size = offset + size

        total_size = (total_size + (total_alignment - 1)) & ~(total_alignment - 1)

        # Merge reference maps of sequence objects
        ref_map = {}
        for idx, obj in enumerate(seq):
            ref_map |= {(idx, key): ref for key, ref in obj.ref_map.items()}

        # Call the parent constructor
        if total_size != 0:
            super().__init__(length=1, stride=total_size, alignment=total_alignment, tag=tag, refs=ref_map)
        else:
            super().__init__(length=0, stride=1, alignment=1, tag=tag, refs=ref_map)

    @property
    def objects(self):
        """Get the tuple of objects.
        """
        return self._sequence

    @property
    def total_padding(self):
        """Get the total number of padding bytes in the object sequence.
        """
        return self._padding

    def _write(self, eff_address, /):
        """Write object sequence to memory.
        """
        # Set nominal addresses of key objects and all objects equivalent to them
        for obj, offset in self._objects.items():
            obj._set_address(self.address_of() + offset)

        # Write key objects to memory at their respective offsets
        for obj, offset in self._objects.items():
            obj._write(eff_address + offset)

        # Reset nominal addresses of objects
        for obj in self._objects.keys():
            obj._reset_address()

    def _equivalent_to(self, other, /):
        """Check equivalence of object sequences.
        """
        if not isinstance(other, ObjectSequence):
            return False
        elif len(self.objects) == len(other.objects):
            return False

        for obj1, obj2 in zip(self.objects, other.objects):
            if not obj1.equivalent_to(obj2):
                return False

        return True

##############################################################################

class ObjectRefTree(ObjectSequence):
    """Immutable representation of an object packed together with its all nested references.

    Equivalent objects are identified and merged to reduce the total size of the sequence.
    """
    def __init__(self, root, /):
        """Initialize an object reference tree.
        """
        if not isinstance(root, Object):
            raise TypeError
        elif root.total_size == 0:
            raise ValueError("root object cannot be of zero size")

        # Get the whole tree of subreferences
        objects = root.ref_set(nested=True)
        objects.discard(root) # should not be needed, as circular dependencies are not supported

        ###############################################
        # Optimize size by merging equivalent objects #
        ###############################################

        # Create a map of objects to sets of all equivalent objects
        equivalents = {root: set()}

        while objects:
            obj = next(iter(objects))

            if obj.total_size != 0: # zero size objects are omitted and won't be assigned an address
                for key_obj, eq_objs in equivalents.items():
                    if obj.equivalent_to(key_obj):
                        eq_objs.add(obj)
                        break
                else:
                    equivalents[obj] = set()

            objects.remove(obj)

        # Merge the root object equivalents
        if equivalents[root]:
            equivalents[root].add(root)
            root = ObjectEquivalentSet(frozenset(equivalents[root]))

        del equivalents[root]

        # Create the list of remaining objects
        objects = []
        for key_obj, eq_objs in equivalents.items():
            if eq_objs:
                eq_objs.add(key_obj)
                objects.append(ObjectEquivalentSet(frozenset(eq_objs)))
            else:
                objects.append(key_obj)

        del equivalents

        #####################################################
        # Reorder the list of objects in a (sub)optimal way #
        #####################################################

        # Sort objects by descending alignment (for equal alignments, by descending size)
        def compare(obj1, obj2):
            return obj2.total_alignment - obj1.total_alignment \
                    if obj2.total_alignment != obj1.total_alignment \
                    else obj2.total_size - obj1.total_size

        from functools import cmp_to_key
        objects.sort(key=cmp_to_key(compare))

        # Pack the sorted list of objects
        offset_obj = {}

        gaps = [] # list of gaps: (offset, size)
        total_size = root.total_size

        for obj in objects:
            obj_size = obj.total_size
            obj_alignment = obj.total_alignment

            # Try to fit the object into the present gaps
            for gap_idx, (gap_offset, gap_size) in enumerate(gaps):
                 # Compute the aligned offset within the gap and the remaining gap size
                gap_offset_p = (gap_offset + (obj_alignment - 1)) & ~(obj_alignment - 1)
                gap_size_p = gap_size - (gap_offset_p - gap_offset)

                # Check if the object fits in the gap
                if obj_size <= gap_size_p:
                    offset = gap_offset_p

                    # Update the list of gaps
                    new_gaps = []

                    if gap_offset_p > gap_offset:
                        new_gaps.append((gap_offset, gap_offset_p - gap_offset))

                    if obj_size < gap_size_p:
                        new_gaps.append((gap_offset_p + obj_size, gap_size_p - obj_size))

                    gaps[gap_idx:gap_idx+1] = new_gaps
                    break

            else: # the object does not fit into any gaps, append it to the end
                offset = (total_size + (obj_alignment - 1)) & ~(obj_alignment - 1)

                if offset > total_size:
                    gaps.append((total_size, offset - total_size))

                total_size = offset + obj_size

            # Insert the object into the map
            offset_obj[offset] = obj

        # Prepare the final list of objects
        objects = [root] + [pair[1] for pair in sorted(offset_obj.items())]

        ###############################
        # Call the parent constructor #
        ###############################

        super().__init__(tuple(objects), tag=root.tag)

    @property
    def root_object(self):
        """Get the root object.
        """
        return self.objects[0]

    def _equivalent_to(self, other, /):
        """Check equivalence of object reference trees.
        """
        return isinstance(other, ObjectRefTree) \
                and self.root_object.equivalent_to(other.root_object)

##############################################################################

class PrimitiveData(Object):
    """Immutable representation of primitive data (without pointers).
    """
    def __init__(self, cobject, /, tag=None):
        """Initialize a primitive data representation.
        """
        self._buffer = bytes(cobject)
        self._type = type(cobject)

        if isinstance(cobject, c.Array):
            self._type1 = cobject._type_
            super().__init__(length=len(cobject), stride=c.sizeof(cobject._type_),
                             alignment=c.alignment(cobject._type_), tag=tag)
        else:
            self._type1 = self._type
            super().__init__(length=1, stride=c.sizeof(cobject),
                             alignment=c.alignment(cobject), tag=tag)

    def __str__(self):
        return f"PrimitiveData({self.c_object})"

    @property
    def buffer(self):
        """Get managed bytes buffer.
        """
        return self._buffer

    @property
    def c_type(self):
        """Get type of the original ctypes object.
        """
        return self._type

    @property
    def c_element_type(self):
        """Get type of an element of the original ctypes object.
        """
        return self._type1

    @property
    def c_object(self):
        """Get copy of the original ctypes object.
        """
        return self.c_type.from_buffer_copy(self.buffer)

    def _write(self, eff_address, /):
        """Write object to memory.
        """
        c.memmove(eff_address, self._buffer, len(self._buffer))

    def _equivalent_to(self, other, /):
        """Check equivalence with another C object.
        """
        return isinstance(other, PrimitiveData) \
                and self.buffer == other.buffer

    @staticmethod
    def from_bytes(buf, /, tag=None):
        """Create a primitive data representation from a raw byte buffer.
        """
        if not isinstance(buf, (type(None), bytes)):
            raise TypeError

        if buf is None:
            return None

        return PrimitiveData(c.create_string_buffer(buf, len(buf)), tag=tag)


class String(PrimitiveData):
    """Immutable representation of a C string.
    """
    def __init__(self, string, /, encoding=None):
        """Initialize a C string wrapper.
        """
        if not isinstance(string, str):
            raise TypeError

        self._string = string

        super().__init__(c.create_string_buffer(string.encode() if encoding is None else
                                                string.encode(encoding=encoding)))

    def __str__(self):
        return f"String('{self.string}')"

    @property
    def string(self):
        """Get the original string.
        """
        return self._string

    @staticmethod
    def nullable(string, /, encoding=None):
        """Construct a C string representation.
        """
        if string is None:
            return None

        return String(string, encoding=encoding)


class ComplexData(Object):
    """Immutable representation of complex data (with pointers).

    In a subclass:
    - `TYPE` must be redefined as a type derived from `c.Structure`;
    - `TAG` may be redefined as an integer tag of the type;
    - `REFS` must be a redefined as a dict {key: subclass_of_Object},
    where `key` is a string suitable as a kwargs key.
    """
    TYPE = None
    TAG = None
    REFS = None

    def __init_subclass__(cls):
        """Initialize a complex data representation subclass.
        """
        if not issubclass(cls.TYPE, c.Structure):
            raise TypeError
        elif not isinstance(cls.TAG, (type(None), int)):
            raise TypeError
        elif cls.TAG is not None and (cls.TAG < 0 or cls.TAG > ac.archi_pointer_attr_t.DATA_TAG_MAX):
            raise ValueError
        elif not isinstance(cls.REFS, dict):
            raise TypeError

        for key, value in cls.REFS.items():
            if not isinstance(key, str):
                raise TypeError
            elif not issubclass(value, Object):
                raise TypeError

    def __init__(self, _=None, /, **kwargs):
        """Initialize a complex data representation instance.
        """
        if not isinstance(_, (type(None), type(self).TYPE)):
            raise TypeError

        for key, obj in kwargs.items():
            if key not in type(self).REFS:
                raise KeyError(f"Complex data object: unrecognized reference key '{key}'")
            elif not isinstance(obj, (type(None), type(self).REFS[key])):
                raise TypeError(f"Complex data object: reference '{key}' has incorrect type")

        if _ is None:
            _ = type(self).TYPE()

        self._buffer = bytes(_)

        refs = {key: (kwargs[key] if key in kwargs else None) for key in type(self).REFS}

        super().__init__(length=1, stride=c.sizeof(_), alignment=c.alignment(_),
                         tag=type(self).TAG, refs=refs)

    @property
    def buffer(self):
        """Get managed bytes buffer.
        """
        return self._buffer

    @property
    def c_object(self):
        """Get copy of the original ctypes object.
        """
        return type(self).TYPE.from_buffer_copy(self.buffer)

    def _write(self, eff_address, /):
        """Write object to memory.
        """
        c.memmove(eff_address, self._buffer, len(self._buffer))

        cobject = type(self).TYPE.from_address(eff_address)
        self._write_fields(cobject)

    def _write_fields(self, cobject, /):
        """Assign the object pointer fields.

        This method is to be implemented in derived classes.
        """
        raise NotImplementedError

    def _equivalent_to(self, other, /):
        """Check equivalence with another C object.
        """
        return isinstance(other, ComplexData) \
                and self.buffer == other.buffer

##############################################################################
# Base types
##############################################################################

class KeyValueList(ComplexData):
    """Representation of a key-value list node.
    """
    TYPE = ac.archi_kvlist_t
    TAG = ac.archi_kvlist_t.TAG
    REFS = {# 'next': KeyValueList,
            'key': String,
            'value': Object}

    def _write_fields(self, cobject, /):
        cobject.next = c.cast(self.address_of('next'), c.POINTER(ac.archi_kvlist_t))
        cobject.key = self.address_of('key')
        cobject.value.assign(self['value'])

    @classmethod
    def construct(cls, key_object_tuples, /):
        """Construct a key-value list object from a list of (key, value_obj) tuples.
        """
        if not key_object_tuples:
            return None

        key_object_tuples = list(key_object_tuples)

        key_obj = {key for key, _ in key_object_tuples}
        key_obj = {key: String.nullable(key) for key in key_obj}

        kvlist_node = None
        for key, obj in reversed(key_object_tuples):
            kvlist_node = cls(key=key_obj[key], value=obj, next=kvlist_node)

        return kvlist_node

KeyValueList.REFS['next'] = KeyValueList

##############################################################################
# Types used to describe aggregate types
##############################################################################

class AggregateMemberType(ComplexData):
    """Base class for descriptions of aggregate member types.
    """
    TYPE = ac.archi_layout_type_t
    REFS = {}

    KIND = None


class AggregateMember(ComplexData):
    """Description of an aggregate member.
    """
    TYPE = ac.archi_aggr_member_t
    REFS = {'name': String,
            'member_type': AggregateMemberType}

    def _write_fields(self, cobject, /):
        cobject.name = self.address_of('name')
        if self['member_type'] is not None:
            cobject.kind = self['member_type'].KIND
        cobject.layout = c.cast(self.address_of('member_type'), c.POINTER(ac.archi_layout_type_t))


class AggregateMembers(ObjectSequence):
    """Array of descriptions of aggregate members.
    """
    def __init__(self, member_seq, /):
        if not isinstance(member_seq, tuple):
            raise TypeError
        elif not all(isinstance(obj, AggregateMemberType) for obj in member_seq):
            raise TypeError

        super().__init__(member_seq)

    @property
    def num_members(self):
        """Get number of members in the array.
        """
        return len(self.objects)

    @property
    def members(self):
        """Get tuple of members in the array.
        """
        return self.objects


class AggregateMemberType_Value(AggregateMemberType):
    """Description of a value type of an aggregate member.
    """
    TYPE = ac.archi_aggr_member_type__value_t
    REFS = {}

    KIND = ac.archi_aggr_member_t.KIND_VALUE

    def _write_fields(self, cobject, /):
        pass # nothing to do


class AggregateMemberType_Pointer(AggregateMemberType):
    """Description of a pointer type of an aggregate member.
    """
    TYPE = ac.archi_aggr_member_type__pointer_t
    REFS = {}

    KIND = ac.archi_aggr_member_t.KIND_POINTER

    def _write_fields(self, cobject, /):
        pass # nothing to do


class AggregateMemberType_Aggregate(AggregateMemberType):
    """Description of an aggregate member type.
    """
    TYPE = ac.archi_aggr_member_type__aggregate_t
    REFS = {'members': AggregateMembers}

    KIND = ac.archi_aggr_member_t.KIND_AGGREGATE

    def _write_fields(self, cobject, /):
        cobject.num_members = self['members'].num_members if self['members'] is not None else 0
        cobject.members = c.cast(self.address_of('members'), c.POINTER(ac.archi_aggr_member_t))


class AggregateType(ComplexData):
    """Description of an aggregate type.
    """
    TYPE = ac.archi_aggr_type_t
    TAG = ac.archi_aggr_type_t.TAG
    REFS = {'members': AggregateMembers,
            'init_value': Object,
            'init_value_fam': Object}

    def _write_fields(self, cobject, /):
        cobject.top_level.num_members = self['members'].num_members if self['members'] is not None else 0
        cobject.top_level.members = c.cast(self.address_of('members'), c.POINTER(ac.archi_aggr_member_t))

        cobject.init_value = self.address_of('init_value')
        cobject.init_value_fam = self.address_of('init_value_fam')

##############################################################################
# Types used in input files
##############################################################################

class AppInputFileHeader(ComplexData):
    """Immutable representation of an application input file header.
    """
    TYPE = ac.archi_app_input_file_header_t
    REFS = {'contents': KeyValueList}

    def _write_fields(self, cobject, /):
        cobject.contents = c.cast(self.address_of('contents'), c.POINTER(ac.archi_kvlist_t))


class AppInputFile(ObjectRefTree):
    """Immutable representation of an application input file.
    """
    def __init__(self, header, /):
        """Initialize an input file representation.
        """
        if not isinstance(header, AppInputFileHeader):
            raise TypeError

        super().__init__(header)

    def _write(self, eff_address, /):
        """Write input file to memory.
        """
        super()._write(eff_address)

        header = ac.archi_app_input_file_header_t.from_address(eff_address)

        header.header.addr = self.address_of()
        header.header.size = self.total_size - c.sizeof(header.header)

##############################################################################
# Registry operations: data for basic context registry operations
##############################################################################

class ContextSlotIndices(PrimitiveData):
    """Immutable representation of an array of context slot indices.
    """
    def __init__(self, cobject, /):
        """Initialize a representation of context slot index array.
        """
        if not isinstance(cobject, c.Array):
            raise TypeError
        elif not issubclass(cobject._type_, ac.archi_context_slot_index_t):
            raise TypeError

        super().__init__(cobject)

    @staticmethod
    def nullable(indices, /):
        """Construct context slot indices representation.
        """
        if not indices:
            return None

        return ContextSlotIndices((ac.archi_context_slot_index_t * len(indices))(*indices))


class RegistryOpData_delete(ComplexData):
    """Registry operation data: delete a context.
    """
    TYPE = ac.archi_app_registry_op_data__delete_t
    REFS = {'key': String}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')

    @classmethod
    def construct(cls, *, key):
        return cls(cls.TYPE(),
                   key=String(key))


class RegistryOpData_alias(ComplexData):
    """Registry operation data: create a context alias.
    """
    TYPE = ac.archi_app_registry_op_data__alias_t
    REFS = {'key': String,
            'original_key': String}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.original_key = self.address_of('original_key')

    @classmethod
    def construct(cls, *, key, original_key):
        return cls(cls.TYPE(),
                   key=String(key),
                   original_key=String(original_key))


class RegistryOpData_create_as(ComplexData):
    """Registry operation data: create a context using interface of another context.
    """
    TYPE = ac.archi_app_registry_op_data__create_as_t
    REFS = {'key': String,
            'sample_key': String,
            'init_params_context_key': String,
            'init_params_list': KeyValueList}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.sample_key = self.address_of('sample_key')
        cobject.init_params.context_key = self.address_of('init_params_context_key')
        cobject.init_params.list = c.cast(self.address_of('init_params_list'), c.POINTER(ac.archi_kvlist_t))

    @classmethod
    def construct(cls, *, key, sample_key, init_params_context_key, init_params_list):
        return cls(cls.TYPE(),
                   key=String(key),
                   sample_key=String(sample_key),
                   init_params_context_key=String.nullable(init_params_context_key),
                   init_params_list=KeyValueList.construct(init_params_list.items()))


class RegistryOpData_create_from(ComplexData):
    """Registry operation data: create a context using interface obtained from another context slot.
    """
    TYPE = ac.archi_app_registry_op_data__create_from_t
    REFS = {'key': String,
            'source_key': String,
            'source_slot_name': String,
            'source_slot_indices': ContextSlotIndices,
            'init_params_context_key': String,
            'init_params_list': KeyValueList}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.source_key = self.address_of('source_key')
        cobject.source_slot.name = self.address_of('source_slot_name')
        cobject.source_slot.index = c.cast(self.address_of('source_slot_indices'), c.POINTER(ac.archi_context_slot_index_t))
        cobject.source_slot.num_indices = self['source_slot_indices'].length if self['source_slot_indices'] is not None else 0
        cobject.init_params.context_key = self.address_of('init_params_context_key')
        cobject.init_params.list = c.cast(self.address_of('init_params_list'), c.POINTER(ac.archi_kvlist_t))

    @classmethod
    def construct(cls, *, key, source_key, source_slot_name, source_slot_indices,
                  init_params_context_key, init_params_list):
        return cls(cls.TYPE(),
                   key=String(key),
                   source_key=String(source_key),
                   source_slot_name=String.nullable(source_slot_name),
                   source_slot_indices=ContextSlotIndices.nullable(source_slot_indices),
                   init_params_context_key=String.nullable(init_params_context_key),
                   init_params_list=KeyValueList.construct(init_params_list.items()))


class RegistryOpData_create_params(ComplexData):
    """Registry operation data: create a parameter list context.
    """
    TYPE = ac.archi_app_registry_op_data__create_params_t
    REFS = {'key': String,
            'params_context_key': String,
            'params_list': KeyValueList}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.params.context_key = self.address_of('params_context_key')
        cobject.params.list = c.cast(self.address_of('params_list'), c.POINTER(ac.archi_kvlist_t))

    @classmethod
    def construct(cls, *, key, params_context_key, params_list):
        return cls(cls.TYPE(),
                   key=String(key),
                   params_context_key=String.nullable(params_context_key),
                   params_list=KeyValueList.construct(params_list.items()))


class RegistryOpData_create_ptr(ComplexData):
    """Registry operation data: create a pointer context.
    """
    TYPE = ac.archi_app_registry_op_data__create_ptr_t
    REFS = {'key': String,
            'pointee': Object}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.pointee.assign(self['pointee'])

    @classmethod
    def construct(cls, *, key, pointee):
        return cls(cls.TYPE(),
                   key=String(key),
                   pointee=pointee)


class RegistryOpData_create_dptr_array(ComplexData):
    """Registry operation data: create a data pointer array context.
    """
    TYPE = ac.archi_app_registry_op_data__create_dptr_array_t
    REFS = {'key': String}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')

    @classmethod
    def construct(cls, *, key, length):
        return cls(cls.TYPE(length),
                   key=String(key))


class RegistryOpData_invoke(ComplexData):
    """Registry operation data: invoke context call.
    """
    TYPE = ac.archi_app_registry_op_data__invoke_t
    REFS = {'key': String,
            'slot_name': String,
            'slot_indices': ContextSlotIndices,
            'call_params_context_key': String,
            'call_params_list': KeyValueList}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.slot.name = self.address_of('slot_name')
        cobject.slot.index = c.cast(self.address_of('slot_indices'), c.POINTER(ac.archi_context_slot_index_t))
        cobject.slot.num_indices = self['slot_indices'].length if self['slot_indices'] is not None else 0
        cobject.call_params.context_key = self.address_of('call_params_context_key')
        cobject.call_params.list = c.cast(self.address_of('call_params_list'), c.POINTER(ac.archi_kvlist_t))

    @classmethod
    def construct(cls, *, key, slot_name, slot_indices,
                  call_params_context_key, call_params_list):
        return cls(cls.TYPE(),
                   key=String(key),
                   slot_name=String.nullable(slot_name),
                   slot_indices=ContextSlotIndices.nullable(slot_indices),
                   call_params_context_key=String.nullable(call_params_context_key),
                   call_params_list=KeyValueList.construct(call_params_list.items()))


class RegistryOpData_unassign(ComplexData):
    """Registry operation data: unset context slot.
    """
    TYPE = ac.archi_app_registry_op_data__unassign_t
    REFS = {'key': String,
            'slot_name': String,
            'slot_indices': ContextSlotIndices}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.slot.name = self.address_of('slot_name')
        cobject.slot.index = c.cast(self.address_of('slot_indices'), c.POINTER(ac.archi_context_slot_index_t))
        cobject.slot.num_indices = self['slot_indices'].length if self['slot_indices'] is not None else 0

    @classmethod
    def construct(cls, *, key, slot_name, slot_indices):
        return cls(cls.TYPE(),
                   key=String(key),
                   slot_name=String.nullable(slot_name),
                   slot_indices=ContextSlotIndices.nullable(slot_indices))


class RegistryOpData_assign(ComplexData):
    """Registry operation data: set context slot to value.
    """
    TYPE = ac.archi_app_registry_op_data__assign_t
    REFS = {'key': String,
            'slot_name': String,
            'slot_indices': ContextSlotIndices,
            'value': Object}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.slot.name = self.address_of('slot_name')
        cobject.slot.index = c.cast(self.address_of('slot_indices'), c.POINTER(ac.archi_context_slot_index_t))
        cobject.slot.num_indices = self['slot_indices'].length if self['slot_indices'] is not None else 0
        cobject.value.assign(self['value'])

    @classmethod
    def construct(cls, *, key, slot_name, slot_indices, value):
        return cls(cls.TYPE(),
                   key=String(key),
                   slot_name=String.nullable(slot_name),
                   slot_indices=ContextSlotIndices.nullable(slot_indices),
                   value=value)


class RegistryOpData_assign_slot(ComplexData):
    """Registry operation data: set context slot to value of another context slot.
    """
    TYPE = ac.archi_app_registry_op_data__assign_slot_t
    REFS = {'key': String,
            'slot_name': String,
            'slot_indices': ContextSlotIndices,
            'source_key': String,
            'source_slot_name': String,
            'source_slot_indices': ContextSlotIndices}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.slot.name = self.address_of('slot_name')
        cobject.slot.index = c.cast(self.address_of('slot_indices'), c.POINTER(ac.archi_context_slot_index_t))
        cobject.slot.num_indices = self['slot_indices'].length if self['slot_indices'] is not None else 0
        cobject.source_key = self.address_of('source_key')
        cobject.source_slot.name = self.address_of('source_slot_name')
        cobject.source_slot.index = c.cast(self.address_of('source_slot_indices'), c.POINTER(ac.archi_context_slot_index_t))
        cobject.source_slot.num_indices = self['source_slot_indices'].length if self['source_slot_indices'] is not None else 0

    @classmethod
    def construct(cls, *, key, slot_name, slot_indices, source_key, source_slot_name, source_slot_indices):
        return cls(cls.TYPE(),
                   key=String(key),
                   slot_name=String.nullable(slot_name),
                   slot_indices=ContextSlotIndices.nullable(slot_indices),
                   source_key=String(source_key),
                   source_slot_name=String.nullable(source_slot_name),
                   source_slot_indices=ContextSlotIndices.nullable(source_slot_indices))


class RegistryOpData_assign_call(ComplexData):
    """Registry operation data: set context slot to result of another context call.
    """
    TYPE = ac.archi_app_registry_op_data__assign_call_t
    REFS = {'key': String,
            'slot_name': String,
            'slot_indices': ContextSlotIndices,
            'source_key': String,
            'source_slot_name': String,
            'source_slot_indices': ContextSlotIndices,
            'source_call_params_context_key': String,
            'source_call_params_list': KeyValueList}

    def _write_fields(self, cobject, /):
        cobject.key = self.address_of('key')
        cobject.slot.name = self.address_of('slot_name')
        cobject.slot.index = c.cast(self.address_of('slot_indices'), c.POINTER(ac.archi_context_slot_index_t))
        cobject.slot.num_indices = self['slot_indices'].length if self['slot_indices'] is not None else 0
        cobject.source_key = self.address_of('source_key')
        cobject.source_slot.name = self.address_of('source_slot_name')
        cobject.source_slot.index = c.cast(self.address_of('source_slot_indices'), c.POINTER(ac.archi_context_slot_index_t))
        cobject.source_slot.num_indices = self['source_slot_indices'].length if self['source_slot_indices'] is not None else 0
        cobject.source_call_params.context_key = self.address_of('source_call_params_context_key')
        cobject.source_call_params.list = c.cast(self.address_of('source_call_params_list'), c.POINTER(ac.archi_kvlist_t))

    @classmethod
    def construct(cls, *, key, slot_name, slot_indices, source_key, source_slot_name, source_slot_indices,
                  source_call_params_context_key, source_call_params_list):
        return cls(cls.TYPE(),
                   key=String(key),
                   slot_name=String.nullable(slot_name),
                   slot_indices=ContextSlotIndices.nullable(slot_indices),
                   source_key=String(source_key),
                   source_slot_name=String.nullable(source_slot_name),
                   source_slot_indices=ContextSlotIndices.nullable(source_slot_indices),
                   source_call_params_context_key=String.nullable(source_call_params_context_key),
                   source_call_params_list=KeyValueList.construct(source_call_params_list.items()))

##############################################################################
# Signal management
##############################################################################

class SignalSet(PrimitiveData):
    """Immutable representation of a set of POSIX signals.
    """
    # Input file contents key for signal sets
    INPUT_FILE_KEY = 'signals'

    def __init__(self, cobject, /):
        """Initialize a representation of a set of POSIX signals.
        """
        if not isinstance(cobject, ac.archi_signal_set_t):
            raise TypeError

        super().__init__(cobject)

    @classmethod
    def construct(cls, **signals):
        """Construct a set of POSIX signals.
        """
        signal_set = ac.archi_signal_set_t()

        for signal, value in signals.items():
            if signal == ac.SIGNAL_RT_MIN or signal == ac.SIGNAL_RT_MAX:
                for index in value:
                    getattr(signal_set, signal)[index] = True
            else:
                setattr(signal_set, signal, value)

        return signal_set

