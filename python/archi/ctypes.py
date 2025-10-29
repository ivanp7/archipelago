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
# @brief C bindings of the Archipelago library.

import ctypes as c
import enum

##############################################################################
# Base types
##############################################################################

archi_pointer_attr_t = c.c_uint64


def pointer_attr_transp_data(length: 'int', stride: 'int', alignment: 'int',
                     writable: 'bool' = False) -> 'archi_pointer_attr_t':
    """Compute attributes for a transparent data type.
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

    SIZE_MAX   = (1 << 56) - 1  # 0x00FFFFFFFFFFFFFF
    STRIDE_MAX =  1 << (56 - 1) # 0x0080000000000000

    if length * stride > SIZE_MAX:
        raise ValueError("Size (length*stride) exceeds maximum supported value of {SIZE_MAX} bytes")
    elif stride > STRIDE_MAX:
        raise ValueError("Stride exceeds maximum supported value of {STRIDE_MAX} bytes")

    stride_width = (stride - 1).bit_length()

    attr_memtype = (0x1 if writable else 0x2) << 62
    attr_stride_width = stride_width << 56
    attr_stride_over_alignment = (stride / alignment - 1) << (56 - stride_width)
    attr_length = length

    return archi_pointer_attr_t(attr_memtype | attr_stride_width |
                                attr_stride_over_alignment | attr_length)


def pointer_attr_opaque_data(tag: 'int' = 0, writable: 'bool' = False) -> 'archi_pointer_attr_t':
    """Compute attributes for an opaque data type.
    """
    if not isinstance(tag, int) or not isinstance(writable, bool):
        raise TypeError

    if tag < 0:
        raise ValueError("Tag is negative")

    TAG_MAX = ~(56 << 56) & ((1 << 62) - 1) # 0x07FFFFFFFFFFFFFF

    if tag > TAG_MAX:
        raise ValueError("Tag exceeds maximum supported value of {TAG_MAX}")

    attr_memtype = (0x1 if writable else 0x2) << 62
    attr_tag = (~tag) & ((1 << 62) - 1)

    return archi_pointer_attr_t(attr_memtype | attr_tag)


class archi_pointer_t(c.Structure):
    """Generic wrapper for data pointers with attributes.
    """
    _fields_ = [('ptr', c.c_void_p),
                ('attr', archi_pointer_attr_t)]

    def assign(self, obj, /, writable: 'bool' = False):
        """Assign pointer fields to refer the specified object.
        """
        if not isinstance(writable, bool):
            raise TypeError

        if obj is not None:
            self.ptr = obj.address()

            if obj.tag() is not None:
                self.attr = pointer_attr_transp_data(obj.length(), obj.stride(),
                                                     obj.alignment(), writable)
            else:
                self.attr = pointer_attr_opaque_data(obj.tag(), writable)
        else:
            self.ptr = None
            self.attr = 0


class archi_kvlist_t(c.Structure):
    """List of key-value pairs.
    """
    pass

archi_kvlist_t._fields_ = \
        [('next', c.POINTER(archi_kvlist_t)),
         ('key', c.c_char_p),
         ('value', archi_pointer_t)]

##############################################################################
# Types used in input files
##############################################################################

class archi_file_ptr_support_header_t(c.Structure):
    """Header of a memory-mapped file.
    """
    _fields_ = [('addr', c.c_void_p),
                ('size', c.c_size_t)]


class archi_exe_input_file_header_t(c.Structure):
    """Description of an input file for the executable.
    """
    MAGIC = "[archi]"

    _fields_ = [('header', archi_file_ptr_support_header_t),
                ('magic', c.c_char * (len(MAGIC) + 1)),
                ('contents', c.POINTER(archi_kvlist_t))]

    def __init__(self):
        self.magic = type(self).MAGIC.encode() + b'\x00'

##############################################################################
# Registry operations: auxiliary structures
##############################################################################

class archi_context_slot_t(c.Structure):
    """Context slot designator.
    """
    _fields_ = [('name', c.c_char_p),
                ('index', c.POINTER(c.c_ssize_t)),
                ('num_indices', c.c_size_t)]


class archi_context_registry_op_data_key_slot_t(c.Structure):
    """Key-slot pair for context registry operation data.
    """
    _fields_ = [('key', c.c_char_p),
                ('slot', archi_context_slot_t)]


class archi_context_registry_op_data_params_t(c.Structure):
    """Parameter list description for context registry operation data.
    """
    _fields_ = [('list_key', c.c_char_p),
                ('list', c.POINTER(archi_kvlist_t))]


class archi_context_registry_op_data_init_info_t(c.Structure):
    """Context initialization info for context registry operation data.
    """
    _fields_ = [('key', c.c_char_p),
                ('init_params', archi_context_registry_op_data_params_t)]

##############################################################################
# Registry operations: data for basic context registry operations
##############################################################################

class archi_context_registry_op_data__delete_t(c.Structure):
    """Context registry operation data: delete a context.
    """
    _fields_ = [('key', c.c_char_p)]


class archi_context_registry_op_data__alias_t(c.Structure):
    """Context registry operation data: create a context alias.
    """
    _fields_ = [('alias_key', c.c_char_p),
                ('origin_key', c.c_char_p)]


class archi_context_registry_op_data__create_as_t(c.Structure):
    """Context registry operation data: create a context using interface of another context.
    """
    _fields_ = [('context', archi_context_registry_op_data_init_info_t),
                ('instance_key', c.c_char_p)]


class archi_context_registry_op_data__create_from_t(c.Structure):
    """Context registry operation data: create a context using interface obtained from another context slot.
    """
    _fields_ = [('context', archi_context_registry_op_data_init_info_t),
                ('slot', archi_context_registry_op_data_key_slot_t)]


class archi_context_registry_op_data__call_t(c.Structure):
    """Context registry operation data: invoke context call.
    """
    _fields_ = [('target', archi_context_registry_op_data_key_slot_t),
                ('call_params', archi_context_registry_op_data_params_t)]


class archi_context_registry_op_data__set_t(c.Structure):
    """Context registry operation data: set context slot to value.
    """
    _fields_ = [('target', archi_context_registry_op_data_key_slot_t),
                ('value', archi_pointer_t)]


class archi_context_registry_op_data__assign_t(c.Structure):
    """Context registry operation data: set context slot to value of another context slot.
    """
    _fields_ = [('target', archi_context_registry_op_data_key_slot_t),
                ('source', archi_context_registry_op_data_key_slot_t)]


class archi_context_registry_op_data__assign_call_t(c.Structure):
    """Context registry operation data: set context slot to result of another context call.
    """
    _fields_ = [('target', archi_context_registry_op_data_key_slot_t),
                ('source', archi_context_registry_op_data_key_slot_t),
                ('call_params', archi_context_registry_op_data_params_t)]

##############################################################################
# Registry operations: data for context registry operations for creating contexts of built-in interfaces
##############################################################################

class archi_context_registry_op_data__create_parameters_t(c.Structure):
    """Context registry operation data: create a parameters list context.
    """
    _fields_ = [('key', c.c_char_p),
                ('init_params', archi_context_registry_op_data_params_t)]


class archi_context_registry_op_data_pointer_type_t(enum.Enum):
    """Type of context interface for pointers.
    """
    GENERIC = 0
    DATA = enum.auto()
    TRANSP_DATA = enum.auto()
    OPAQUE_DATA = enum.auto()
    FUNCTION = enum.auto()


class archi_context_registry_op_data__create_pointer_to_value_t(c.Structure):
    """Context registry operation data: create a pointer context (to value).
    """
    _fields_ = [('key', c.c_char_p),
                ('value', archi_pointer_t),
                ('type', c.c_int), # expects one of values from archi_context_registry_op_data_pointer_type_t
                ('init_params_list', c.POINTER(archi_kvlist_t))]


class archi_context_registry_op_data__create_pointer_to_context_t(c.Structure):
    """Context registry operation data: create a pointer context (to context slot).
    """
    _fields_ = [('key', c.c_char_p),
                ('pointee', archi_context_registry_op_data_key_slot_t),
                ('type', c.c_int), # expects one of values from archi_context_registry_op_data_pointer_type_t
                ('init_params_list', c.POINTER(archi_kvlist_t))]


class archi_context_registry_op_data__create_dptr_array_t(c.Structure):
    """Context registry operation data: create a data pointer array context.
    """
    _fields_ = [('key', c.c_char_p),
                ('length', c.c_size_t)]

##############################################################################
# Signal management
##############################################################################

class archi_signal_watch_set_t(c.Structure):
    """Mask of POSIX signals to watch and handle.
    """
    import signal

    SIGNALS = ['SIGINT', 'SIGQUIT', 'SIGTERM',              # interruption events
               'SIGCHLD', 'SIGCONT', 'SIGTSTP',             # process events
               'SIGXCPU', 'SIGXFSZ',                        # limit exceeding events
               'SIGPIPE', 'SIGPOLL', 'SIGURG',              # input/output events
               'SIGALRM', 'SIGVTALRM', 'SIGPROF',           # timer events
               'SIGHUP', 'SIGTTIN', 'SIGTTOU', 'SIGWINCH',  # terminal events
               'SIGUSR1', 'SIGUSR2']                        # user-defined events
    NUM_RT_SIGNALS = signal.SIGRTMAX - signal.SIGRTMIN + 1

    _fields_ = [(f'f_{signal}', c.c_bool) for signal in SIGNALS] \
            + [('f_SIGRTMIN', c.c_bool * NUM_RT_SIGNALS)]

    def __init__(self, watch=set(), invert_normal=False, invert_realtime=False):
        default_normal = True
        default_realtime = True

        if invert_normal:
            default_normal = False
            for signal in archi_signal_watch_set_t.SIGNALS:
                setattr(self, f'f_{signal}', True)

        if invert_realtime:
            default_realtime = False
            for idx in range(archi_signal_watch_set_t.NUM_RT_SIGNALS):
                self.f_SIGRTMIN[idx] = True

        for signal in watch:
            if isinstance(signal, str):
                if signal not in archi_signal_watch_set_t.SIGNALS:
                    raise ValueError(f"Unknown signal '{signal}'")

                setattr(self, f'f_{signal}', default_normal)

            elif isinstance(signal, tuple) and len(signal) == 2 \
                    and isinstance(signal[0], str) and isinstance(signal[1], int):
                if signal[0] == 'SIGRTMIN':
                    if signal[1] < 0 or signal[1] >= archi_signal_watch_set_t.NUM_RT_SIGNALS:
                        raise ValueError("Real-time signal out of supported range")

                    self.f_SIGRTMIN[signal[1]] = default_realtime

                elif signal[0] == 'SIGRTMAX':
                    if signal[1] > 0 or signal[1] <= -archi_signal_watch_set_t.NUM_RT_SIGNALS:
                        raise ValueError("Real-time signal out of supported range")

                    self.f_SIGRTMIN[(archi_signal_watch_set_t.NUM_RT_SIGNALS-1) \
                            + signal[1]] = default_realtime

                else:
                    raise ValueError(f"Unknown signal '{signal[0]}'")

            else:
                raise ValueError("Unsupported signal signature")

