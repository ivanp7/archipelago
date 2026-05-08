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
from types import SimpleNamespace

import archi.ctypes as actype
from .object import PrimitiveData, String
from .context import (
        TypeAttributes,
        Context,
        ContextWhitelist,
        ParametersWhitelist,
        AggregateTypeSymbol,
        DexgraphOperationDataSymbol,
        MemoryInterfaceSymbol,
        MemoryAllocDataSymbol,
)
from .helper import new_aggregate_object
from .procedure import Procedure, MemoryAllocations


PLUGIN_OPENCL = 'archi_opencl'

##############################################################################

def _make_uint32_t(value):
    if value < 0:
        raise ValueError
    return PrimitiveData(c.c_uint32(value))

def _make_uint32_array(values):
    if not all(value >= 0 for value in values):
        raise ValueError
    return PrimitiveData((c.c_uint32 * len(values))(*values))

def _make_size_t(value):
    if value < 0:
        raise ValueError
    return PrimitiveData(c.c_size_t(value))

def _make_size_array(values):
    if not all(value >= 0 for value in values):
        raise ValueError
    return PrimitiveData((c.c_size_t * len(values))(*values))

_TYPE_DATA = TypeAttributes.complex_data()
_TYPE_BOOL = (TypeAttributes.from_type(c.c_char),
              lambda value: PrimitiveData(c.c_char(bool(value))))
_TYPE_CL_UINT = (TypeAttributes.from_type(c.c_uint32), _make_uint32_t)
_TYPE_CL_UINT_ARRAY = (TypeAttributes.from_type(c.c_uint32 * 1), _make_uint32_array)
_TYPE_SIZE = (TypeAttributes.from_type(c.c_size_t), _make_size_t)
_TYPE_SIZE_ARRAY = (TypeAttributes.from_type(c.c_size_t * 1), _make_size_array)
_TYPE_DATA_PTR_ARRAY = TypeAttributes.from_type(c.c_void_p * 1)
_TYPE_BYTES = (TypeAttributes.from_type(c.c_ubyte * 1),
               lambda value: PrimitiveData.from_bytes(value))
_TYPE_STRING = (TypeAttributes.from_type(c.c_char * 1),
                lambda value: String(value))

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

##############################################################################

def new_opencl_svm_alloc_data(registry, plugin_key, key, /, cl_context=None,
                              read=False, write=False, fine_grain=False, atomics=False):
    """Create OpenCL SVM allocation function data.
    """
    if not read and not write:
        raise ValueError
    elif not fine_grain and atomics:
        raise ValueError

    plugin = registry[plugin_key]

    alloc_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        MemoryAllocDataSymbol.full_name('opencl_svm'), plugin))

    if cl_context is not None:
        alloc_data.member.context = cl_context

    if read and write:
        mem_flags = 1 << 0
    elif write:
        mem_flags = 1 << 1
    elif read:
        mem_flags = 1 << 2

    if fine_grain:
        mem_flags |= 1 << 10
    if atomics:
        mem_flags |= 1 << 11

    alloc_data.member.mem_flags = PrimitiveData(c.c_ulong(mem_flags))

    return alloc_data


def new_opencl_svm_map_data(registry, plugin_key, key, /, command_queue=None,
                            read=False, write=False, invalidate=False):
    """Create OpenCL SVM mapping function data.
    """
    if not read and not write:
        raise ValueError
    elif invalidate and (read or not write):
        raise ValueError

    plugin = registry[plugin_key]

    map_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        MemoryMapDataSymbol.full_name('opencl_svm'), plugin))

    if command_queue is not None:
        map_data.member.command_queue = command_queue

    if write and invalidate:
        map_flags = 1 << 2
    else:
        map_flags = 0
        if read:
            map_flags |= 1 << 0
        if write:
            map_flags |= 1 << 1

    map_data.member.map_flags = PrimitiveData(c.c_ulong(map_flags))

    return map_data


def new_opencl_kernel_enqueue_data(registry, plugin_key, key, /,
                                   command_queue=None, kernel=None,
                                   work_offset_global=None, work_size_global=None,
                                   work_size_local=None):
    """Create OpenCL kernel enqueueing function data.
    """
    if work_size_global is not None:
        if isinstance(work_size_global, int):
            work_size_global = (work_size_global,)
        elif not isinstance(work_size_global, tuple):
            raise TypeError
        elif not work_size_global:
            raise ValueError

        for elt in work_size_global:
            if not isinstance(elt, int):
                raise TypeError
            elif elt < 0:
                raise ValueError

        num_dimensions = len(work_size_global)
    else:
        if work_offset_global is not None:
            raise ValueError
        elif work_size_local is not None:
            raise ValueError

        num_dimensions = 1 # 0 dimensions is not allowed by clEnqueueNDRangeKernel()

    if work_offset_global is not None:
        if isinstance(work_offset_global, int):
            work_offset_global = (work_offset_global,)
        elif not isinstance(work_offset_global, tuple):
            raise TypeError

        if len(work_offset_global) != num_dimensions:
            raise ValueError

        for elt in work_offset_global:
            if not isinstance(elt, int):
                raise TypeError
            elif elt < 0:
                raise ValueError

    if work_size_local is not None:
        if isinstance(work_size_local, int):
            work_size_local = (work_size_local,)
        elif not isinstance(work_size_local, tuple):
            raise TypeError

        if len(work_size_local) != num_dimensions:
            raise ValueError

        for elt in work_size_local:
            if not isinstance(elt, int):
                raise TypeError
            elif elt < 0:
                raise ValueError

    plugin = registry[plugin_key]

    enqueue_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        DexgraphOperationDataSymbol.full_name('opencl_kernel_enqueue'), plugin))

    if command_queue is not None:
        enqueue_data.member.command_queue = command_queue
    if kernel is not None:
        enqueue_data.member.kernel = kernel

    enqueue_data.member.num_work_dimensions = PrimitiveData(c.c_size_t(num_dimensions))

    vtype = c.c_size_t * num_dimensions

    if work_offset_global is not None:
        enqueue_data.member.work_offset_global = PrimitiveData(vtype(*work_offset_global))
    if work_size_global is not None:
        enqueue_data.member.work_size_global = PrimitiveData(vtype(*work_size_global))
    if work_size_local is not None:
        enqueue_data.member.work_size_local = PrimitiveData(vtype(*work_size_local))

    return enqueue_data


