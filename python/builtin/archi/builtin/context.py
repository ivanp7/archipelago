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
        TypeDescriptor, PrivateType, PublicType,
        Context, ContextWhitelistable, Parameters, ParametersWhitelistable,
        PointerContext, ArrayContext
        )
from archi.base.ctypes.base import archi_array_layout_t, archi_pointer_flags_t
from archi.base.ctypes.context import archi_context_interface_t

from .ctypes.signal import archi_signal_watch_set_t

###############################################################################

_TYPE_BOOL = PublicType(c.c_byte, constr=lambda v: c.c_byte(bool(v)))
_TYPE_INT = PublicType(c.c_int)
_TYPE_SIZE = PublicType(c.c_size_t)
_TYPE_ARRAY_LAYOUT = PublicType(archi_array_layout_t, constr=lambda v: archi_array_layout_t(*v))
_TYPE_POINTER_FLAGS = PublicType(archi_pointer_flags_t)
_TYPE_STR = PublicType(c.c_char, array=True, constr=lambda v: c.create_string_buffer(v.encode()))

_TYPE_HSP_STATE = PrivateType('archi.hsp.state')
_TYPE_HSP_STATE_FUNC = PrivateType('archi.hsp.state.function')
_TYPE_HSP_FRAME = PrivateType('archi.hsp.frame')
_TYPE_HSP_TRANSITION = PrivateType('archi.hsp.transition')
_TYPE_HSP_TRANSITION_FUNC = PrivateType('archi.hsp.transition.function')
_TYPE_HSP_BRANCH_STATE_DATA = PrivateType('archi.hsp.state.branch.data')
_TYPE_HSP_BRANCH_STATE_SELECTOR_FUNC = PrivateType('archi.hsp.state.branch.selector_func')
_TYPE_HSP_TRANSITION_ATTACHMENT_DATA = PrivateType('archi.hsp.transition.attachment.data')

_TYPE_HASHMAP = PrivateType('archi.hashmap')
_TYPE_HASHMAP_ALLOC_PARAMS = PrivateType('archi.hashmap.alloc_params')

_TYPE_LFQUEUE = PrivateType('archi.lfqueue')
_TYPE_LFQUEUE_ALLOC_PARAMS = PrivateType('archi.lfqueue.alloc_params')

_TYPE_SIGNAL_WATCH_SET = PublicType(archi_signal_watch_set_t)
_TYPE_SIGNAL_FLAGS = PrivateType('archi.signal.flags')
_TYPE_SIGNAL_HANDLER = PrivateType('archi.signal.handler')
_TYPE_SIGNAL_HANDLER_FUNC = PrivateType('archi.signal.handler.function')
_TYPE_SIGNAL_MANAGEMENT_CONTEXT = PrivateType('archi.signal.management')

_TYPE_MEMORY = PrivateType('archi.memory')
_TYPE_MEMORY_INTERFACE = PrivateType('archi.memory.interface')

_TYPE_FILE_OPEN_PARAMS = PrivateType('archi.file.open_params')
_TYPE_FILE_MAP_PARAMS = PrivateType('archi.file.map_params')

_TYPE_LIBRARY_HANDLE = PrivateType('archi.library')
_TYPE_LIBRARY_LOAD_PARAMS = PrivateType('archi.library.load_params')

_TYPE_THREAD_GROUP = PrivateType('archi.thread_group')
_TYPE_THREAD_GROUP_START_PARAMS = PrivateType('archi.thread_group.start_params')
_TYPE_THREAD_GROUP_DISPATCH_PARAMS = PrivateType('archi.thread_group.dispatch_params')
_TYPE_THREAD_GROUP_WORK = PrivateType('archi.thread_group.work')
_TYPE_THREAD_GROUP_WORK_FUNC = PrivateType('archi.thread_group.work.function')
_TYPE_THREAD_GROUP_CALLBACK = PrivateType('archi.thread_group.callback')
_TYPE_THREAD_GROUP_CALLBACK_FUNC = PrivateType('archi.thread_group.callback.function')
_TYPE_THREAD_GROUP_DISPATCH_DATA = PrivateType('archi.thread_group.dispatch_data')

