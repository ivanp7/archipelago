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
from types import MappingProxyType, NoneType, SimpleNamespace

import archi.ctypes as typ
from ..object import Object, PrimitiveData, String
from ..context import (
        TypeAttr,
        Context,
        ContextWhitelist,
        ParametersWhitelist,
        AggregateTypeSymbol,
        DexgraphOperationDataSymbol,
        MemoryInterfaceSymbol,
        MemoryAllocDataSymbol,
        MemoryMapDataSymbol,
)
from ..registry import Registry
from ..helper import new_aggregate_object
from ..procedure import Procedure, MemoryAllocationProcedure


PLUGIN_OPENCL = 'archi_opencl'

##############################################################################

### archi/opencl ###

ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID = 0x1000
ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID = 0x1001
ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT = 0x1002
ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE = 0x1003
ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM = 0x1004
ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL = 0x1005
ARCHI_POINTER_DATA_TAG__OPENCL_EVENT = 0x1006
ARCHI_POINTER_DATA_TAG__OPENCL_MEM_OBJECT = 0x1007
ARCHI_POINTER_DATA_TAG__OPENCL_SVM = 0x1008
ARCHI_POINTER_DATA_TAG__OPENCL_SVM_ALLOC_DATA = 0x1009
ARCHI_POINTER_DATA_TAG__OPENCL_SVM_MAP_DATA = 0x100A

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

_TYPE_DATA = TypeAttr.complex_data()
_TYPE_BOOL = (TypeAttr.from_type(c.c_char),
              lambda value: PrimitiveData(c.c_char(bool(value))))
_TYPE_CL_UINT = (TypeAttr.from_type(c.c_uint32), _make_uint32_t)
_TYPE_CL_UINT_ARRAY = (TypeAttr.from_type(c.c_uint32 * 1), _make_uint32_array)
_TYPE_SIZE = (TypeAttr.from_type(c.c_size_t), _make_size_t)
_TYPE_SIZE_ARRAY = (TypeAttr.from_type(c.c_size_t * 1), _make_size_array)
_TYPE_DATA_PTR_ARRAY = TypeAttr.from_type(c.c_void_p * 1)
_TYPE_BYTES = (TypeAttr.from_type(c.c_ubyte * 1),
               lambda value: PrimitiveData.from_bytes(value))
_TYPE_STRING = (TypeAttr.from_type(c.c_char * 1),
                lambda value: String(value))

##############################################################################

### archi/opencl ###

class OpenCLContext(ContextWhitelist):
    """OpenCL context.
    """
    C_NAME = 'opencl_context'

    CONTEXT_TYPE = TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'platform': _TYPE_CL_UINT,
                  'device': _TYPE_CL_UINT_ARRAY}

    GETTER_SLOTS = {'platform_id': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
                    'device_id': {0: _TYPE_DATA_PTR_ARRAY,
                                  1: TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID)}}


class OpenCLCommandQueueContext(ContextWhitelist):
    """OpenCL command queue.
    """
    C_NAME = 'opencl_command_queue'

    CONTEXT_TYPE = TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'context': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                  'device_id': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID),
                  'out_of_order_exec': _TYPE_BOOL,
                  'profiling': _TYPE_BOOL,
                  'priority_hint': _TYPE_CL_UINT,
                  'throttle_hint': _TYPE_CL_UINT}

    GETTER_SLOTS = {'context': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                    'platform_id': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
                    'device_id': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID)}


class OpenCLProgramSrcContext(ContextWhitelist):
    """OpenCL program built from source.
    """
    C_NAME = 'opencl_program_src'

    CONTEXT_TYPE = TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'context': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                  'device_id': _TYPE_DATA_PTR_ARRAY,
                  'headers': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__KVLIST),
                  'sources': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__KVLIST),
                  'libraries': _TYPE_DATA_PTR_ARRAY,
                  'cflags': _TYPE_STRING,
                  'lflags': _TYPE_STRING}

    GETTER_SLOTS = {'context': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                    'platform_id': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
                    'device_id': {0: _TYPE_DATA_PTR_ARRAY,
                                  1: TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID)},
                    'binary.size': {0: _TYPE_SIZE_ARRAY,
                                    1: _TYPE_SIZE},
                    'binary': {0: _TYPE_DATA_PTR_ARRAY,
                               1: _TYPE_BYTES}}


