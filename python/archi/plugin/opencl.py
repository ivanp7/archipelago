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


def new_opencl_kernel_set_argument_data(registry, key, opencl_plugin, /,
                                        kernel=None, arg_index=None, value=None):
    """Create OpenCL kernel argument setting function data.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if kernel is not None and not TypeAttr.compatible(
            TypeAttr.of(kernel),
            TypeAttr.complex_data(ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)):
        raise TypeError

    if isinstance(arg_index, int):
        if arg_index < 0:
            raise ValueError

        arg_index = PrimitiveData(c.c_uint(arg_index))
    elif arg_index is not None and not TypeAttr.compatible(
            TypeAttr.of(arg_index), TypeAttr.from_type(c.c_uint)):
        raise TypeError

    if value is not None and not TypeAttr.compatible(
            TypeAttr.of(value), TypeAttr.complex_data(0)):
        raise TypeError

    setarg_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        DexgraphOperationDataSymbol.full_name('opencl_kernel_set_argument'), opencl_plugin))

    if kernel is not None:
        registry(setarg_data.member.kernel << kernel)
    if arg_index is not None:
        registry(setarg_data.member.arg_index << arg_index)
    if value is not None:
        registry(setarg_data.member.value << value)

    return setarg_data


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


def new_opencl_event_profile_data(registry, key, opencl_plugin, /,
                                  timer=None, from_time='START', to_time='END'):
    """Create OpenCL event time recording function data.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if timer is not None and not TypeAttr.compatible(
            TypeAttr.of(timer),
            TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__TIMER)):
        raise TypeError

    profiling_info = {'QUEUED': 0x1280, 'SUBMIT': 0x1281,
                      'START': 0x1282, 'END': 0x1283,
                      'COMPLETE': 0x1284}

    if from_time not in profiling_info:
        raise  ValueError
    elif to_time not in profiling_info:
        raise  ValueError

    recording_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        DexgraphOperationDataSymbol.full_name('opencl_event_profile'), opencl_plugin))

    if timer is not None:
        registry(recording_data.member.timer << timer)
    registry(recording_data.member.from_time << PrimitiveData(c.c_uint(profiling_info[from_time])))
    registry(recording_data.member.to_time << PrimitiveData(c.c_uint(profiling_info[to_time])))

    return recording_data

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


class _OpenCLEvent:
    def __init__(self, /, event_ptr):
        """Initialize a description of an OpenCL event.
        """
        if not isinstance(event_ptr, Context.Slot):
            raise TypeError

        self._event_ptr = event_ptr

    @property
    def event_ptr(self, /):
        """Obtain the event slot.
        """
        return self._event_ptr


class _OpenCLEventArray:
    """Description of an array of (pointers to) OpenCL events.
    """
    def __init__(self, /, array, array_size=None):
        """Initialize a description of event array.
        """
        if not isinstance(array, Context.Slot):
            raise TypeError
        elif not isinstance(array_size, (NoneType, Context.Slot)):
            raise TypeError

        self._array = array
        self._array_size = array_size

    @property
    def event_array(self, /):
        """Obtain the event array slot.
        """
        return self._array

    @property
    def event_array_size(self, /):
        """Obtain the event array size slot.
        """
        return self._array_size