_TYPE_TIMER = PrivateType('archi.timer')

_TYPE_CONTEXT_INTERFACE = PublicType(archi_context_interface_t)

###############################################################################

class HSPFrameContext(ContextWhitelistable):
    """Context type for HSP frames.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'num_states': _TYPE_SIZE,
                }

    class ActionExecuteParameters(ParametersWhitelistable):
        PARAMETERS = {
                'transition_function': _TYPE_HSP_TRANSITION_FUNC,
                'transition_data': None,
                }

    INTERFACE_SYMBOL = 'archi_context_hsp_frame_interface'

    DATA_TYPE = _TYPE_HSP_FRAME

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_states': {0: _TYPE_SIZE},
            'state': {1: _TYPE_HSP_STATE},
            'state.function': {1: _TYPE_HSP_STATE_FUNC},
            'state.data': {1: None},
            'state.metadata': {1: None},
            }

    SETTER_SLOT_TYPES = {
            'state.function': {1: _TYPE_HSP_STATE_FUNC},
            'state.data': {1: None},
            'state.metadata': {1: None},
            }

    ACTION_PARAMETER_CLASSES = {
            'execute': {0: ActionExecuteParameters},
            }


class HSPTransitionContext(ContextWhitelistable):
    """Context type for HSP transitions.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': _TYPE_HSP_TRANSITION_FUNC,
                'data': None,
                }

    INTERFACE_SYMBOL = 'archi_context_hsp_transition_interface'

    DATA_TYPE = _TYPE_HSP_TRANSITION

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'function': {0: _TYPE_HSP_TRANSITION_FUNC},
            'data': {0: None},
            }

    SETTER_SLOT_TYPES = {
            'function': {0: _TYPE_HSP_TRANSITION_FUNC},
            'data': {0: None},
            }


class HSPBranchStateDataContext(ContextWhitelistable):
    """Context type for HSP branch state data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'num_branches': _TYPE_SIZE,
                'selector_function': _TYPE_HSP_BRANCH_STATE_SELECTOR_FUNC,
                'selector_data': None,
                }

    INTERFACE_SYMBOL = 'archi_context_hsp_branch_state_data_interface'

    DATA_TYPE = _TYPE_HSP_BRANCH_STATE_DATA

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_branches': {0: _TYPE_SIZE},
            'branch': {1: _TYPE_HSP_FRAME},
            'selector.function': {0: _TYPE_HSP_BRANCH_STATE_SELECTOR_FUNC},
            'selector.data': {0: None},
            }

    SETTER_SLOT_TYPES = {
            'branch': {1: _TYPE_HSP_FRAME},
            'selector.function': {0: _TYPE_HSP_BRANCH_STATE_SELECTOR_FUNC},
            'selector.data': {0: None},
            }


class HSPTransitionAttachmentDataContext(ContextWhitelistable):
    """Context type for HSP transition attachment data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'pre_function': _TYPE_HSP_TRANSITION_FUNC,
                'pre_data': None,
                'post_function': _TYPE_HSP_TRANSITION_FUNC,
                'post_data': None,
                }

    INTERFACE_SYMBOL = 'archi_context_hsp_transition_attachment_data_interface'

    DATA_TYPE = _TYPE_HSP_TRANSITION_ATTACHMENT_DATA

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'pre.function': {0: _TYPE_HSP_TRANSITION_FUNC},
            'pre.data': {0: None},
            'post.function': {0: _TYPE_HSP_TRANSITION_FUNC},
            'post.data': {0: None},
            }

    SETTER_SLOT_TYPES = {
            'pre.function': {0: _TYPE_HSP_TRANSITION_FUNC},
            'pre.data': {0: None},
            'post.function': {0: _TYPE_HSP_TRANSITION_FUNC},
            'post.data': {0: None},
            }

###############################################################################

