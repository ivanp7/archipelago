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
# @brief Built-in context types in Archipelago executable.

from ..app import Context, Parameters


BUILTIN_CONTEXT_REGISTRY = ('archi.registry', HashmapContext)
BUILTIN_CONTEXT_EXECUTABLE = ('archi.executable', ExecutableContext)
BUILTIN_CONTEXT_INPUT_FILE = ('archi.input_file', FileContext)
BUILTIN_CONTEXT_SIGNAL = ('archi.signal', SignalManagementContext)

# TODO

# archi_context_memory_interface
# archi_context_memory_mapping_interface

# archi_context_hsp_frame_interface
# archi_context_hsp_transition_interface
# archi_context_hsp_branch_state_data_interface
# archi_context_hsp_transition_attachment_data_interface

# archi_context_converter_string_to_number_interface

# archi_context_util_timer_interface

# archi_context_ds_hashmap_interface
# archi_context_ds_lfqueue_interface

# archi_context_ipc_env_interface
# archi_context_ipc_signal_handler_interface
# archi_context_ipc_signal_management_interface

# archi_context_res_file_interface
# archi_context_res_library_interface

# archi_context_res_thread_group_interface
# archi_context_res_thread_group_work_interface
# archi_context_res_thread_group_callback_interface
# archi_context_res_thread_group_dispatch_data_interface