def new_opencl_event_wait_data(registry, plugin_key, key, /):
    """Create OpenCL event waiting function data.
    """
    plugin = registry[plugin_key]

    return new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        'opencl_event_array', plugin))

##############################################################################

class OpenCLSVMAllocations(MemoryAllocations):
    """OpenCL SVM allocation and initialization procedure.
    """
    def __init__(self, registry, plugin_key, /, alloc_data, map_data=None, namespace=''):
        """Initialize a procedure.
        """
        if alloc_data is None:
            raise ValueError

        plugin = registry[plugin_key]

        super().__init__(registry, namespace=namespace,
                         interface=MemoryInterfaceSymbol.slot('opencl_svm', plugin),
                         alloc_data=alloc_data,
                         map_data=map_data)


class OpenCLEventOrderingGraph(Procedure):
    """OpenCL event ordering graph construction procedure.
    """
    def __init__(self, registry, /, namespace=''):
        """Initialize a procedure.
        """
        super().__init__(registry, namespace=namespace)

        self._nodes = {}

    def node(self, key, /, wait_list=None, out_list=None):
        """Add a node to the event ordering graph.
        """
        if self.active:
            raise RuntimeError
        elif not isinstance(key, str):
            raise TypeError
        elif not key:
            raise ValueError
        elif key in self._nodes:
            raise KeyError(f"Node '{key}' is added to the graph already")

        if wait_list is not None:
            if not isinstance(wait_list, (Context, Context.Slot)):
                raise TypeError
            elif not self.registry.owns(wait_list):
                raise ValueError

        if out_list is not None:
            if not isinstance(out_list, (Context, Context.Slot)):
                raise TypeError
            elif not self.registry.owns(out_list):
                raise ValueError

        self._nodes[key] = SimpleNamespace(waiting_for={},
                                           waited_by={},
                                           wait_list=wait_list,
                                           out_list=out_list)

        return self

    def waits_for(self, key, other_key, /):
        """Specify that a node is waiting for another node.
        """
        if key not in self._nodes:
            raise KeyError(f"No such node with key '{key}' in the graph")
        elif other_key not in self._nodes:
            raise KeyError(f"No such node with key '{other_key}' in the graph")
        elif key == other_key:
            raise ValueError

        node = self._nodes[key]
        other_node = self._nodes[other_key]

        if node.wait_list is None:
            raise RuntimeError(f"Node '{key}' has no event wait list")
        elif other_node.out_list is None:
            raise RuntimeError(f"Node '{key}' has no event output list")
        elif other_key in node.waiting_for:
            raise RuntimeError(f"Node '{key}' is waiting for '{other_key}' already")
        elif key in other_node.waited_by:
            raise RuntimeError(f"Node '{key}' is waiting for '{other_key}' already")

        node.waiting_for[other_key] = len(node.waiting_for)
        other_node.waited_by[key] = len(other_node.waited_by)

        return self

    def _begin(self, /):
        """Implementation of the procedure beginning.
        """
        # Create pointer array contexts (event lists)
        wait_list_contexts = {}
        out_list_contexts = {}

        for key, node in self._nodes.items():
            if node.wait_list is not None:
                wait_list_contexts[key] = self.registry.new_context(
                        (None,) * len(node.waiting_for),
                        key=self.temp_key(f'wait_list_{key}'))

            if node.out_list is not None:
                out_list_contexts[key] = self.registry.new_context(
                        (None,) * len(node.waited_by),
                        key=self.temp_key(f'out_list_{key}'))

        # Assign event lists
        for key, node in self._nodes.items():
            if node.wait_list is not None:
                node.wait_list.num_events = len(node.waiting_for)
                node.wait_list.event = wait_list_contexts[key].ptrs

            if node.out_list is not None:
                node.out_list.num_event_ptrs = len(node.waited_by)
                node.out_list.event_ptr = out_list_contexts[key].ptrs

        # Assign pointers to events
        for key, node in self._nodes.items():
            if node.out_list is None:
                continue

            out_list_context = out_list_contexts[key]

            for other_key, out_list_index in node.waited_by.items():
                wait_list_context = wait_list_contexts[other_key]
                wait_list_index = self._nodes[other_key].waiting_for[key]

                out_list_context[out_list_index] = \
                        Context.weak_ref(wait_list_context.ptr[wait_list_index])

        # Delete temporary contexts
        for context in wait_list_contexts.values():
            self.registry.del_context(context)

        for context in out_list_contexts.values():
            self.registry.del_context(context)

    def _end(self, /):
        """Implementation of the procedure end.
        """
        for node in self._nodes.values():
            if node.wait_list is not None:
                node.wait_list.num_events = 0
                node.wait_list.event = None

            if node.out_list is not None:
                node.out_list.num_event_ptrs = 0
                node.out_list.event_ptr = None

    def _reset(self, /):
        """Implementation of the procedure reset.
        """
        self._nodes.clear()