class HashmapContext(Context):
    """Context type for hashmaps.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': _TYPE_HASHMAP_ALLOC_PARAMS,
                'capacity': _TYPE_SIZE,
                }

    INTERFACE_SYMBOL = 'archi_context_hashmap_interface'

    DATA_TYPE = _TYPE_HASHMAP

    INIT_PARAMETERS_CLASS = InitParameters

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if indices:
            raise KeyError
        return None

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if len(indices) > 1:
            raise KeyError
        return None

    @classmethod
    def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
        raise KeyError

###############################################################################

class LockFreeQueueContext(ContextWhitelistable):
    """Context type for lock-free queues.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': _TYPE_LFQUEUE_ALLOC_PARAMS,
                'capacity_log2': _TYPE_SIZE,
                'element_size': _TYPE_SIZE,
                'element_alignment': _TYPE_SIZE,
                }

    INTERFACE_SYMBOL = 'archi_context_lfqueue_interface'

    DATA_TYPE = _TYPE_LFQUEUE

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'capacity_log2': {0: _TYPE_SIZE},
            'element_size': {0: _TYPE_SIZE},
            'element_alignment': {0: _TYPE_SIZE},
            }

###############################################################################

class EnvVarContext(ContextWhitelistable):
    """Context type for environmental variables.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'name': _TYPE_STR,
                'default_value': _TYPE_STR,
                }

    INTERFACE_SYMBOL = 'archi_context_envvar_interface'

    DATA_TYPE = _TYPE_STR

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class SignalHandlerContext(ContextWhitelistable):
    """Context type for signal handlers.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': _TYPE_SIGNAL_HANDLER_FUNC,
                'data': None,
                }

    INTERFACE_SYMBOL = 'archi_context_signal_handler_interface'

    DATA_TYPE = _TYPE_SIGNAL_HANDLER

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'function': {0: _TYPE_SIGNAL_HANDLER_FUNC},
            'data': {0: None},
            }

    SETTER_SLOT_TYPES = {
            'function': {0: _TYPE_SIGNAL_HANDLER_FUNC},
            'data': {0: None},
            }


class SignalManagementContext(Context):
    """Context type for signal management contexts.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'signals': _TYPE_SIGNAL_WATCH_SET,
                }

    DATA_TYPE = _TYPE_SIGNAL_MANAGEMENT_CONTEXT

    INIT_PARAMETERS_CLASS = InitParameters

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if name == 'flags':
            if indices:
                raise KeyError
            return _TYPE_SIGNAL_FLAGS
        elif name[:8] == 'handler.':
            if indices:
                raise KeyError
            return _TYPE_SIGNAL_HANDLER
        else:
            raise KeyError

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if name[:8] == 'handler.':
            if indices:
                raise KeyError
            return _TYPE_SIGNAL_HANDLER
        else:
            raise KeyError

###############################################################################

class MemoryContext(ContextWhitelistable):
    """Context type for memory objects.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'interface': _TYPE_MEMORY_INTERFACE,
                'alloc_data': None,
                'layout': _TYPE_ARRAY_LAYOUT,
                'num_elements': _TYPE_SIZE,
                'element_size': _TYPE_SIZE,
                'element_alignment': _TYPE_SIZE,
                }

    INTERFACE_SYMBOL = 'archi_context_memory_interface'

    DATA_TYPE = _TYPE_MEMORY

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'interface': {0: _TYPE_MEMORY_INTERFACE},
            'allocation': {0: None},
            'layout': {0: _TYPE_ARRAY_LAYOUT},
            'num_elements': {0: _TYPE_SIZE},
            'element_size': {0: _TYPE_SIZE},
            'element_alignment': {0: _TYPE_SIZE},
            'full_size': {0: _TYPE_SIZE},
            }


