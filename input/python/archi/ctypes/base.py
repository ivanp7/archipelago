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
# @brief C types of Archipelago base.

import ctypes as c


archi_status_t = c.c_int


class archi_array_layout_t(c.Structure):
    """Array layout description.
    """
    _fields_ = [('num_of', c.c_size_t),
                ('size', c.c_size_t),
                ('alignment', c.c_size_t)]

    def __init__(self, num_of: 'int' = 0, size: 'int' = 0, alignment: 'int' = 0, /):
        """Initialize an instance.
        """
        super().__init__()

        if num_of < 0 or size < 0 or alignment < 0:
            raise ValueError
        elif size == 0 and num_of > 1:
            raise ValueError
        elif alignment != 0 and (alignment & (alignment - 1)) != 0:
            raise ValueError

        self.num_of = num_of
        self.size = size
        self.alignment = alignment


archi_pointer_flags_t = c.c_uint64


class archi_pointer_t(c.Structure):
    """Generic wrapper for data or function pointers with metadata.
    """
    class union(c.Union):
        """Union of a generic pointer to data and a generic pointer to function.
        """
        _fields_ = [('ptr', c.c_void_p),
                    ('fptr', c.CFUNCTYPE(None))]

    NUM_FLAG_BITS = 64 - 2

    _anonymous_ = ['as']
    _fields_ = [('as', union),
                ('ref_count', c.c_void_p),
                ('flags', archi_pointer_flags_t),
                ('element', archi_array_layout_t)]


class archi_parameter_list_t(c.Structure):
    """List of named values.
    """
    pass

archi_parameter_list_t._fields_ = \
        [('next', c.POINTER(archi_parameter_list_t)),
         ('name', c.c_char_p),
         ('value', archi_pointer_t)]


###############################################################################

class archi_context_slot_t(c.Structure):
    """Context slot designator.
    """
    _fields_ = [('name', c.c_char_p),
                ('index', c.POINTER(c.c_size_t)),
                ('num_indices', c.c_size_t)]


archi_context_init_func_t = c.CFUNCTYPE(archi_status_t,
                                        c.POINTER(c.POINTER(archi_pointer_t)),
                                        c.POINTER(archi_parameter_list_t))

archi_context_final_func_t = c.CFUNCTYPE(None,
                                         c.POINTER(archi_pointer_t))

archi_context_get_func_t = c.CFUNCTYPE(archi_status_t,
                                       c.POINTER(archi_pointer_t),
                                       archi_context_slot_t,
                                       c.POINTER(archi_pointer_t))

archi_context_set_func_t = c.CFUNCTYPE(archi_status_t,
                                       c.POINTER(archi_pointer_t),
                                       archi_context_slot_t,
                                       archi_pointer_t)

archi_context_act_func_t = c.CFUNCTYPE(archi_status_t,
                                       c.POINTER(archi_pointer_t),
                                       archi_context_slot_t,
                                       c.POINTER(archi_parameter_list_t))


class archi_context_interface_t(c.Structure):
    """Context interface functions.
    """
    _fields_ = [('init_fn', archi_context_init_func_t),
                ('final_fn', archi_context_final_func_t),
                ('get_fn', archi_context_get_func_t),
                ('set_fn', archi_context_set_func_t),
                ('act_fn', archi_context_act_func_t)]

###############################################################################

class archi_file_header_t(c.Structure):
    """Header of a memory-mapped file.
    """
    _fields_ = [('addr', c.c_void_p),
                ('end', c.c_void_p)]


class archi_exe_input_file_header_t(c.Structure):
    """Description of an input file for the executable.
    """
    MAGIC = "[archi]"

    _fields_ = [('header', archi_file_header_t),
                ('magic', c.c_char * 8),
                ('contents', c.POINTER(archi_parameter_list_t))]

    def __init__(self):
        self.magic = type(self).MAGIC.encode() + b'\x00'

