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
# @brief Python module: C types used in an Archipelago initialization file

import ctypes as c


class archi_array_layout_t(c.Structure):
    """Array layout description.
    """
    _fields_ = [('num_of', c.c_size_t),
                ('size', c.c_size_t),
                ('alignment', c.c_size_t)]


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
                ('flags', c.c_uint64),
                ('element', archi_array_layout_t)]


class archi_parameter_list_t(c.Structure):
    """List of named values.
    """
    pass

archi_parameter_list_t._fields_ = \
        [('next', c.POINTER(archi_parameter_list_t)),
         ('name', c.c_char_p),
         ('value', archi_pointer_t)]


class archi_context_op_designator_t(c.Structure):
    """Context operation designator.
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


class archi_exe_input_file_header_t(c.Structure):
    """Description of an input file for the executable.
    """
    MAGIC = "[archi]"

    _fields_ = [('header', archi_file_header_t),
                ('magic', c.c_char * 8),
                ('contents', c.POINTER(archi_parameter_list_t))]

###############################################################################

class archi_exe_registry_instr_init_from_context_t(c.Structure):
    """Context registry instruction: initialize a new context using interface of a source context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('interface_source_key', c.c_char_p),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]


class archi_exe_registry_instr_init_from_slot_t(c.Structure):
    """Context registry instruction: initialize a new context using interface from a context slot.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('interface_source_key', c.c_char_p),
                ('interface_source_slot', archi_context_op_designator_t),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]


class archi_exe_registry_instr_init_pointer_t(c.Structure):
    """Context registry instruction: initialize a new pointer context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('value', archi_pointer_t)]


class archi_exe_registry_instr_copy_t(c.Structure):
    """Context registry instruction: create a context alias.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('original_key', c.c_char_p)]


class archi_exe_registry_instr_final_t(c.Structure):
    """Context registry instruction: finalize a context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p)]


class archi_exe_registry_instr_set_to_value_t(c.Structure):
    """Context registry instruction: set context slot to pointer to a value.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_op_designator_t),
                ('value', archi_pointer_t)]


class archi_exe_registry_instr_set_to_context_data_t(c.Structure):
    """Context registry instruction: set context slot to pointer to a source context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_op_designator_t),
                ('source_key', c.c_char_p)]


class archi_exe_registry_instr_set_to_context_slot_t(c.Structure):
    """Context registry instruction: set context slot to a source context slot.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_op_designator_t),
                ('source_key', c.c_char_p),
                ('source_slot', archi_context_op_designator_t)]


class archi_exe_registry_instr_act_t(c.Structure):
    """Context registry instruction: invoke context action.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('action', archi_context_op_designator_t),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]

###############################################################################

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