class MemoryMappingContext(ContextWhitelistable):
    """Context type for memory mappings.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'memory': _TYPE_MEMORY,
                'map_data': None,
                'offset': _TYPE_SIZE,
                'num_elements': _TYPE_SIZE,
                'writeable': _TYPE_BOOL,
                }

    class ActionCopyParameters(ParametersWhitelistable):
        PARAMETERS = {
                'source': None,
                'source_offset': _TYPE_SIZE,
                'num_elements': _TYPE_SIZE,
                }

    INTERFACE_SYMBOL = 'archi_context_memory_mapping_interface'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'memory': {0: _TYPE_MEMORY},
            '': {0: None, 1: None},
            'layout': {0: _TYPE_ARRAY_LAYOUT},
            'num_elements': {0: _TYPE_SIZE},
            'element_size': {0: _TYPE_SIZE},
            'element_alignment': {0: _TYPE_SIZE},
            'full_size': {0: _TYPE_SIZE},
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
                'params': _TYPE_FILE_OPEN_PARAMS,
                'size': _TYPE_SIZE,
                'pathname': _TYPE_STR,
                'create': _TYPE_BOOL,
                'exclusive': _TYPE_BOOL,
                'truncate': _TYPE_BOOL,
                'readable': _TYPE_BOOL,
                'writable': _TYPE_BOOL,
                'nonblock': _TYPE_BOOL,
                'flags': _TYPE_INT,
                'mode': _TYPE_INT,
                }

    class ActionMapParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': _TYPE_FILE_MAP_PARAMS,
                'size': _TYPE_SIZE,
                'offset': _TYPE_SIZE,
                'has_header': _TYPE_BOOL,
                'readable': _TYPE_BOOL,
                'writable': _TYPE_BOOL,
                'shared': _TYPE_BOOL,
                'flags': _TYPE_INT,
                'close_fd': _TYPE_BOOL,
                }

    INTERFACE_SYMBOL = 'archi_context_file_interface'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'fd': {0: _TYPE_INT},
            'map': {0: None},
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
                'params': _TYPE_LIBRARY_LOAD_PARAMS,
                'pathname': _TYPE_STR,
                'lazy': _TYPE_BOOL,
                'global': _TYPE_BOOL,
                'flags': _TYPE_INT,
                }

    class ActionSymbolParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': _TYPE_BOOL,
                'flags': _TYPE_POINTER_FLAGS,
                'layout': _TYPE_ARRAY_LAYOUT,
                'num_elements': _TYPE_SIZE,
                'element_size': _TYPE_SIZE,
                'element_alignment': _TYPE_SIZE,
                }

    INTERFACE_SYMBOL = 'archi_context_library_interface'

    DATA_TYPE = _TYPE_LIBRARY_HANDLE

    INIT_PARAMETERS_CLASS = InitParameters

    SYMBOLS = {}

    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        if not isinstance(cls.SYMBOLS, dict):
            raise TypeError
        elif not all(isinstance(key, str) for key in cls.SYMBOLS.keys()):
            raise TypeError
        elif not all(isinstance(value, (type(None), TypeDescriptor)) for value in cls.SYMBOLS.values()):
            raise TypeError

        super().__init_subclass__()

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        if indices:
            raise KeyError
        return cls.SYMBOLS.get(name, None)

    @classmethod
    def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
        if indices:
            raise KeyError
        return cls.ActionSymbolParameters

###############################################################################

class ThreadGroupContext(ContextWhitelistable):
    """Context type for thread groups.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': _TYPE_THREAD_GROUP_START_PARAMS,
                'num_threads': _TYPE_SIZE,
                }

    INTERFACE_SYMBOL = 'archi_context_thread_group_interface'

    DATA_TYPE = _TYPE_THREAD_GROUP

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_threads': {0: _TYPE_SIZE},
            }


