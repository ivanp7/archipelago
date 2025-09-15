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
# @brief C types for Archipelago hierarchical state processing.

import ctypes as c


class archi_hsp_branch_select_loop_data_t(c.Structure):
    """Data for the simple loop selector function.
    """
    _fields_ = [('num_iterations', c.c_size_t),
                ('iteration', c.c_size_t)]

    def __init__(self, num_iterations, /):
        if not isinstance(num_iterations, int):
            raise TypeError
        elif num_iterations < 0:
            raise ValueError

        self.num_iterations = num_iterations

