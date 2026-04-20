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
# @brief C bindings of the Archipelago library.

import ctypes as c
import signal


##############################################################################
# Base types
##############################################################################

ARCHI_POINTER_DATA_TAG__POINTER = 0x02
ARCHI_POINTER_DATA_TAG__RCPOINTER = 0x03
ARCHI_POINTER_DATA_TAG__DPTR_ARRAY = 0x04
ARCHI_POINTER_DATA_TAG__KVLIST = 0x06
ARCHI_POINTER_DATA_TAG__KRCVLIST = 0x07
ARCHI_POINTER_FUNC_TAG__STRING_COMP = 0x05


class archi_pointer_attr_t(c.c_uint64):
    """Pointer attributes.
    """
    ATTR_BITS = 64 - 2        # 62
    SIZE_BITS = ATTR_BITS - 6 # 56

    DATA_SIZE_MAX   = (1 <<  SIZE_BITS) - 1  # 0x00FFFFFFFFFFFFFF
    DATA_STRIDE_MAX =  1 << (SIZE_BITS  - 1) # 0x0080000000000000
    DATA_TAG_MAX = ((1 << ATTR_BITS) - 1) & ~(SIZE_BITS << SIZE_BITS) # 0x07FFFFFFFFFFFFFF
    FUNC_TAG_MAX =  (1 << ATTR_BITS) - 1                              # 0x3FFFFFFFFFFFFFFF

    TYPE_MASK = 0x3 << ATTR_BITS
    DATA_ON_STACK = 0x0 << ATTR_BITS
    DATA_WRITABLE = 0x1 << ATTR_BITS
    DATA_READONLY = 0x2 << ATTR_BITS
    FUNCTION      = 0x3 << ATTR_BITS

    def __str__(self):
        if self.is_data_on_stack:
            mem_type = "on stack"
        elif self.is_data_writable:
            mem_type = "writable"
        elif self.is_data_readonly:
            mem_type = "read only"
        else:
            return f"pointer_attr(function, tag={self.tag})"

        tag = self.tag

        if tag is None:
            return f"pointer_attr(data, {mem_type}, length={self.length}, stride={self.stride}, alignment={self.alignment})"
        else:
            return f"pointer_attr(data, {mem_type}, tag={self.tag})"

    @property
    def is_data(self):
        """Check if the attributes describes a data type.
        """
        return not self.is_function

    @property
    def is_data_on_stack(self):
        """Check if the attributes describes data stored on stack.
        """
        return self.value & archi_pointer_attr_t.TYPE_MASK == archi_pointer_attr_t.DATA_ON_STACK

    @property
    def is_data_writable(self):
        """Check if the attributes describes data stored in writable memory.
        """
        return self.value & archi_pointer_attr_t.TYPE_MASK == archi_pointer_attr_t.DATA_WRITABLE

    @property
    def is_data_readonly(self):
        """Check if the attributes describes data stored in read-only memory.
        """
        return self.value & archi_pointer_attr_t.TYPE_MASK == archi_pointer_attr_t.DATA_READONLY

    @property
    def is_function(self):
        """Check if the attributes describes a function.
        """
        return self.value & archi_pointer_attr_t.TYPE_MASK == archi_pointer_attr_t.FUNCTION

    @property
    def layout(self):
        """Extract memory layout parameters: length, stride, alignment.
        """
        if self.is_function:
            return (None, None, None)

        attr_bits = archi_pointer_attr_t.ATTR_BITS
        size_bits = archi_pointer_attr_t.SIZE_BITS

        stride_width = (self.value >> size_bits) & ((1 << (attr_bits - size_bits)) - 1)

        if stride_width >= size_bits:
            return (None, None, None)

        length = self.value & ((1 << (size_bits - stride_width)) - 1)

        stride_over_alignment = (self.value >> (size_bits - stride_width)) & ((1 << stride_width) - 1)
        alignment_log2 = stride_width - stride_over_alignment.bit_length()
        alignment = 1 << alignment_log2
        stride = (stride_over_alignment + 1) * alignment

        return (length, stride, alignment)

    @property
    def length(self):
        """Extract data length.
        """
        return self.layout[0]

    @property
    def stride(self):
        """Extract data stride.
        """
        return self.layout[1]

    @property
    def alignment(self):
        """Extract data alignment requirement.
        """
        return self.layout[2]

    @property
    def tag(self):
        """Extract type tag from the attributes.
        """
        attr_bits = archi_pointer_attr_t.ATTR_BITS

        if self.is_data:
            tag = ~self.value & ((1 << attr_bits) - 1) # data type tag

            if tag > archi_pointer_attr_t.DATA_TAG_MAX:
                return None
        else:
            tag = self.value & ((1 << attr_bits) - 1) # function type tag

        return tag

    def is_compatible_to(self, other):
        """Check attributes for compatibility.
        """
        if not isinstance(other, archi_pointer_attr_t):
            raise TypeError

        if self.is_data and other.is_data:
            tag = self.tag
            other_tag = other.tag

            if tag is None and other_tag is None:
                length, stride, alignment = self.layout
                other_length, other_stride, other_alignment = other.layout

                return (stride == other_stride) \
                        and (alignment == other_alignment) \
                        and (length >= other_length)
            else:
                return (tag == other_tag) or (tag == 0) or (other_tag == 0)
        elif self.is_function and other.is_function:
            tag = self.tag
            other_tag = other.tag

            return (tag == other_tag) or (tag == 0) or (other_tag == 0)
        else:
            return False

    @staticmethod
    def primitive_data(length, stride=1, alignment=1, writable=False):
        """Compute attributes for a primitive data type.
        """
        if not isinstance(length, int) or not isinstance(stride, int) \
                or not isinstance(alignment, int) or not isinstance(writable, bool):
            raise TypeError

        if length < 0:
            raise ValueError("Length is negative")
        elif stride <= 0:
            raise ValueError("Stride is non-positive")
        elif (alignment <= 0) or ((alignment & (alignment - 1)) != 0):
            raise ValueError("Alignment requirement is not a power of two")
        elif stride % alignment != 0:
            raise ValueError("Stride is not divisible by alignment requirement")
        elif length * stride > archi_pointer_attr_t.DATA_SIZE_MAX:
            raise ValueError("Size (length*stride) exceeds maximum supported value of {archi_pointer_attr_t.DATA_SIZE_MAX} bytes")
        elif stride > archi_pointer_attr_t.DATA_STRIDE_MAX:
            raise ValueError("Stride exceeds maximum supported value of {archi_pointer_attr_t.DATA_STRIDE_MAX} bytes")

        stride_width = (stride - 1).bit_length()

        attr_memtype = archi_pointer_attr_t.DATA_WRITABLE if writable else archi_pointer_attr_t.DATA_READONLY
        attr_stride_width = stride_width << archi_pointer_attr_t.SIZE_BITS
        attr_stride_over_alignment = (stride // alignment - 1) << (archi_pointer_attr_t.SIZE_BITS - stride_width)
        attr_length = length

        return archi_pointer_attr_t(attr_memtype | attr_stride_width |
                                    attr_stride_over_alignment | attr_length)

    @staticmethod
    def complex_data(tag=0, writable=False):
        """Compute attributes for a complex data type.
        """
        if not isinstance(tag, int) or not isinstance(writable, bool):
            raise TypeError

        if tag < 0:
            raise ValueError("Data type tag is negative")
        elif tag > archi_pointer_attr_t.DATA_TAG_MAX:
            raise ValueError("Data type tag exceeds maximum supported value of {archi_pointer_attr_t.DATA_TAG_MAX:016x}")

        attr_memtype = archi_pointer_attr_t.DATA_WRITABLE if writable else archi_pointer_attr_t.DATA_READONLY
        attr_tag = (~tag) & ((1 << archi_pointer_attr_t.ATTR_BITS) - 1)

        return archi_pointer_attr_t(attr_memtype | attr_tag)

    @staticmethod
    def function(tag=0):
        """Compute attributes for a function type.
        """
        if not isinstance(tag, int):
            raise TypeError

        if tag < 0:
            raise ValueError("Function type tag is negative")
        elif tag > archi_pointer_attr_t.FUNC_TAG_MAX:
            raise ValueError("Function type tag exceeds maximum supported value of {archi_pointer_attr_t.FUNC_TAG_MAX:016x}")

        attr_ptrtype = archi_pointer_attr_t.FUNCTION
        attr_tag = tag

        return archi_pointer_attr_t(attr_ptrtype | attr_tag)

    @staticmethod
    def from_type(c_type, /, writable=False):
        """Compute attributes for a primitive data type.
        """
        tag = getattr(c_type, 'TAG', None)

        if tag is None:
            if isinstance(c_type, c.Array):
                return archi_pointer_attr_t.primitive_data(
                        len(c_type), c.sizeof(c_type._type_), c.alignment(c_type._type_), writable)
            else:
                return archi_pointer_attr_t.primitive_data(
                        1, c.sizeof(c_type), c.alignment(c_type), writable)
        else:
            return archi_pointer_attr_t.complex_data(tag, writable)

    @staticmethod
    def from_object(obj, /, writable=False):
        if obj is None:
            return None

        if obj.tag is None:
            return archi_pointer_attr_t.primitive_data(
                    obj.length, obj.stride, obj.alignment, writable)
        else:
            return archi_pointer_attr_t.complex_data(obj.tag, writable)


class archi_pointer_t(c.Structure):
    """Generic wrapper for data pointers with attributes.
    """
    TAG = ARCHI_POINTER_DATA_TAG__POINTER

    POINTEE__ANY = 0
    POINTEE__PDATA = 1
    POINTEE__CDATA = 2
    POINTEE__FUNC = 3

    _fields_ = [('ptr', c.c_void_p),
                ('attr', archi_pointer_attr_t)]

    def assign(self, obj, /, writable=False):
        """Assign pointer fields to refer the specified object.
        """
        if not isinstance(writable, bool):
            raise TypeError

        if obj is not None:
            self.ptr = obj.address_of()
            self.attr = archi_pointer_attr_t.from_object(obj, writable)
        else:
            self.ptr = None
            self.attr = 0


class archi_kvlist_t(c.Structure):
    """List of key-value pairs.
    """
    TAG = ARCHI_POINTER_DATA_TAG__KVLIST

archi_kvlist_t._fields_ = \
        [('next', c.POINTER(archi_kvlist_t)),
         ('key', c.c_char_p),
         ('value', archi_pointer_t)]

##############################################################################
# Contexts
##############################################################################

ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE = 0x10
ARCHI_POINTER_DATA_TAG__CONTEXT = 0x11


archi_context_slot_index_t = c.c_longlong


class archi_context_slot_t(c.Structure):
    """Context slot designator.
    """
    _fields_ = [('name', c.c_char_p),
                ('index', c.POINTER(archi_context_slot_index_t)),
                ('num_indices', c.c_size_t)]


##############################################################################
# Description of aggregate types
##############################################################################

ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE = 0x20
ARCHI_POINTER_DATA_TAG__AGGR = 0x21
ARCHI_POINTER_DATA_TAG__AGGR_TYPE = 0x28


class archi_layout_type_t(c.Structure):
    """Size and alignment requirement of a type.
    """
    _fields_ = [('size', c.c_size_t),
                ('alignment', c.c_size_t)]

    def __init__(self, c_type):
        self.size = c.sizeof(c_type)
        self.alignment = c.alignment(c_type)


class archi_layout_struct_t(c.Structure):
    """Layout of a structure in memory.
    """
    _fields_ = [('base', archi_layout_type_t),
                ('fam_stride', c.c_size_t)]

    def __init__(self, c_type, fam_elt_type):
        self.base = archi_layout_type_t(c_type)
        self.fam_stride = c.sizeof(fam_elt_type)


class archi_layout_array_t(c.Structure):
    """Size, alignment, and length of an array.
    """
    _fields_ = [('base', archi_layout_type_t),
                ('length', c.c_size_t)]

    def __init__(self, c_type):
        if issubclass(c_type, c.Array):
            self.base = archi_layout_type_t(c_type._type_)
            self.length = c_type._length_
        else:
            self.base = archi_layout_type_t(c_type)
            self.length = 1


class archi_aggr_member_type__value_t(c.Structure):
    """Description of a value type of an aggregate member.
    """
    _fields_ = [('layout', archi_layout_type_t),
                ('value_tag', archi_pointer_attr_t)]

    def __init__(self, value_type, /, value_tag=0):
        if not isinstance(value_tag, int):
            raise TypeError
        elif value_tag < 0 or value_tag > archi_pointer_attr_t.DATA_TAG_MAX:
            raise ValueError

        self.layout = archi_layout_type_t(value_type)
        self.value_tag = value_tag


class archi_aggr_member_type__pointer_t(c.Structure):
    """Description of a pointer type of an aggregate member.
    """
    class union(c.Union):
        _fields_ = [('pointee_tag', archi_pointer_attr_t),
                    ('pointee_layout', archi_layout_array_t)]

    _anonymous_ = ['u']
    _fields_ = [('layout', archi_layout_type_t),
                ('pointee_kind', c.c_ubyte),
                ('u', union)]

    def __init__(self, ptr_type, pointee_kind, /, pointee_type=None, pointee_tag=0):
        if pointee_kind < archi_pointer_t.POINTEE__ANY \
                or pointee_kind > archi_pointer_t.POINTEE__FUNC:
            raise ValueError
        elif not isinstance(pointee_tag, int):
            raise TypeError
        elif pointee_tag < 0 or pointee_tag > archi_pointer_attr_t.DATA_TAG_MAX:
            raise ValueError

        self.layout = archi_layout_type_t(ptr_type)
        self.pointee_kind = pointee_kind

        if pointee_kind == archi_pointer_t.POINTEE__PDATA:
            self.pointee_layout = archi_layout_array_t(pointee_type)
        elif pointee_kind == archi_pointer_t.POINTEE__CDATA \
                or pointee_kind == archi_pointer_t.POINTEE__FUNC:
            self.pointee_tag = pointee_tag


class archi_aggr_member_type__aggregate_t(c.Structure):
    """Description of an aggregate member type.
    """
    def __init__(self, c_type):
        self.layout = archi_layout_type_t(c_type)


class archi_aggr_member_t(c.Structure):
    """Description of an aggregate member.
    """
    KIND_VALUE = 0
    KIND_POINTER = 1
    KIND_AGGREGATE = 2

    class union(c.Union):
        _fields_ = [('layout', c.POINTER(archi_layout_type_t)),
                    ('value', c.POINTER(archi_aggr_member_type__value_t)),
                    ('pointer', c.POINTER(archi_aggr_member_type__pointer_t)),
                    ('aggregate', c.POINTER(archi_aggr_member_type__aggregate_t))]

    _anonymous_ = ['u']
    _fields_ = [('name', c.c_char_p),
                ('offset', c.c_size_t),
                ('length', c.c_size_t),
                ('kind', c.c_int),
                ('u', union)]

    def __init__(self, offset, length=1):
        if not isinstance(offset, int):
            raise TypeError
        elif offset < 0:
            raise ValueError
        elif not isinstance(length, int):
            raise TypeError
        elif length < 0:
            raise ValueError

        self.offset = offset
        self.length = length


archi_aggr_member_type__aggregate_t._fields_ = \
        [('layout', archi_layout_type_t),
         ('num_members', c.c_size_t),
         ('members', c.POINTER(archi_aggr_member_t))]


class archi_aggr_type_t(c.Structure):
    """Description of an aggregate type.
    """
    TAG = ARCHI_POINTER_DATA_TAG__AGGR_TYPE

    _fields_ = [('top_level', archi_aggr_member_type__aggregate_t),
                ('init_value', c.c_void_p),
                ('init_value_fam', c.c_void_p)]

    def __init__(self, c_type):
        self.top_level = archi_aggr_member_type__aggregate_t(c_type)

##############################################################################
# Directed execution graphs
##############################################################################

ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE = 0x30
ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY = 0x31
ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION = 0x30
ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION = 0x31


archi_dexgraph_branch_index_t = c.c_size_t

##############################################################################
# Concurrent processing
##############################################################################

ARCHI_POINTER_DATA_TAG__THREAD_GROUP = 0x40
ARCHI_POINTER_DATA_TAG__THREAD_LFQUEUE = 0x41
ARCHI_POINTER_FUNC_TAG__THREAD_WORK = 0x40
ARCHI_POINTER_FUNC_TAG__THREAD_CALLBACK = 0x41


class archi_thread_group_start_params_t(c.Structure):
    """Thread group creation parameters.
    """
    _fields_ = [('num_threads', c.c_size_t)]

    def __init__(self, num_threads):
        self.num_threads = num_threads


class archi_thread_lfqueue_alloc_params_t(c.Structure):
    """Lock-free queue allocation parameters.
    """
    _fields_ = [('capacity', c.c_size_t),
                ('elt_size', c.c_size_t)]

    def __init__(self, capacity, elt_size=0):
        if capacity <= 0 or capacity & (capacity - 1) != 0:
            raise ValueError
        elif elt_size < 0:
            raise ValueError

        self.capacity = capacity
        self.elt_size = elt_size

##############################################################################
# Signal management
##############################################################################

ARCHI_POINTER_DATA_TAG__SIGNAL_SET = 0x50
ARCHI_POINTER_DATA_TAG__SIGNAL_FLAGS = 0x51
ARCHI_POINTER_FUNC_TAG__SIGNAL_HANDLER = 0x50
ARCHI_POINTER_DATA_TAG__SIGNAL_HANDLER_DATA__HASHMAP = 0x58


SIGNALS = ['SIGINT', 'SIGQUIT', 'SIGTERM',              # interruption events
           'SIGCHLD', 'SIGCONT', 'SIGTSTP',             # process events
           'SIGXCPU', 'SIGXFSZ',                        # limit exceeding events
           'SIGPIPE', 'SIGPOLL', 'SIGURG',              # input/output events
           'SIGALRM', 'SIGVTALRM', 'SIGPROF',           # timer events
           'SIGHUP', 'SIGTTIN', 'SIGTTOU', 'SIGWINCH',  # terminal events
           'SIGUSR1', 'SIGUSR2']                        # user-defined events
SIGNAL_RT_MIN = 'SIGRTMIN' # minimum real-time signal
SIGNAL_RT_MAX = 'SIGRTMAX' # maximum real-time signal

NUM_RT_SIGNALS = (signal.SIGRTMAX - signal.SIGRTMIN + 1) if signal.SIGRTMIN <= signal.SIGRTMAX else 0


archi_signal_set_mask_t = c.c_uint32

NUM_SET_MASKS = (len(SIGNALS) + NUM_RT_SIGNALS + (32 - 1)) // 32


class archi_signal_set_t(archi_signal_set_mask_t * NUM_SET_MASKS):
    """Set of POSIX signals.
    """
    class _realtime_signal:
        def __init__(self, signal_set, from_max):
            self._signal_set = signal_set
            self._from_max = from_max

        def __getitem__(self, index):
            if not self._from_max:
                if index < 0 or index >= NUM_RT_SIGNALS:
                    raise IndexError(f"Real-time signal {SIGNAL_RT_MIN}{index:+} is out of supported range")
            else:
                if index > 0 or index <= -NUM_RT_SIGNALS:
                    raise IndexError(f"Real-time signal {SIGNAL_RT_MAX}{index:+} is out of supported range")

                index += (NUM_RT_SIGNALS - 1)

            index += len(SIGNALS)
            return self._signal_set[index // 32] & (1 << (index % 32)) != 0

        def __setitem__(self, index, value):
            if not isinstance(value, bool):
                raise TypeError

            if not self._from_max:
                if index < 0 or index >= NUM_RT_SIGNALS:
                    raise IndexError(f"Real-time signal {SIGNAL_RT_MIN}{index:+} is out of supported range")
            else:
                if index > 0 or index <= -NUM_RT_SIGNALS:
                    raise IndexError(f"Real-time signal {SIGNAL_RT_MAX}{index:+} is out of supported range")

                index += (NUM_RT_SIGNALS - 1)

            index += len(SIGNALS)

            if not value:
                self._signal_set[index // 32] &= ~(1 << (index % 32))
            else:
                self._signal_set[index // 32] |= 1 << (index % 32)

    def __init__(self, **kwargs):
        self._rtmin = _realtime_signal(self, False)
        self._rtmax = _realtime_signal(self, True)

        super().__init__(**kwargs)

    def __getattr__(self, key):
        """Check if the specified signal is in the set.
        """
        try:
            index = SIGNALS.index(key)
            return self[index // 32] & (1 << (index % 32)) != 0
        except ValueError:
            if key == SIGNAL_RT_MIN:
                return self._rtmin
            elif key == SIGNAL_RT_MAX:
                return self._rtmax
            else:
                raise AttributeError("Unknown signal '{key}'")

    def __setattr__(self, key, value):
        if not isinstance(value, bool):
            raise TypeError

        try:
            index = SIGNALS.index(key)

            if not value:
                self[index // 32] &= ~(1 << (index % 32))
            elif value:
                self[index // 32] |= 1 << (index % 32)
        except ValueError:
            if key == SIGNAL_RT_MIN or key == SIGNAL_RT_MAX:
                raise AttributeError("Cannot set real-time signal without an index")
            else:
                raise AttributeError("Unknown signal '{key}'")


class archi_signal_handler_t(c.Structure):
    """Signal handler.
    """
    _fields_ = [('function', c.CFUNCTYPE(None)),
                ('data', c.c_void_p)]

##############################################################################
# Memory handling
##############################################################################

ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE = 0x60
ARCHI_POINTER_DATA_TAG__MEMORY = 0x61
ARCHI_POINTER_DATA_TAG__MEMORY_MAPPING = 0x62

##############################################################################
# Files
##############################################################################

ARCHI_POINTER_DATA_TAG__FILE_STREAM = 0x70


archi_file_descriptor_t = c.c_int


class archi_file_open_params_t(c.Structure):
    """File opening parameters.
    """
    _fields_ = [('size', c.c_size_t),
                ('readable', c.c_bool),
                ('writable', c.c_bool),
                ('create', c.c_bool),
                ('exclusive', c.c_bool),
                ('truncate', c.c_bool),
                ('append', c.c_bool),
                ('flags', c.c_int),
                ('mode', c.c_int)]

    def __init__(self, size=0, readable=False, writable=False, create=False,
                 exclusive=False, truncate=False, append=False, flags=0, mode=0):
        if size < 0:
            raise ValueError
        elif flags < 0:
            raise ValueError
        elif mode < 0:
            raise ValueError

        self.size = size
        self.readable = readable
        self.writable = writable
        self.create = create
        self.exclusive = exclusive
        self.truncate = truncate
        self.append = append
        self.flags = flags
        self.mode = mode


class archi_file_map_params_t(c.Structure):
    """File mapping parameters.
    """
    _fields_ = [('size', c.c_size_t),
                ('offset', c.c_size_t),
                ('ptr_support', c.c_bool),
                ('readable', c.c_bool),
                ('writable', c.c_bool),
                ('shared', c.c_bool),
                ('flags', c.c_int)]

    def __init__(self, size=0, offset=0, ptr_support=False,
                 readable=False, writable=False, shared=False, flags=0):
        if size < 0:
            raise ValueError
        elif offset < 0:
            raise ValueError
        elif flags < 0:
            raise ValueError

        self.size = size
        self.offset = offset
        self.ptr_support = ptr_support
        self.readable = readable
        self.writable = writable
        self.shared = shared
        self.flags = flags

##############################################################################
# Shared libraries
##############################################################################

ARCHI_POINTER_DATA_TAG__LIBRARY_HANDLE = 0x80


class archi_library_load_params_t(c.Structure):
    """Shared library loading parameters.
    """
    _fields_ = [('lazy', c.c_bool),
                ('global', c.c_bool),
                ('flags', c.c_int)]

    def __init__(self, lazy=False, globl=False, flags=0):
        if flags < 0:
            raise ValueError

        self.lazy = lazy
        setattr(self, 'global', globl)
        self.flags = flags

##############################################################################
# Hashmaps
##############################################################################

ARCHI_POINTER_DATA_TAG__HASHMAP = 0x90
ARCHI_POINTER_FUNC_TAG__HASHMAP_HASH = 0x90


class archi_hashmap_alloc_params_t(c.Structure):
    """Hashmap allocation parameters.
    """
    _fields_ = [('capacity', c.c_size_t)]

    def __init__(self, capacity):
        if capacity < 0:
            raise ValueError

        self.capacity = capacity

##############################################################################
# Timers
##############################################################################

ARCHI_POINTER_DATA_TAG__TIMER = 0xB0

##############################################################################
# Types used in input files
##############################################################################

class archi_file_ptr_support_header_t(c.Structure):
    """Header of a memory-mapped file.
    """
    _fields_ = [('addr', c.c_void_p),
                ('size', c.c_size_t)]


class archi_app_input_file_header_t(c.Structure):
    """Description of an application input file.
    """
    MAGIC = b'[archi]\x00'

    _fields_ = [('header', archi_file_ptr_support_header_t),
                ('magic', c.c_char * len(MAGIC)),
                ('contents', c.POINTER(archi_kvlist_t))]

    def __init__(self):
        self.magic = type(self).MAGIC

##############################################################################
# Registry operations: auxiliary structures
##############################################################################

class archi_app_registry_op_data_params_t(c.Structure):
    """Parameter list description for context registry operation data.
    """
    _fields_ = [('context_key', c.c_char_p),
                ('list', c.POINTER(archi_kvlist_t))]

##############################################################################
# Registry operations: data for basic context registry operations
##############################################################################

class archi_app_registry_op_data__delete_t(c.Structure):
    """Registry operation data: delete a context.
    """
    _fields_ = [('key', c.c_char_p)]


class archi_app_registry_op_data__alias_t(c.Structure):
    """Registry operation data: create a context alias.
    """
    _fields_ = [('key', c.c_char_p),
                ('original_key', c.c_char_p)]


class archi_app_registry_op_data__create_as_t(c.Structure):
    """Registry operation data: create a context using interface of another context.
    """
    _fields_ = [('key', c.c_char_p),
                ('sample_key', c.c_char_p),
                ('init_params', archi_app_registry_op_data_params_t)]


class archi_app_registry_op_data__create_from_t(c.Structure):
    """Registry operation data: create a context using interface obtained from another context slot.
    """
    _fields_ = [('key', c.c_char_p),
                ('source_key', c.c_char_p),
                ('source_slot', archi_context_slot_t),
                ('init_params', archi_app_registry_op_data_params_t)]


class archi_app_registry_op_data__create_params_t(c.Structure):
    """Registry operation data: create a parameter list context.
    """
    _fields_ = [('key', c.c_char_p),
                ('params', archi_app_registry_op_data_params_t)]


class archi_app_registry_op_data__create_ptr_t(c.Structure):
    """Registry operation data: create a pointer context.
    """
    _fields_ = [('key', c.c_char_p),
                ('pointee', archi_pointer_t)]


class archi_app_registry_op_data__create_dptr_array_t(c.Structure):
    """Registry operation data: create a data pointer array context.
    """
    _fields_ = [('key', c.c_char_p),
                ('length', c.c_size_t)]

    def __init__(self, length):
        self.length = length


class archi_app_registry_op_data__invoke_t(c.Structure):
    """Registry operation data: invoke context call.
    """
    _fields_ = [('key', c.c_char_p),
                ('slot', archi_context_slot_t),
                ('call_params', archi_app_registry_op_data_params_t)]


class archi_app_registry_op_data__unassign_t(c.Structure):
    """Registry operation data: unset context slot.
    """
    _fields_ = [('key', c.c_char_p),
                ('slot', archi_context_slot_t)]


class archi_app_registry_op_data__assign_t(c.Structure):
    """Registry operation data: set context slot to value.
    """
    _fields_ = [('key', c.c_char_p),
                ('slot', archi_context_slot_t),
                ('value', archi_pointer_t)]


class archi_app_registry_op_data__assign_slot_t(c.Structure):
    """Registry operation data: set context slot to value of another context slot.
    """
    _fields_ = [('key', c.c_char_p),
                ('slot', archi_context_slot_t),
                ('source_key', c.c_char_p),
                ('source_slot', archi_context_slot_t)]


class archi_app_registry_op_data__assign_call_t(c.Structure):
    """Registry operation data: set context slot to result of another context call.
    """
    _fields_ = [('key', c.c_char_p),
                ('slot', archi_context_slot_t),
                ('source_key', c.c_char_p),
                ('source_slot', archi_context_slot_t),
                ('source_call_params', archi_app_registry_op_data_params_t)]

##############################################################################
# PSFv2 font
##############################################################################

ARCHI_POINTER_DATA_TAG__FONT_PSF2 = 0x864ab572

##############################################################################
# SDL2
##############################################################################

ARCHI_POINTER_DATA_TAG__SDL2_WINDOW = 0x800
ARCHI_POINTER_DATA_TAG__SDL2_RENDERER = 0x801
ARCHI_POINTER_DATA_TAG__SDL2_TEXTURE = 0x802

##############################################################################
# OpenCL
##############################################################################

ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID = 0x1000
ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID = 0x1001
ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT = 0x1002
ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE = 0x1003
ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM = 0x1004
ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL = 0x1005
ARCHI_POINTER_DATA_TAG__OPENCL_EVENT = 0x1006
ARCHI_POINTER_DATA_TAG__OPENCL_MEM_OBJECT = 0x1007
ARCHI_POINTER_DATA_TAG__OPENCL_SVM = 0x1008
ARCHI_POINTER_DATA_TAG__OPENCL_SVM_ALLOC_DATA = 0x1009
ARCHI_POINTER_DATA_TAG__OPENCL_SVM_MAP_DATA = 0x100A

