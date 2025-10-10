# @file
# @brief Context types in the plugin.

import ctypes as c

from archi.base.app import (
        PrivateType, PublicType,
        ContextWhitelistable, ParametersWhitelistable,
        )
from archi.base.ctypes.base import archi_pointer_t
from archi.base.ctypes.context import archi_context_interface_t
from archi.builtin.context import LibraryContext

###############################################################################

_TYPE_BOOL = PublicType(c.c_byte, constr=lambda v: c.c_byte(bool(v)))
_TYPE_SIZE = PublicType(c.c_size_t)
_TYPE_SIZE_ARRAY = PublicType(c.c_size_t, array=True, constr=lambda v: (c.c_size_t * len(v))(*v))
_TYPE_UINT32 = PublicType(c.c_uint32)
_TYPE_UINT32_ARRAY = PublicType(c.c_uint32, array=True, constr=lambda v: (c.c_uint32 * len(v))(*v))
_TYPE_UINT64 = PublicType(c.c_uint64)
_TYPE_STR = PublicType(c.c_char, array=True, constr=lambda v: c.create_string_buffer(v.encode()))
_TYPE_VOID_P = PublicType(c.c_void_p)
_TYPE_VOID_P_ARRAY = PublicType(c.c_void_p, array=True)
_TYPE_CHAR_ARRAY = PublicType(c.c_char, array=True, constr=lambda v: (c.c_char * len(v))(*v))
_TYPE_CHAR_P_ARRAY = PublicType(c.c_char_p, array=True)
_TYPE_POINTER_ARRAY = PublicType(archi_pointer_t, array=True)

_TYPE_HASHMAP = PrivateType('archi.hashmap')

_TYPE_OPENCL_PLATFORM_ID = PrivateType('opencl.platform_id')
_TYPE_OPENCL_DEVICE_ID = PrivateType('opencl.device_id')
_TYPE_OPENCL_CONTEXT = PrivateType('opencl.context')
_TYPE_OPENCL_COMMAND_QUEUE = PrivateType('opencl.command_queue')
_TYPE_OPENCL_PROGRAM = PrivateType('opencl.program')
_TYPE_OPENCL_KERNEL = PrivateType('opencl.kernel')
_TYPE_OPENCL_SVM_ALLOC_DATA = PrivateType('opencl.svm.alloc_data')
_TYPE_OPENCL_SVM_MAP_DATA = PrivateType('opencl.svm.map_data')
_TYPE_OPENCL_EVENT_ARRAY = PrivateType('opencl.event_array')
_TYPE_OPENCL_WORK_VECTOR = PrivateType('opencl.work_vector')
_TYPE_OPENCL_KERNEL_ENQUEUE_DATA = PrivateType('opencl.kernel.enqueue_data')

_TYPE_CONTEXT_INTERFACE = PublicType(archi_context_interface_t)

###############################################################################

class OpenCLContextHandleContext(ContextWhitelistable):
    """Context type for OpenCL contexts.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'platform_idx': _TYPE_UINT32,
                'device_idx': _TYPE_UINT32_ARRAY,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_context_interface'

    DATA_TYPE = _TYPE_OPENCL_CONTEXT

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'platform_id': {0: _TYPE_OPENCL_PLATFORM_ID},
            'device_id': {0: _TYPE_VOID_P_ARRAY,
                          1: _TYPE_OPENCL_DEVICE_ID},
            }

###############################################################################

class OpenCLCommandQueueContext(ContextWhitelistable):
    """Context type for OpenCL command queues.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': _TYPE_OPENCL_CONTEXT,
                'device_id': _TYPE_OPENCL_DEVICE_ID,
                'out_of_order_exec': _TYPE_BOOL,
                'profiling': _TYPE_BOOL,
                'priority_hint': _TYPE_UINT32,
                'throttle_hint': _TYPE_UINT32,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_command_queue_interface'

    DATA_TYPE = _TYPE_OPENCL_COMMAND_QUEUE

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'context': {0: _TYPE_OPENCL_CONTEXT},
            'device_id': {0: _TYPE_OPENCL_DEVICE_ID},
            }