class OpenCLProgramBinContext(ContextWhitelist):
    """OpenCL program created from binaries.
    """
    C_NAME = 'opencl_program_bin'

    CONTEXT_TYPE = TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'context': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
                  'device_id': _TYPE_DATA_PTR_ARRAY,
                  'binaries': _TYPE_DATA_PTR_ARRAY,
                  'binary_sizes': _TYPE_SIZE_ARRAY,
                  'build': _TYPE_BOOL}

    GETTER_SLOTS = OpenCLProgramSrcContext.GETTER_SLOTS


class OpenCLKernelContext(ContextWhitelist):
    """OpenCL kernel created from a program.
    """
    C_NAME = 'opencl_kernel_new'

    CONTEXT_TYPE = TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'program': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM),
                  'name': _TYPE_STRING}

    GETTER_SLOTS = {'program': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM),
                    'name': _TYPE_STRING,
                    'num_args': _TYPE_CL_UINT}

    SETTER_SLOTS = {'arg.local_mem_size': {1: _TYPE_SIZE},
                    'arg.value': {1: _TYPE_DATA},
                    'arg.mem_object': {1: TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_MEM_OBJECT)},
                    'arg.svm_ptr': {1: TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_SVM)},
                    'exec_info.svm_ptrs': _TYPE_DATA_PTR_ARRAY}


class OpenCLKernelCloneContext(ContextWhitelist):
    """OpenCL kernel cloned from another kernel.
    """
    C_NAME = 'opencl_kernel_clone'

    CONTEXT_TYPE = TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'kernel': TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)}

    GETTER_SLOTS = OpenCLKernelContext.GETTER_SLOTS

    SETTER_SLOTS = OpenCLKernelContext.SETTER_SLOTS

##############################################################################

def opencl_svm_interface(opencl_plugin, /):
    """Obtain OpenCL SVM interface.
    """
    return MemoryInterfaceSymbol.slot('opencl_svm', opencl_plugin)


def opencl_svm_flags(read=False, write=False, fine_grain=False, atomics=False):
    """Calculate OpenCL SVM flags.
    """
    if not isinstance(read, bool):
        raise TypeError
    elif not isinstance(write, bool):
        raise TypeError
    elif not isinstance(fine_grain, bool):
        raise TypeError
    elif not isinstance(atomics, bool):
        raise TypeError

    if not read and not write:
        raise ValueError
    elif not fine_grain and atomics:
        raise ValueError

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

    return mem_flags


def opencl_mem_map_flags(read=False, write=False, invalidate=False):
    """Calculate OpenCL SVM map flags.
    """
    if not isinstance(read, bool):
        raise TypeError
    elif not isinstance(write, bool):
        raise TypeError
    elif not isinstance(invalidate, bool):
        raise TypeError

    if not read and not write:
        raise ValueError
    elif invalidate and (read or not write):
        raise ValueError

    if write and invalidate:
        map_flags = 1 << 2
    else:
        map_flags = 0
        if read:
            map_flags |= 1 << 0
        if write:
            map_flags |= 1 << 1

    return map_flags