class ThreadGroupWorkContext(ContextWhitelistable):
    """Context type for thread group work description.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': _TYPE_THREAD_GROUP_WORK_FUNC,
                'data': None,
                'size': _TYPE_SIZE,
                }

    INTERFACE_SYMBOL = 'archi_context_thread_group_work_interface'

    DATA_TYPE = _TYPE_THREAD_GROUP_WORK

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'function': {0: _TYPE_THREAD_GROUP_WORK_FUNC},
            'data': {0: None},
            'size': {0: _TYPE_SIZE},
            }

    SETTER_SLOT_TYPES = {
            'function': {0: _TYPE_THREAD_GROUP_WORK_FUNC},
            'data': {0: None},
            'size': {0: _TYPE_SIZE},
            }


class ThreadGroupCallbackContext(ContextWhitelistable):
    """Context type for thread group callback description.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'function': _TYPE_THREAD_GROUP_CALLBACK_FUNC,
                'data': None,
                }

    INTERFACE_SYMBOL = 'archi_context_thread_group_callback_interface'

    DATA_TYPE = _TYPE_THREAD_GROUP_CALLBACK

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'function': {0: _TYPE_THREAD_GROUP_CALLBACK_FUNC},
            'data': {0: None},
            }

    SETTER_SLOT_TYPES = {
            'function': {0: _TYPE_THREAD_GROUP_CALLBACK_FUNC},
            'data': {0: None},
            }


class ThreadGroupDispatchDataContext(ContextWhitelistable):
    """Context type for thread group dispatch data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': _TYPE_THREAD_GROUP,
                'work': _TYPE_THREAD_GROUP_WORK,
                'callback': _TYPE_THREAD_GROUP_CALLBACK,
                'params': _TYPE_THREAD_GROUP_DISPATCH_PARAMS,
                'batch_size': _TYPE_SIZE,
                'name': _TYPE_STR,
                }

    INTERFACE_SYMBOL = 'archi_context_thread_group_dispatch_data_interface'

    DATA_TYPE = _TYPE_THREAD_GROUP_DISPATCH_DATA

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'context': {0: _TYPE_THREAD_GROUP},
            'work': {0: _TYPE_THREAD_GROUP_WORK},
            'callback': {0: _TYPE_THREAD_GROUP_CALLBACK},
            'batch_size': {0: _TYPE_SIZE},
            'name': {0: _TYPE_STR},
            }

    SETTER_SLOT_TYPES = {
            'context': {0: _TYPE_THREAD_GROUP},
            'work': {0: _TYPE_THREAD_GROUP_WORK},
            'callback': {0: _TYPE_THREAD_GROUP_CALLBACK},
            'batch_size': {0: _TYPE_SIZE},
            }

###############################################################################

class StringToNumberConverterContext(ContextWhitelistable):
    """Context type for string-to-number converters.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'as_uchar': _TYPE_STR,
                'as_ushort': _TYPE_STR,
                'as_uint': _TYPE_STR,
                'as_ulong': _TYPE_STR,
                'as_ulonglong': _TYPE_STR,
                'as_schar': _TYPE_STR,
                'as_sshort': _TYPE_STR,
                'as_sint': _TYPE_STR,
                'as_slong': _TYPE_STR,
                'as_slonglong': _TYPE_STR,
                'as_float': _TYPE_STR,
                'as_double': _TYPE_STR,
                'as_longdouble': _TYPE_STR,
                }

    INTERFACE_SYMBOL = 'archi_context_convert_string_to_number_interface'

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class TimerContext(ContextWhitelistable):
    """Context type for timers.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'name': _TYPE_STR,
                }

    class ActionResetParameters(ParametersWhitelistable):
        PARAMETERS = {}

    INTERFACE_SYMBOL = 'archi_context_timer_interface'

    DATA_TYPE = _TYPE_TIMER

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
    SYMBOLS = {cls.INTERFACE_SYMBOL: _TYPE_CONTEXT_INTERFACE for cls in CONTEXT_CLASSES}


REGISTRY_BUILTIN_REGISTRY = ('archi.registry', HashmapContext)
REGISTRY_BUILTIN_EXECUTABLE = ('archi.executable', ExecutableContext)
REGISTRY_BUILTIN_INPUT_FILE = ('archi.input_file', FileContext)
REGISTRY_BUILTIN_SIGNAL = ('archi.signal', SignalManagementContext)

