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

import ctypes as c

from archi.base.app import (
        Context, ContextWhitelistable, Parameters, ParametersWhitelistable,
        PointerContext, ArrayContext
        )
from archi.base.ctypes.base import archi_array_layout_t, archi_pointer_flags_t
from archi.base.ctypes.context import archi_context_interface_t

from .ctypes.signal import archi_signal_watch_set_t


class HSPFrameContext(ContextWhitelistable):
    """Context type for HSP frames.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'num_states': (c.c_size_t, lambda v: c.c_size_t(v)),
                }

    class ActionExecuteParameters(ParametersWhitelistable):
        PARAMETERS = {
                'transition_function': ...,
                'transition_data': ...,
                }

    INTERFACE_SYMBOL = 'archi_context_hsp_frame_interface'

    CONTEXT_TYPE = 'archi.hsp.frame'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_states': {0: c.c_size_t},
            'state': {1: 'archi.hsp.state'},
            'state.function': {1: ...},
            'state.data': {1: ...},
            'state.metadata': {1: ...},
            }

    SETTER_SLOT_TYPES = {
            'state.function': {1: ...},
            'state.data': {1: ...},
            'state.metadata': {1: ...},
            }

    ACTION_PARAMETER_CLASSES = {
            'execute': {0: ActionExecuteParameters},
            }


class HSPTransitionContext(ContextWhitelistable):
    """Context type for HSP transitions.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': ...,
                'data': ...,
                }

    INTERFACE_SYMBOL = 'archi_context_hsp_transition_interface'

    CONTEXT_TYPE = 'archi.hsp.transition'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'function': {0: ...},
            'data': {0: ...},
            }

    SETTER_SLOT_TYPES = {
            'function': {0: ...},
            'data': {0: ...},
            }


class HSPBranchStateDataContext(ContextWhitelistable):
    """Context type for HSP branch state data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'num_branches': (c.c_size_t, lambda v: c.c_size_t(v)),
                'selector_fn': ...,
                'selector_data': ...,
                }

    INTERFACE_SYMBOL = 'archi_context_hsp_branch_state_data_interface'

    CONTEXT_TYPE = 'archi.hsp.state.branch.data'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_branches': {0: c.c_size_t},
            'branch': {1: 'archi.hsp.frame'},
            'selector_fn': {0: ...},
            'selector_data': {0: ...},
            }

    SETTER_SLOT_TYPES = {
            'branch': {1: 'archi.hsp.frame'},
            'selector_fn': {0: ...},
            'selector_data': {0: ...},
            }


class HSPTransitionAttachmentDataContext(ContextWhitelistable):
    """Context type for HSP transition attachment data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'pre_function': ...,
                'pre_data': ...,
                'post_function': ...,
                'post_data': ...,
                }

    INTERFACE_SYMBOL = 'archi_context_hsp_transition_attachment_data_interface'

    CONTEXT_TYPE = 'archi.hsp.transition.attachment.data'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'pre.function': {0: ...},
            'pre.data': {0: ...},
            'post.function': {0: ...},
            'post.data': {0: ...},
            }

    SETTER_SLOT_TYPES = {
            'pre.function': {0: ...},
            'pre.data': {0: ...},
            'post.function': {0: ...},
            'post.data': {0: ...},
            }

###############################################################################

class HashmapContext(Context):
    """Context type for hashmaps.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': 'archi.hashmap.alloc_params',
                'capacity': (c.c_size_t, lambda v: c.c_size_t(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_hashmap_interface'

    INIT_PARAMETERS_CLASS = InitParameters

    @classmethod
    def context_type(cls):
        return 'archi.hashmap'

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if indices:
            raise KeyError
        return ...

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if len(indices) > 1:
            raise KeyError
        return ..., None

    @classmethod
    def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
        raise KeyError

###############################################################################

class LockFreeQueueContext(ContextWhitelistable):
    """Context type for lock-free queues.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': 'archi.lfqueue.alloc_params',
                'capacity_log2': (c.c_size_t, lambda v: c.c_size_t(v)),
                'element_size': (c.c_size_t, lambda v: c.c_size_t(v)),
                'element_alignment': (c.c_size_t, lambda v: c.c_size_t(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_lfqueue_interface'

    CONTEXT_TYPE = 'archi.lfqueue'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'capacity_log2': {0: c.c_size_t},
            'element_size': {0: c.c_size_t},
            'element_alignment': {0: c.c_size_t},
            }

###############################################################################

class EnvVarContext(ContextWhitelistable):
    """Context type for environmental variables.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'name': (str, lambda v: str(v)),
                'default_value': (str, lambda v: str(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_envvar_interface'

    CONTEXT_TYPE = str

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class SignalHandlerContext(ContextWhitelistable):
    """Context type for signal handlers.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': ...,
                'data': ...,
                }

    INTERFACE_SYMBOL = 'archi_context_signal_handler_interface'

    CONTEXT_TYPE = 'archi.signal.handler'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'function': {0: ...},
            'data': {0: ...},
            }

    SETTER_SLOT_TYPES = {
            'function': {0: ...},
            'data': {0: ...},
            }