def new_opencl_svm_alloc_data(registry, key, opencl_plugin, /, cl_context=None, mem_flags=None):
    """Create OpenCL SVM allocation function data.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if cl_context is not None and not TypeAttr.compatible(
            TypeAttr.of(cl_context),
            TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT)):
        raise TypeError

    if isinstance(mem_flags, int):
        if mem_flags < 0:
            raise ValueError

        mem_flags = PrimitiveData(c.c_ulong(mem_flags))
    elif mem_flags is not None and not TypeAttr.compatible(
            TypeAttr.of(mem_flags), TypeAttr.from_type(c.c_ulong)):
        raise TypeError

    alloc_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        MemoryAllocDataSymbol.full_name('opencl_svm'), opencl_plugin))

    if cl_context is not None:
        registry(alloc_data.member.context << cl_context)
    if mem_flags is not None:
        registry(alloc_data.member.mem_flags << mem_flags)

    return alloc_data


def new_opencl_svm_map_data(registry, key, opencl_plugin, /, command_queue=None, map_flags=None):
    """Create OpenCL SVM mapping function data.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if command_queue is not None and not TypeAttr.compatible(
            TypeAttr.of(command_queue),
            TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE)):
        raise TypeError

    if isinstance(map_flags, int):
        if map_flags < 0:
            raise ValueError

        map_flags = PrimitiveData(c.c_ulong(map_flags))
    elif map_flags is not None and not TypeAttr.compatible(
            TypeAttr.of(map_flags), TypeAttr.from_type(c.c_ulong)):
        raise TypeError

    map_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        MemoryMapDataSymbol.full_name('opencl_svm'), opencl_plugin))

    if command_queue is not None:
        registry(map_data.member.command_queue << command_queue)
    if map_flags is not None:
        registry(map_data.member.map_flags << map_flags)

    return map_data


