# @file
# @brief Context types in the plugin.

import ctypes as c

from archi.base.app import (
        ContextWhitelistable, ParametersWhitelistable,
        )
from archi.base.ctypes.base import archi_pointer_t
from archi.base.ctypes.context import archi_context_interface_t
from archi.builtin.context import LibraryContext


class OpenCLContextContext(ContextWhitelistable):
    """Context type for OpenCL contexts.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'platform_idx': (c.c_uint32, lambda v: c.c_uint32(v)),
                'device_idx': (..., lambda v: (c.c_uint32 * len(v))(*v)),
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_context_interface'

    CONTEXT_TYPE = 'opencl.context'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'platform_id': {0: 'opencl.platform_id'},
            'device_id': {0: c.c_void_p,
                          1: 'opencl.device_id'},
            }

###############################################################################

class OpenCLCommandQueueContext(ContextWhitelistable):
    """Context type for OpenCL command queues.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': 'opencl.context',
                'device_id': 'opencl.device_id',
                'out_of_order_exec': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'profiling': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'priority_hint': (c.c_uint32, lambda v: c.c_uint32(v)),
                'throttle_hint': (c.c_uint32, lambda v: c.c_uint32(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_command_queue_interface'

    CONTEXT_TYPE = 'opencl.command_queue'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'context': {0: 'opencl.context'},
            'device_id': {0: 'opencl.device_id'},
            }

###############################################################################

class _OpenCLProgramContext(ContextWhitelistable):
    """The base class for context types for OpenCL programs.
    """
    CONTEXT_TYPE = 'opencl.program'

    GETTER_SLOT_TYPES = {
            'context': {0: 'opencl.context'},
            'platform_id': {0: 'opencl.platform_id'},
            'device_id': {0: c.c_void_p,
                          1: 'opencl.device_id'},
            'binary_size': {0: c.c_size_t,
                            1: c.c_size_t},
            'binary': {0: c.POINTER(c.POINTER(c.c_ubyte)),
                       1: c.POINTER(c.c_ubyte)},
            }


class OpenCLProgramFromSourcesContext(_OpenCLProgramContext):
    """Context type for OpenCL programs created from source.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': 'opencl.context',
                'device_id': c.c_void_p,
                'headers': 'archi.hashmap',
                'sources': 'archi.hashmap',
                'libraries': c.c_void_p,
                'cflags': (str, lambda v: str(v)),
                'lflags': (str, lambda v: str(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_program_src_interface'

    INIT_PARAMETERS_CLASS = InitParameters


class OpenCLProgramFromBinariesContext(_OpenCLProgramContext):
    """Context type for OpenCL programs created from binaries.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': 'opencl.context',
                'device_id': c.c_void_p,
                'binaries': archi_pointer_t,
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_program_bin_interface'

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class _OpenCLKernelContext(ContextWhitelistable):
    """The base class for context types for OpenCL kernels.
    """
    CONTEXT_TYPE = 'opencl.kernel'

    GETTER_SLOT_TYPES = {
            'program': {0: 'opencl.program'},
            'name': {0: str},
            'num_arguments': {0: c.c_uint32},
            }

    SETTER_SLOT_TYPES = {
            'arg.value': {1: ...},
            'arg.svm_ptr': {1: ...},
            'exec_info.svm_ptrs': {0: c.c_void_p},
            }


class OpenCLKernelNewContext(_OpenCLKernelContext):
    """Context type for newly created OpenCL kernels.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'program': 'opencl.program',
                'name': (str, lambda v: str(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_kernel_new_interface'

    INIT_PARAMETERS_CLASS = InitParameters


class OpenCLKernelCloneContext(_OpenCLKernelContext):
    """Context type for copied OpenCL kernels.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'kernel': 'opencl.kernel',
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_kernel_clone_interface'

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class OpenCLSVMAllocDataContext(ContextWhitelistable):
    """Context type for SVM allocation data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'context': 'opencl.context',
                'mem_flags': (c.c_uint64, lambda v: c.c_uint64(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_svm_alloc_data_interface'

    CONTEXT_TYPE = 'opencl.svm.alloc_data'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'context': {0: 'opencl.context'},
            'mem_flags': {0: c.c_uint64},
            }


class OpenCLSVMMapDataContext(ContextWhitelistable):
    """Context type for SVM map data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'command_queue': 'opencl.command_queue',
                'map_flags': (c.c_uint64, lambda v: c.c_uint64(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_svm_map_data_interface'

    CONTEXT_TYPE = 'opencl.svm.map_data'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'command_queue': {0: 'opencl.command_queue'},
            'map_flags': {0: c.c_uint64},
            }

###############################################################################

class OpenCLEventArrayContext(ContextWhitelistable):
    """Context type for OpenCL event arrays.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'num_events': (c.c_uint32, lambda v: c.c_uint32(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_event_array_interface'

    CONTEXT_TYPE = 'opencl.event_array'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_events': {0: c.c_uint32},
            'event_ptr': {1: c.c_void_p},
            }

###############################################################################

class OpenCLWorkVectorContext(ContextWhitelistable):
    """Context type for OpenCL work vectors.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'num_dimensions': (c.c_uint32, lambda v: c.c_uint32(v)),
                'dimensions': (..., lambda v: (c.c_size_t * len(v))(*v)),
                }

    INTERFACE_SYMBOL = 'archi_context_opencl_work_vector_interface'

    CONTEXT_TYPE = 'opencl.work_vector'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'num_dimensions': {0: c.c_uint32},
            'dimension': {1: c.c_size_t},
            }

    SETTER_SLOT_TYPES = {
            'dimension': {1: (c.c_size_t, lambda v: c.c_size_t(v))},
            }