class OpenCLEventDependencyGraphProcedure(Procedure):
    """OpenCL event dependency graph construction procedure.
    """
    EventPtr = _OpenCLEvent
    EventArray = _OpenCLEventArray

    class _CleanupProcedure(Procedure):
        """OpenCL event dependency graph cleanup procedure.
        """
        def __init__(self, arrays, event_ptrs, /):
            self._arrays = arrays
            self._event_ptrs = event_ptrs

        def _impl(self, registry, /, prefix):
            for array in self._arrays:
                registry(array.event_array << None)

                if array.event_array_size is not None:
                    registry(array.event_array_size << PrimitiveData(c.c_size_t(0)))

            for event_ptr in self._event_ptrs:
                registry(event_ptr.event_ptr << None)

    def __init__(self, /, producers, consumers, dependencies):
        """Initialize a procedure.
        """
        if not isinstance(producers, dict):
            raise TypeError
        elif not isinstance(consumers, dict):
            raise TypeError
        elif not isinstance(dependencies, dict):
            raise TypeError

        for key, value in producers.items():
            if not isinstance(key, str):
                raise TypeError
            elif not isinstance(value, self.__class__.EventArray):
                raise TypeError

        for key, value in consumers.items():
            if not isinstance(key, str):
                raise TypeError
            elif not isinstance(value, (self.__class__.EventPtr, self.__class__.EventArray)):
                raise TypeError

        self._revdeps = {key: {} for key in producers.keys()}

        for consumer_key, value in dependencies.items():
            if consumer_key not in consumers:
                raise KeyError

            if isinstance(consumers[consumer_key], self.__class__.EventArray):
                value = frozenset(value)
            elif isinstance(consumers[consumer_key], self.__class__.EventPtr):
                if not isinstance(value, str):
                    raise TypeError

                value = frozenset((value,))

            for producer_key in value:
                if producer_key not in producers:
                    raise KeyError

                self._revdeps[producer_key][consumer_key] = len(self._revdeps[producer_key])

        self._producers = producers.copy()
        self._consumers = consumers.copy()
        self._dependencies = {key: {k: i for i, k in enumerate(value)}
                              for key, value in dependencies.items()}

        self._cleanup = self.__class__._CleanupProcedure(
                tuple(producers.values())
                + tuple(value for value in consumers.values()
                        if isinstance(value, self.__class__.EventArray)),
                tuple(value for value in consumers.values()
                 if isinstance(value, self.__class__.EventPtr)))

    @property
    def producers(self, /):
        """Get the dictionary of event producers.
        """
        return MappingProxyType(self._producers)

    @property
    def consumers(self, /):
        """Get the dictionary of event consumers.
        """
        return MappingProxyType(self._consumers)

    @property
    def dependencies(self, /):
        """Get the dictionary of producers for each consumer.
        """
        return MappingProxyType({key: frozenset(value.keys())
                                 for key, value in self._dependencies})

    @property
    def rev_dependencies(self, /):
        """Get the dictionary of consumers for each producer.
        """
        return MappingProxyType({key: frozenset(value.keys())
                                 for key, value in self._revdeps})

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
        out_list_contexts = {}

        for producer_key, consumers in self._revdeps.items():
            if consumers:
                out_list_contexts[producer_key] = registry.new_context(
                        registry.temp_key(f'{producer_key}.out_list', prefix=prefix),
                        (None,) * len(consumers))
            else:
                out_list_contexts[producer_key] = None

        wait_list_contexts = {}

        for consumer_key, producers in self._dependencies.items():
            if not isinstance(self._consumers[consumer_key], self.__class__.EventArray):
                continue

            if producers:
                wait_list_contexts[consumer_key] = registry.new_context(
                        registry.temp_key(f'{consumer_key}.wait_list', prefix=prefix),
                        (None,) * len(producers))
            else:
                wait_list_contexts[consumer_key] = None

        # Assign event lists
        for producer_key, context in out_list_contexts.items():
            producer = self._producers[producer_key]

            if context is not None:
                registry(producer.event_array << context.ptrs)

            if producer.event_array_size is not None:
                registry(producer.event_array_size << PrimitiveData(
                    c.c_size_t(len(self._revdeps[producer_key]))))

        for consumer_key, context in wait_list_contexts.items():
            consumer = self._consumers[consumer_key]

            if context is not None:
                registry(consumer.event_array << context.ptrs)

            if consumer.event_array_size is not None:
                registry(consumer.event_array_size << PrimitiveData(
                    c.c_size_t(len(self._dependencies[consumer_key]))))

        # Assign pointers to events
        for producer_key, context in out_list_contexts.items():
            for consumer_key, consumer_index in self._revdeps[producer_key].items():
                consumer = self._consumers[consumer_key]

                if isinstance(consumer, self.__class__.EventPtr):
                    event_ptr = consumer.event_ptr
                elif isinstance(consumer, self.__class__.EventArray):
                    wait_list = wait_list_contexts[consumer_key]
                    producer_index = self._dependencies[consumer_key][producer_key]

                    event_ptr = wait_list[producer_index].ptr

                registry(context[consumer_index] << Context.Slot.weak_ref(event_ptr))

        # Delete temporary contexts
        for context in wait_list_contexts.values():
            registry.delete(context)

        for context in out_list_contexts.values():
            registry.delete(context)

