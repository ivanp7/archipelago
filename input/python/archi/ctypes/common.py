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
# @brief Common C types used in Archipelago initialization files.

import ctypes as c


class archi_array_layout_t(c.Structure):
    """Array layout description.
    """
    _fields_ = [('num_of', c.c_size_t),
                ('size', c.c_size_t),
                ('alignment', c.c_size_t)]


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


class archi_context_slot_t(c.Structure):
    """Context slot designator.
    """
    _fields_ = [('name', c.c_char_p),
                ('index', c.POINTER(c.c_size_t)),
                ('num_indices', c.c_size_t)]

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