###############################################################################

class OpenCLKernelEnqueueData(ContextWhitelistable):
    """Context type for OpenCL kernel enqueue data.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'command_queue': 'opencl.command_queue',
                'kernel': 'opencl.kernel',
                'global_work_offset': 'opencl.work_vector',
                'global_work_size': 'opencl.work_vector',
                'local_work_size': 'opencl.work_vector',
                'wait_list': 'opencl.event_array',
                'name': (str, lambda v: str(v)),
                }

    INTERFACE_SYMBOL = 'archi_opencl_kernel_enqueue_data_interface'

    CONTEXT_TYPE = 'opencl.kernel.enqueue_data'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'command_queue': {0: 'opencl.command_queue'},
            'kernel': {0: 'opencl.kernel'},
            'global_work_offset': {0: 'opencl.work_vector'},
            'global_work_size': {0: 'opencl.work_vector'},
            'local_work_size': {0: 'opencl.work_vector'},
            'wait_list': {0: 'opencl.event_array'},
            'name': {0: str},
            }

    SETTER_SLOT_TYPES = {
            'command_queue': {0: 'opencl.command_queue'},
            'kernel': {0: 'opencl.kernel'},
            'global_work_offset': {0: 'opencl.work_vector'},
            'global_work_size': {0: 'opencl.work_vector'},
            'local_work_size': {0: 'opencl.work_vector'},
            'wait_list': {0: 'opencl.event_array'},
            'output_event_ptr': {0: c.c_void_p},
            }

###############################################################################
###############################################################################

CONTEXT_CLASSES = [
        OpenCLContextContext, OpenCLCommandQueueContext,
        OpenCLProgramFromSourcesContext, OpenCLProgramFromBinariesContext,
        OpenCLKernelNewContext, OpenCLKernelCloneContext,
        OpenCLSVMAllocDataContext, OpenCLSVMMapDataContext,
        OpenCLEventArrayContext, OpenCLWorkVectorContext,
        OpenCLKernelEnqueueData,
        ]


class OpenCLPluginContext(LibraryContext):
    """Library context type for the OpenCL plugin.
    """
    SYMBOLS = {cls.INTERFACE_SYMBOL: archi_context_interface_t for cls in CONTEXT_CLASSES}