class SignalManagementContext(Context):
    """Context type for signal management contexts.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'signals': archi_signal_watch_set_t,
                }

    INIT_PARAMETERS_CLASS = InitParameters

    @classmethod
    def context_type(cls):
        return 'archi.signal.management'

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if name == 'flags':
            if indices:
                raise KeyError
            return 'archi.signal.flags'
        elif name[:8] == 'handler.':
            if indices:
                raise KeyError
            return 'archi.signal.handler'
        else:
            raise KeyError

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if name[:8] == 'handler.':
            if indices:
                raise KeyError
            return ('archi.signal.handler', None)
        else:
            raise KeyError

###############################################################################

class MemoryContext(ContextWhitelistable):
    """Context type for memory objects.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'interface': 'archi.memory.interface',
                'alloc_data': ...,
                'layout': (archi_array_layout_t, lambda v: archi_array_layout_t(*v)),
                'num_elements': (c.c_size_t, lambda v: c.c_size_t(v)),
                'element_size': (c.c_size_t, lambda v: c.c_size_t(v)),
                'element_alignment': (c.c_size_t, lambda v: c.c_size_t(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_memory_interface'

    CONTEXT_TYPE = 'archi.memory'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'interface': {0: 'archi.memory.interface'},
            'allocation': {0: ...},
            'layout': {0: archi_array_layout_t},
            'num_elements': {0: c.c_size_t},
            'element_size': {0: c.c_size_t},
            'element_alignment': {0: c.c_size_t},
            'full_size': {0: c.c_size_t},
            }


class MemoryMappingContext(ContextWhitelistable):
    """Context type for memory mappings.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'memory': 'archi.memory',
                'map_data': ...,
                'offset': (c.c_size_t, lambda v: c.c_size_t(v)),
                'num_elements': (c.c_size_t, lambda v: c.c_size_t(v)),
                'writeable': (c.c_byte, lambda v: c.c_byte(bool(v))),
                }

    class ActionCopyParameters(ParametersWhitelistable):
        PARAMETERS = {
                'source': ...,
                'source_offset': (c.c_size_t, lambda v: c.c_size_t(v)),
                'num_elements': (c.c_size_t, lambda v: c.c_size_t(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_memory_mapping_interface'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'memory': {0: 'archi.memory'},
            '': {0: ..., 1: ...},
            'layout': {0: archi_array_layout_t},
            'num_elements': {0: c.c_size_t},
            'element_size': {0: c.c_size_t},
            'element_alignment': {0: c.c_size_t},
            'full_size': {0: c.c_size_t},
            }

    ACTION_PARAMETER_CLASSES = {
            'copy': {0: ActionCopyParameters,
                     1: ActionCopyParameters},
            }

###############################################################################

class FileContext(ContextWhitelistable):
    """Context type for files.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': 'archi.file.open_params',
                'size': (c.c_size_t, lambda v: c.c_size_t(v)),
                'pathname': (str, lambda v: str(v)),
                'create': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'exclusive': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'truncate': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'readable': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'writable': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'nonblock': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'flags': (c.c_int, lambda v: c.c_int(v)),
                'mode': (c.c_int, lambda v: c.c_int(v)),
                }

    class ActionMapParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': 'archi.file.map_params',
                'size': (c.c_size_t, lambda v: c.c_size_t(v)),
                'offset': (c.c_size_t, lambda v: c.c_size_t(v)),
                'has_header': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'readable': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'writable': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'shared': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'flags': (c.c_int, lambda v: c.c_int(v)),
                'close_fd': (c.c_byte, lambda v: c.c_byte(bool(v))),
                }

    INTERFACE_SYMBOL = 'archi_context_file_interface'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'fd': {0: c.c_int},
            'map': {0: ...},
            }

    ACTION_PARAMETER_CLASSES = {
            'map': {0: ActionMapParameters},
            }

###############################################################################

