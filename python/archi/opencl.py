 #############################################################################
 # Copyright (C) 2023-2026 by Ivan Podmazov                                  #
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
# @brief Context types of the OpenCL plugin.

import ctypes as c

import archi.ctypes as actype
from .object import PrimitiveData, String
from .context import TypeAttributes, ContextWhitelist, ParametersWhitelist


_TYPE_DATA = TypeAttributes.complex_data()
_TYPE_BOOL = (TypeAttributes.from_type(c.c_char), lambda value: PrimitiveData(c.c_char(bool(value))))
_TYPE_CL_UINT = (TypeAttributes.from_type(c.c_uint32), lambda value: PrimitiveData(c.c_uint32(value)))
_TYPE_CL_UINT_ARRAY = (TypeAttributes.from_type(c.c_uint32 * 1), lambda value: PrimitiveData((c.c_uint32 * len(value))(*value)))
_TYPE_UINT32 = (TypeAttributes.from_type(c.c_uint32), lambda value: PrimitiveData(c.c_uint32(value)))
_TYPE_SIZE = (TypeAttributes.from_type(c.c_size_t), lambda value: PrimitiveData(c.c_size_t(value)))
_TYPE_SIZE_ARRAY = (TypeAttributes.from_type(c.c_size_t * 1), lambda value: PrimitiveData((c.c_size_t * len(value))(*value)))
_TYPE_DATA_PTR_ARRAY = TypeAttributes.from_type(c.c_void_p * 1)
_TYPE_BYTES = (TypeAttributes.from_type(c.c_ubyte * 1), lambda value: PrimitiveData.from_bytes(value))
_TYPE_STRING = (TypeAttributes.from_type(c.c_char * 1), lambda value: String(value))

##############################################################################

### archi/opencl ###

class OpenCLContext(ContextWhitelist):
    """OpenCL context.
    """
    C_NAME = 'opencl_context'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'platform': _TYPE_CL_UINT,
                  'device': _TYPE_CL_UINT_ARRAY}

    GETTER_SLOTS = {'platform_id': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
                    'device_id': {0: _TYPE_DATA_PTR_ARRAY,
                                  1: TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID)}}


class OpenCLCommandQueueContext(ContextWhitelist):
    """OpenCL command queue.
    """
    C_NAME = 'opencl_command_queue'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'context': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                  'device_id': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID),
                  'out_of_order_exec': _TYPE_BOOL,
                  'profiling': _TYPE_BOOL,
                  'priority_hint': _TYPE_CL_UINT,
                  'throttle_hint': _TYPE_CL_UINT}

    GETTER_SLOTS = {'context': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                    'platform_id': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
                    'device_id': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID)}


class OpenCLProgramSrcContext(ContextWhitelist):
    """OpenCL program built from source.
    """
    C_NAME = 'opencl_program_src'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'context': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                  'device_id': _TYPE_DATA_PTR_ARRAY,
                  'headers': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__KVLIST),
                  'sources': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__KVLIST),
                  'libraries': _TYPE_DATA_PTR_ARRAY,
                  'cflags': _TYPE_STRING,
                  'lflags': _TYPE_STRING}

    GETTER_SLOTS = {'context': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                    'platform_id': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
                    'device_id': {0: _TYPE_DATA_PTR_ARRAY,
                                  1: TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID)},
                    'binary.size': {0: _TYPE_SIZE_ARRAY,
                                    1: _TYPE_SIZE},
                    'binary': {0: _TYPE_DATA_PTR_ARRAY,
                               1: _TYPE_BYTES}}


class OpenCLProgramBinContext(ContextWhitelist):
    """OpenCL program created from binaries.
    """
    C_NAME = 'opencl_program_bin'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'context': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                  'device_id': _TYPE_DATA_PTR_ARRAY,
                  'binaries': _TYPE_DATA_PTR_ARRAY,
                  'binary_sizes': _TYPE_SIZE_ARRAY,
                  'build': _TYPE_BOOL}

    GETTER_SLOTS = OpenCLProgramSrcContext.GETTER_SLOTS


class OpenCLKernelContext(ContextWhitelist):
    """OpenCL kernel created from a program.
    """
    C_NAME = 'opencl_kernel_new'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'program': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM),
                  'name': _TYPE_STRING}

    GETTER_SLOTS = {'program': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM),
                    'name': _TYPE_STRING,
                    'num_args': _TYPE_CL_UINT}

    SETTER_SLOTS = {'arg.local_mem_size': {1: _TYPE_SIZE},
                    'arg.value': {1: _TYPE_DATA},
                    'arg.mem_object': {1: TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_MEM_OBJECT)},
                    'arg.svm_ptr': {1: TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_SVM)},
                    'exec_info.svm_ptrs': _TYPE_DATA_PTR_ARRAY}


class OpenCLKernelCloneContext(ContextWhitelist):
    """OpenCL kernel cloned from another kernel.
    """
    C_NAME = 'opencl_kernel_clone'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'kernel': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)}

    GETTER_SLOTS = OpenCLKernelContext.GETTER_SLOTS

    SETTER_SLOTS = OpenCLKernelContext.SETTER_SLOTS