def new_opencl_kernel_enqueue_data(registry, key, opencl_plugin, /,
                                   command_queue=None, kernel=None,
                                   num_work_dimensions=None, work_offset_global=None,
                                   work_size_global=None, work_size_local=None):
    """Create OpenCL kernel enqueueing function data.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if command_queue is not None and not TypeAttr.compatible(
            TypeAttr.of(command_queue),
            TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE)):
        raise TypeError

    if kernel is not None and not TypeAttr.compatible(
            TypeAttr.of(kernel),
            TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)):
        raise TypeError

    if isinstance(work_offset_global, int):
        if work_offset_global < 0:
            raise ValueError

        work_offset_global = PrimitiveData(c.c_size_t(work_offset_global))
    elif isinstance(work_offset_global, tuple):
        for offset in work_offset_global:
            if not isinstance(offset, int):
                raise TypeError
            elif offset < 0:
                raise ValueError

        work_offset_global = PrimitiveData((c.c_size_t * len(work_offset_global))(*work_offset_global))
    elif work_offset_global is not None and not TypeAttr.compatible(
            TypeAttr.of(work_offset_global), TypeAttr.from_type(c.c_size_t)):
        raise TypeError

    if isinstance(work_size_global, int):
        if work_size_global < 0:
            raise ValueError

        work_size_global = PrimitiveData(c.c_size_t(work_size_global))
    elif isinstance(work_size_global, tuple):
        for offset in work_size_global:
            if not isinstance(offset, int):
                raise TypeError
            elif offset < 0:
                raise ValueError

        work_size_global = PrimitiveData((c.c_size_t * len(work_size_global))(*work_size_global))
    elif work_size_global is not None and not TypeAttr.compatible(
            TypeAttr.of(work_size_global), TypeAttr.from_type(c.c_size_t)):
        raise TypeError

    if isinstance(work_size_local, int):
        if work_size_local < 0:
            raise ValueError

        work_size_local = PrimitiveData(c.c_size_t(work_size_local))
    elif isinstance(work_size_local, tuple):
        for offset in work_size_local:
            if not isinstance(offset, int):
                raise TypeError
            elif offset < 0:
                raise ValueError

        work_size_local = PrimitiveData((c.c_size_t * len(work_size_local))(*work_size_local))
    elif work_size_local is not None and not TypeAttr.compatible(
            TypeAttr.of(work_size_local), TypeAttr.from_type(c.c_size_t)):
        raise TypeError

    if isinstance(num_work_dimensions, int):
        if num_work_dimensions <= 0:
            raise ValueError("Number of work dimensions must be positive")

        if isinstance(work_offset_global, Object) and work_offset_global.length != num_work_dimensions:
            raise ValueError("Global work offset has wrong number of dimensions")
        elif isinstance(work_size_global, Object) and work_size_global.length != num_work_dimensions:
            raise ValueError("Global work size has wrong number of dimensions")
        elif isinstance(work_size_local, Object) and work_size_local.length != num_work_dimensions:
            raise ValueError("Local work size has wrong number of dimensions")

        num_work_dimensions = PrimitiveData(c.c_size_t(num_work_dimensions))
    elif num_work_dimensions is not None and not TypeAttr.compatible(
            TypeAttr.of(num_work_dimensions), TypeAttr.from_type(c.c_size_t)):
        raise TypeError

    enqueue_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        DexgraphOperationDataSymbol.full_name('opencl_kernel_enqueue'), opencl_plugin))

    if command_queue is not None:
        registry(enqueue_data.member.command_queue << command_queue)
    if kernel is not None:
        registry(enqueue_data.member.kernel << kernel)
    if num_work_dimensions is not None:
        registry(enqueue_data.member.num_work_dimensions << num_work_dimensions)
    if work_offset_global is not None:
        registry(enqueue_data.member.work_offset_global << work_offset_global)
    if work_size_global is not None:
        registry(enqueue_data.member.work_size_global << work_size_global)
    if work_size_local is not None:
        registry(enqueue_data.member.work_size_local << work_size_local)

    return enqueue_data


def new_opencl_event_wait_data(registry, key, opencl_plugin, /):
    """Create OpenCL event waiting function data.
    """
    return new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        'opencl_event_array', opencl_plugin))

##############################################################################

class OpenCLSVMAllocationProcedure(MemoryAllocationProcedure):
    """OpenCL SVM allocation and initialization procedure.
    """
    def __init__(self, allocations, opencl_plugin, /, alloc_data, map_data=None):
        """Initialize a procedure.
        """
        if alloc_data is None:
            raise ValueError
        elif not TypeAttr.compatible(
                TypeAttr.of(alloc_data),
                TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_SVM_ALLOC_DATA)):
            raise TypeError
        elif map_data is not None and not TypeAttr.compatible(
                TypeAttr.of(map_data),
                TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_SVM_MAP_DATA)):
            raise TypeError

        if map_data is None:
            for alloc in allocations.values():
                if alloc.init_fn is not None:
                    raise ValueError("OpenCL SVM mapping data is not specified -- cannot initialize memory")

        super().__init__(allocations,
                         interface=MemoryInterfaceSymbol.slot('opencl_svm', opencl_plugin),
                         alloc_data=alloc_data,
                         map_data=map_data)


class _OpenCLEventOrderingGraphNode:
    """Description of an OpenCL event ordering graph node.
    """
    def __init__(self, /, wait_list=None, out_list=None, waits_for=None):
        """Initialize a graph node.
        """
        if not isinstance(wait_list, (NoneType, Context, Context.Slot)):
            raise TypeError
        elif not isinstance(out_list, (NoneType, Context, Context.Slot)):
            raise TypeError

        if waits_for is not None:
            if not all(isinstance(key, str) for key in waits_for):
                raise TypeError
        else:
            waits_for = ()

        self._wait_list = wait_list
        self._out_list = out_list

        self._waits_for = {key: index for index, key in enumerate(set(waits_for))}

    @property
    def wait_list(self, /):
        """Get wait list.
        """
        return self._wait_list

    @property
    def out_list(self, /):
        """Get output event list.
        """
        return self._out_list

    @property
    def waits_for(self, /):
        """Get set of node key the node is waiting for.
        """
        return frozenset(self._waits_for.keys())


class OpenCLEventOrderingGraphProcedure(Procedure):
    """OpenCL event ordering graph construction procedure.
    """
    Node = _OpenCLEventOrderingGraphNode

    class _CleanupProcedure(Procedure):
        """OpenCL event ordering graph cleanup procedure.
        """
        def __init__(self, nodes, /):
            self._nodes = nodes

        def _impl(self, registry, /, prefix):
            zero = PrimitiveData(c.c_size_t(0))

            for node in self._nodes:
                if node.wait_list is not None:
                    registry(node.wait_list.num_events << zero)
                    registry(node.wait_list.event << None)

                if node.out_list is not None:
                    registry(node.out_list.num_event_ptrs << zero)
                    registry(node.out_list.event_ptr << None)

    def __init__(self, nodes, /):
        """Initialize a procedure.
        """
        if not isinstance(nodes, dict):
            raise TypeError

        keys = set(nodes.keys())
        waited_by = {key: {} for key in nodes.keys()}

        for key, node in nodes.items():
            if not isinstance(key, str):
                raise TypeError
            elif not isinstance(node, self.__class__.Node):
                raise TypeError
            elif not keys.issuperset(node.waits_for):
                raise KeyError("Node {repr(key)} refers to node(s) that are not in the dictionary of nodes")
            elif key in node.waits_for:
                raise KeyError("Node {repr(key)} refers to itself")

            if node.waits_for and node.wait_list is None:
                raise ValueError("Wait list is not specified")

            for other_key in node.waits_for:
                waited_by[other_key][key] = len(waited_by[other_key])

                other_node = nodes[other_key]

                if other_node.out_list is None:
                    raise ValueError("Event output list is not specified")

        self._nodes = nodes.copy()
        self._waited_by = waited_by

        self._cleanup = self.__class__._CleanupProcedure(self._nodes)

    @property
    def nodes(self, /):
        """Get the dictionary of graph nodes.
        """
        return MappingProxyType(self._nodes)

    @property
    def cleanup(self, /):
        """Get the cleanup procedure.
        """
        return self._cleanup

    def _impl(self, registry, /, prefix):
        """Implementation of the procedure.

        Returns nothing.
        """
        # Create pointer array contexts (event lists)
        wait_list_contexts = {}
        out_list_contexts = {}

        for key, node in self.nodes.items():
            if node.waits_for:
                wait_list_contexts[key] = registry.new_context(
                        registry.temp_key(f'{key}.wait_list', prefix=prefix),
                        (None,) * len(node.waits_for))
            else:
                wait_list_contexts[key] = None

            if self._waited_by[key]:
                out_list_contexts[key] = registry.new_context(
                        registry.temp_key(f'{key}.out_list', prefix=prefix),
                        (None,) * len(self._waited_by))
            else:
                out_list_contexts[key] = None

        # Assign event lists
        for key, node in self.nodes.items():
            if node.wait_list is not None:
                registry(node.wait_list.num_events
                         << PrimitiveData(c.c_size_t(len(node.waits_for))))
                if wait_list_contexts[key] is not None:
                    registry(node.wait_list.event << wait_list_contexts[key].ptrs)

            if node.out_list is not None:
                registry(node.out_list.num_event_ptrs
                         << PrimitiveData(c.c_size_t(len(self._waited_by[key]))))
                if out_list_contexts[key] is not None:
                    registry(node.out_list.event_ptr << out_list_contexts[key].ptrs)

        # Assign pointers to events
        for key, node in self.nodes.items():
            wait_list_context = wait_list_contexts[key]

            for other_key, wait_list_index in node._waits_for.items():
                out_list_context = out_list_contexts[other_key]
                out_list_index = self._waited_by[other_key][key]

                registry(out_list_context[out_list_index]
                         << Context.Slot.weak_ref(wait_list_context[wait_list_index].ptr))

        # Delete temporary contexts
        for context in wait_list_contexts.values():
            registry.delete(context)

        for context in out_list_contexts.values():
            registry.delete(context)