###############################################################################

class _OpenCLProgramContext(ContextWhitelistable):
    """The base class for context types for OpenCL programs.
    """
    DATA_TYPE = _TYPE_OPENCL_PROGRAM

    GETTER_SLOT_TYPES = {
            'context': {0: _TYPE_OPENCL_CONTEXT},
            'platform_id': {0: _TYPE_OPENCL_PLATFORM_ID},
            'device_id': {0: _TYPE_VOID_P_ARRAY,
                          1: _TYPE_OPENCL_DEVICE_ID},
            'binary_size': {0: _TYPE_SIZE_ARRAY,
                            1: _TYPE_SIZE},
            'binary': {0: _TYPE_CHAR_P_ARRAY,
                       1: _TYPE_CHAR_ARRAY},
            }


class OpenCLProgramFromSourcesContext(_OpenCLProgramContext):
    """Context type for OpenCL programs created from source.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': _TYPE_OPENCL_CONTEXT,
                'device_id': _TYPE_VOID_P_ARRAY,
                'headers': _TYPE_HASHMAP,
                'sources': _TYPE_HASHMAP,
                'libraries': _TYPE_VOID_P_ARRAY,
                'cflags': _TYPE_STR,
                'lflags': _TYPE_STR,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_program_src_interface'

    INIT_PARAMETERS_CLASS = InitParameters


class OpenCLProgramFromBinariesContext(_OpenCLProgramContext):
    """Context type for OpenCL programs created from binaries.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': _TYPE_OPENCL_CONTEXT,
                'device_id': _TYPE_VOID_P_ARRAY,
                'binaries': _TYPE_POINTER_ARRAY,
                'build': _TYPE_BOOL,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_program_bin_interface'

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class _OpenCLKernelContext(ContextWhitelistable):
    """The base class for context types for OpenCL kernels.
    """
    DATA_TYPE = _TYPE_OPENCL_KERNEL

    GETTER_SLOT_TYPES = {
            'program': {0: _TYPE_OPENCL_PROGRAM},
            'name': {0: _TYPE_STR},
            'num_arguments': {0: _TYPE_UINT32},
            }

    SETTER_SLOT_TYPES = {
            'arg.value': {1: None},
            'arg.svm_ptr': {1: None},
            'exec_info.svm_ptrs': {0: _TYPE_VOID_P_ARRAY},
            }


class OpenCLKernelNewContext(_OpenCLKernelContext):
    """Context type for newly created OpenCL kernels.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'program': _TYPE_OPENCL_PROGRAM,
                'name': _TYPE_STR,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_kernel_new_interface'

    INIT_PARAMETERS_CLASS = InitParameters


class OpenCLKernelCloneContext(_OpenCLKernelContext):
    """Context type for copied OpenCL kernels.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'kernel': _TYPE_OPENCL_KERNEL,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_kernel_clone_interface'

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class OpenCLSVMAllocDataContext(ContextWhitelistable):
    """Context type for SVM allocation data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': _TYPE_OPENCL_CONTEXT,
                'mem_flags': _TYPE_UINT64,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_svm_alloc_data_interface'

    DATA_TYPE = _TYPE_OPENCL_SVM_ALLOC_DATA

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'context': {0: _TYPE_OPENCL_CONTEXT},
            'mem_flags': {0: _TYPE_UINT64},
            }


class OpenCLSVMMapDataContext(ContextWhitelistable):
    """Context type for SVM map data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'command_queue': _TYPE_OPENCL_COMMAND_QUEUE,
                'map_flags': _TYPE_UINT64,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_svm_map_data_interface'

    DATA_TYPE = _TYPE_OPENCL_SVM_MAP_DATA

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'command_queue': {0: _TYPE_OPENCL_COMMAND_QUEUE},
            'map_flags': {0: _TYPE_UINT64},
            }

###############################################################################

