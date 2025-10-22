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

    def __init__(self, num_of: 'int' = 0, size: 'int' = 0, alignment: 'int' = 0):
        """Initialize an instance.
        """
        super().__init__()

        if not isinstance(num_of, int) or not isinstance(size, int) or not isinstance(alignment, int):
            raise TypeError

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


archi_function_t = c.CFUNCTYPE(None)


class archi_pointer_t(c.Structure):
    """Generic wrapper for data or function pointers with metadata.
    """
    class union(c.Union):
        """Union of a generic pointer to data and a generic pointer to function.
        """
        _fields_ = [('ptr', c.c_void_p),
                    ('fptr', archi_function_t)]

    NUM_FLAG_BITS = 64 - 2

    _anonymous_ = ['as']
    _fields_ = [('as', union),
                ('ref_count', c.c_void_p),
                ('flags', archi_pointer_flags_t),
                ('element', archi_array_layout_t)]

    def __init__(self, layout: 'archi_array_layout_t', flags: 'int' = 0):
        """Initialize an instance.
        """
        super().__init__()

        if not isinstance(layout, archi_array_layout_t):
            raise TypeError
        elif not isinstance(flags, int):
            raise TypeError
        elif (flags < 0) or (flags >= 1 << type(self).NUM_FLAG_BITS):
            raise ValueError(f"Pointer flags must fit into {type(self).NUM_FLAG_BITS} lowest bits")

        self.flags = flags
        self.element = layout


class archi_parameter_list_t(c.Structure):
    """List of named values.
    """
    pass

archi_parameter_list_t._fields_ = \
        [('next', c.POINTER(archi_parameter_list_t)),
         ('name', c.c_char_p),
         ('value', archi_pointer_t)]