class LibraryContext(Context):
    """Context type for shared libraries.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': 'archi.library.load_params',
                'pathname': (str, lambda v: str(v)),
                'lazy': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'global': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'flags': (c.c_int, lambda v: c.c_int(v)),
                }

    class ActionSymbolParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'flags': (archi_pointer_flags_t, lambda v: archi_pointer_flags_t(v)),
                'layout': (archi_array_layout_t, lambda v: archi_array_layout_t(*v)),
                'num_elements': (c.c_size_t, lambda v: c.c_size_t(v)),
                'element_size': (c.c_size_t, lambda v: c.c_size_t(v)),
                'element_alignment': (c.c_size_t, lambda v: c.c_size_t(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_library_interface'

    INIT_PARAMETERS_CLASS = InitParameters

    SYMBOLS = {}

    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        if not isinstance(cls.SYMBOLS, dict):
            raise TypeError
        elif not all(isinstance(key, str) for key in cls.SYMBOLS.keys()):
            raise TypeError

        super().__init_subclass__()

    @classmethod
    def context_type(cls):
        return 'archi.library'

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if indices:
            raise KeyError
        return cls.SYMBOLS.get(name, ...)

    @classmethod
    def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
        if indices:
            raise KeyError
        return ActionSymbolParameters

###############################################################################

class ThreadGroupContext(ContextWhitelistable):
    """Context type for thread groups.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': 'archi.thread_group.start_params',
                'num_threads': (c.c_size_t, lambda v: c.c_size_t(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_thread_group_interface'

    CONTEXT_TYPE = 'archi.thread_group'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_threads': {0: c.c_size_t},
            }


class ThreadGroupWorkContext(ContextWhitelistable):
    """Context type for thread group work description.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': ...,
                'data': ...,
                'size': (c.c_size_t, lambda v: c.c_size_t(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_thread_group_work_interface'

    CONTEXT_TYPE = 'archi.thread_group.work'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'function': {0: ...},
            'data': {0: ...},
            'size': {0: c.c_size_t},
            }

    SETTER_SLOT_TYPES = {
            'function': {0: ...},
            'data': {0: ...},
            'size': {0: (c.c_size_t, lambda v: c.c_size_t(v))},
            }


class ThreadGroupCallbackContext(ContextWhitelistable):
    """Context type for thread group callback description.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': ...,
                'data': ...,
                }

    INTERFACE_SYMBOL = 'archi_context_thread_group_callback_interface'

    CONTEXT_TYPE = 'archi.thread_group.callback'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'function': {0: ...},
            'data': {0: ...},
            }

    SETTER_SLOT_TYPES = {
            'function': {0: ...},
            'data': {0: ...},
            }


class ThreadGroupDispatchDataContext(ContextWhitelistable):
    """Context type for thread group dispatch data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': 'archi.thread_group',
                'work': 'archi.thread_group.work',
                'callback': 'archi.thread_group.callback',
                'params': 'archi.thread_group.dispatch_params',
                'batch_size': (c.c_size_t, lambda v: c.c_size_t(v)),
                'name': (str, lambda v: str(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_thread_group_dispatch_data_interface'

    CONTEXT_TYPE = 'archi.thread_group.dispatch_data'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'context': {0: 'archi.thread_group'},
            'work': {0: 'archi.thread_group.work'},
            'callback': {0: 'archi.thread_group.callback'},
            'batch_size': {0: c.c_size_t},
            'name': {0: str},
            }

    SETTER_SLOT_TYPES = {
            'context': {0: 'archi.thread_group'},
            'work': {0: 'archi.thread_group.work'},
            'callback': {0: 'archi.thread_group.callback'},
            'batch_size': {0: (c.c_size_t, lambda v: c.c_size_t(v))},
            }

###############################################################################

class StringToNumberConverterContext(ContextWhitelistable):
    """Context type for string-to-number converters.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'as_uchar': (str, lambda v: str(v)),
                'as_ushort': (str, lambda v: str(v)),
                'as_uint': (str, lambda v: str(v)),
                'as_ulong': (str, lambda v: str(v)),
                'as_ulonglong': (str, lambda v: str(v)),
                'as_schar': (str, lambda v: str(v)),
                'as_sshort': (str, lambda v: str(v)),
                'as_sint': (str, lambda v: str(v)),
                'as_slong': (str, lambda v: str(v)),
                'as_slonglong': (str, lambda v: str(v)),
                'as_float': (str, lambda v: str(v)),
                'as_double': (str, lambda v: str(v)),
                'as_longdouble': (str, lambda v: str(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_convert_string_to_number_interface'

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class TimerContext(ContextWhitelistable):
    """Context type for timers.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'name': (str, lambda v: str(v)),
                }

    class ActionResetParameters(ParametersWhitelistable):
        PARAMETERS = {}

    INTERFACE_SYMBOL = 'archi_context_timer_interface'

    CONTEXT_TYPE = 'archi.timer'

    INIT_PARAMETERS_CLASS = InitParameters

    ACTION_PARAMETER_CLASSES = {
            'reset': {0: ActionResetParameters},
            }

###############################################################################
###############################################################################

CONTEXT_CLASSES = [
        Parameters.Context, PointerContext, ArrayContext,
        HSPFrameContext, HSPTransitionContext, HSPBranchStateDataContext, HSPTransitionAttachmentDataContext,
        HashmapContext, LockFreeQueueContext, EnvVarContext, SignalHandlerContext,
        MemoryContext, MemoryMappingContext, FileContext, LibraryContext,
        ThreadGroupContext, ThreadGroupWorkContext, ThreadGroupCallbackContext, ThreadGroupDispatchDataContext,
        StringToNumberConverterContext, TimerContext,
        ]


class ExecutableContext(LibraryContext):
    """Library context type for the executable itself.
    """
    SYMBOLS = {cls.INTERFACE_SYMBOL: archi_context_interface_t for cls in CONTEXT_CLASSES}


REGISTRY_BUILTIN_REGISTRY = ('archi.registry', HashmapContext)
REGISTRY_BUILTIN_EXECUTABLE = ('archi.executable', ExecutableContext)
REGISTRY_BUILTIN_INPUT_FILE = ('archi.input_file', FileContext)
REGISTRY_BUILTIN_SIGNAL = ('archi.signal', SignalManagementContext)