class OpenCLEventArrayContext(ContextWhitelistable):
    """Context type for OpenCL event arrays.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'num_events': _TYPE_UINT32,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_event_array_interface'

    DATA_TYPE = _TYPE_OPENCL_EVENT_ARRAY

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_events': {0: _TYPE_UINT32},
            'event_ptr': {1: _TYPE_VOID_P},
            }

###############################################################################

class OpenCLWorkVectorContext(ContextWhitelistable):
    """Context type for OpenCL work vectors.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'num_dimensions': _TYPE_UINT32,
                'dimensions': _TYPE_SIZE_ARRAY,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_work_vector_interface'

    DATA_TYPE = _TYPE_OPENCL_WORK_VECTOR

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_dimensions': {0: _TYPE_UINT32},
            'dimension': {1: _TYPE_SIZE},
            }

    SETTER_SLOT_TYPES = {
            'dimension': {1: _TYPE_SIZE},
            }

###############################################################################

class OpenCLKernelEnqueueData(ContextWhitelistable):
    """Context type for OpenCL kernel enqueue data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'command_queue': _TYPE_OPENCL_COMMAND_QUEUE,
                'kernel': _TYPE_OPENCL_KERNEL,
                'global_work_offset': _TYPE_OPENCL_WORK_VECTOR,
                'global_work_size': _TYPE_OPENCL_WORK_VECTOR,
                'local_work_size': _TYPE_OPENCL_WORK_VECTOR,
                'wait_list': _TYPE_OPENCL_EVENT_ARRAY,
                'name': _TYPE_STR,
                }

    class ActionAddOutputEventParameters(ParametersWhitelistable):
        PARAMETERS = {
                'ptr': _TYPE_VOID_P,
                }

    class ActionResetOutputEventsParameters(ParametersWhitelistable):
        PARAMETERS = {}

    INTERFACE_SYMBOL = 'archi_opencl_kernel_enqueue_data_interface'

    DATA_TYPE = _TYPE_OPENCL_KERNEL_ENQUEUE_DATA

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'command_queue': {0: _TYPE_OPENCL_COMMAND_QUEUE},
            'kernel': {0: _TYPE_OPENCL_KERNEL},
            'global_work_offset': {0: _TYPE_OPENCL_WORK_VECTOR},
            'global_work_size': {0: _TYPE_OPENCL_WORK_VECTOR},
            'local_work_size': {0: _TYPE_OPENCL_WORK_VECTOR},
            'wait_list': {0: _TYPE_OPENCL_EVENT_ARRAY},
            'name': {0: _TYPE_STR},
            }

    SETTER_SLOT_TYPES = {
            'command_queue': {0: _TYPE_OPENCL_COMMAND_QUEUE},
            'kernel': {0: _TYPE_OPENCL_KERNEL},
            'global_work_offset': {0: _TYPE_OPENCL_WORK_VECTOR},
            'global_work_size': {0: _TYPE_OPENCL_WORK_VECTOR},
            'local_work_size': {0: _TYPE_OPENCL_WORK_VECTOR},
            'wait_list': {0: _TYPE_OPENCL_EVENT_ARRAY},
            'name': {0: _TYPE_STR},
            }

    ACTION_PARAMETER_CLASSES = {
            'add_output_event': {0: ActionAddOutputEventParameters},
            'reset_output_events': {0: ActionResetOutputEventsParameters},
            }

###############################################################################
###############################################################################

CONTEXT_CLASSES = [
        OpenCLContextHandleContext, OpenCLCommandQueueContext,
        OpenCLProgramFromSourcesContext, OpenCLProgramFromBinariesContext,
        OpenCLKernelNewContext, OpenCLKernelCloneContext,
        OpenCLSVMAllocDataContext, OpenCLSVMMapDataContext,
        OpenCLEventArrayContext, OpenCLWorkVectorContext,
        OpenCLKernelEnqueueData,
        ]


class OpenCLPluginContext(LibraryContext):
    """Library context type for the OpenCL plugin.
    """
    SYMBOLS = {cls.INTERFACE_SYMBOL: _TYPE_CONTEXT_INTERFACE for cls in CONTEXT_CLASSES}

