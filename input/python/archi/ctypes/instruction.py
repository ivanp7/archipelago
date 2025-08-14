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
# @brief C types for instructions in Archipelago initialization files.

import ctypes as c

from .base import archi_pointer_flags_t, archi_pointer_t, \
        archi_parameter_list_t, archi_context_slot_t


class archi_exe_registry_instr_base_t(c.Structure):
    """Context registry instruction base.
    """
    _fields_ = [('type', c.c_int)]


class archi_exe_registry_instr_list_t(c.Structure):
    """Context registry instruction list.
    """
    pass

archi_exe_registry_instr_list_t._fields_ = \
        [('next', c.POINTER(archi_exe_registry_instr_list_t)),
         ('instruction', c.POINTER(archi_exe_registry_instr_base_t))]

###############################################################################

class archi_exe_registry_instr_init_from_context_t(c.Structure):
    """Context registry instruction: initialize a new context using interface of a source context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('interface_origin_key', c.c_char_p),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]


class archi_exe_registry_instr_init_from_slot_t(c.Structure):
    """Context registry instruction: initialize a new context using interface from a context slot.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('interface_origin_key', c.c_char_p),
                ('interface_origin_slot', archi_context_slot_t),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]


class archi_exe_registry_instr_init_pointer_t(c.Structure):
    """Context registry instruction: initialize a new pointer context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('value', archi_pointer_t)]


class archi_exe_registry_instr_init_array_t(c.Structure):
    """Context registry instruction: initialize a new array context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('num_elements', c.c_size_t),
                ('flags', archi_pointer_flags_t)]


class archi_exe_registry_instr_copy_t(c.Structure):
    """Context registry instruction: create a context alias.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('original_key', c.c_char_p)]


class archi_exe_registry_instr_delete_t(c.Structure):
    """Context registry instruction: delete a context from registry.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p)]


class archi_exe_registry_instr_set_to_value_t(c.Structure):
    """Context registry instruction: set context slot to pointer to a value.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_slot_t),
                ('value', archi_pointer_t)]


class archi_exe_registry_instr_set_to_context_data_t(c.Structure):
    """Context registry instruction: set context slot to pointer to a source context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_slot_t),
                ('source_key', c.c_char_p)]


class archi_exe_registry_instr_set_to_context_slot_t(c.Structure):
    """Context registry instruction: set context slot to a source context slot.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_slot_t),
                ('source_key', c.c_char_p),
                ('source_slot', archi_context_slot_t)]


class archi_exe_registry_instr_act_t(c.Structure):
    """Context registry instruction: invoke context action.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('action', archi_context_slot_t),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]

