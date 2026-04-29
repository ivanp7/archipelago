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
# @brief Helpers for OpenCL contexts.

import ctypes as c

from .object import PrimitiveData
from archi.context import (
        Registry,
        Context,
        LibraryContext,
        DexgraphOperationDataSymbol,
        MemoryAllocDataSymbol,
        MemoryMapDataSymbol,
        )
from archi.opencl import OpenCLContext
from archi.helper.aggr import aggregate_object


def opencl_svm_interface(plugin):
    """Obtain memory interface for OpenCL SVM.
    """
    return MemoryInterfaceSymbol.slot(plugin, 'opencl_svm')


def opencl_svm_alloc_data(registry, plugin, cl_context, /, read=False, write=False,
                          fine_grain=False, atomics=False, key=None):
    """Create a OpenCL SVM allocation data context.
    """
    if not read and not write:
        raise ValueError
    elif not fine_grain and atomics:
        raise ValueError

    alloc_data = aggregate_object(registry, plugin, MemoryAllocDataSymbol.PREFIX + 'opencl_svm',
                                  key=registry.key(key, tmp_prefix='opencl_svm_alloc_data'))

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

    alloc_data.member.context = cl_context
    alloc_data.member.mem_flags = PrimitiveData(c.c_ulong(mem_flags))

    return alloc_data


def opencl_svm_map_data(registry, plugin, command_queue, /,
                        read=False, write=False, invalidate=False, key=None):
    """Create a OpenCL SVM mapping data context.
    """
    if not read and not write:
        raise ValueError
    elif invalidate and (read or not write):
        raise ValueError

    map_data = aggregate_object(registry, plugin, MemoryMapDataSymbol.PREFIX + 'opencl_svm',
                                key=registry.key(key, tmp_prefix='opencl_svm_map_data'))

    if write and invalidate:
        map_flags = 1 << 2
    else:
        map_flags = 0
        if read:
            map_flags |= 1 << 0
        if write:
            map_flags |= 1 << 1

    map_data.member.command_queue = command_queue
    map_data.member.map_flags = PrimitiveData(c.c_ulong(map_flags))

    return map_data


def opencl_kernel_enqueue_data(registry, plugin, command_queue, kernel, /, global_work_size,
                               global_work_offset=None, local_work_size=None, key=None):
    """Create a OpenCL kernel enqueueing function data.
    """
    if global_work_size is not None:
        if isinstance(global_work_size, int):
            global_work_size = (global_work_size,)
        elif not isinstance(global_work_size, tuple):
            raise TypeError
        elif not global_work_size:
            raise ValueError

        for elt in global_work_size:
            if not isinstance(elt, int):
                raise TypeError
            elif elt < 0:
                raise ValueError

        num_dimensions = len(global_work_size)
    else:
        if global_work_offset is not None:
            raise ValueError
        elif local_work_size is not None:
            raise ValueError

        num_dimensions = 1 # 0 dimensions is not allowed by clEnqueueNDRangeKernel()

    if global_work_offset is not None:
        if isinstance(global_work_offset, int):
            global_work_offset = (global_work_offset,)
        elif not isinstance(global_work_offset, tuple):
            raise TypeError
        elif len(global_work_offset) != num_dimensions:
            raise ValueError

        for elt in global_work_offset:
            if not isinstance(elt, int):
                raise TypeError
            elif elt < 0:
                raise ValueError

    if local_work_size is not None:
        if isinstance(local_work_size, int):
            local_work_size = (local_work_size,)
        elif not isinstance(local_work_size, tuple):
            raise TypeError
        elif len(local_work_size) != num_dimensions:
            raise ValueError

        for elt in local_work_size:
            if not isinstance(elt, int):
                raise TypeError
            elif elt < 0:
                raise ValueError

    enqueue_data = aggregate_object(registry, plugin,
                                    DexgraphOperationDataSymbol.PREFIX + 'opencl_kernel_enqueue',
                                    key=registry.key(key, tmp_prefix='opencl_kernel_enqueue_data'))

    enqueue_data.member.command_queue = command_queue
    enqueue_data.member.kernel = kernel

    enqueue_data.member.num_work_dimensions = PrimitiveData(c.c_size_t(num_dimensions))

    vtype = c.c_size_t * num_dimensions

    if global_work_offset is not None:
        enqueue_data.member.work_offset_global = PrimitiveData(vtype(*global_work_offset))
    if global_work_size is not None:
        enqueue_data.member.work_size_global = PrimitiveData(vtype(*global_work_size))
    if local_work_size is not None:
        enqueue_data.member.work_size_local = PrimitiveData(vtype(*local_work_size))

    return enqueue_data


def opencl_event_wait_data(registry, plugin, /, key=None):
    """Create a OpenCL event waiting function data.
    """
    return aggregate_object(registry, plugin,
                            DexgraphOperationDataSymbol.PREFIX + 'opencl_event_array',
                            key=registry.key(key, tmp_prefix='opencl_event_wait_data'))


class OpenCLEventProducing:
    """Helper class representing an OpenCL operation that waits on a list of events and produces a result event.
    """
    def __init__(self, wait_list, out_list):
        """Initialize an instance.
        """
        self._context_wait_list = wait_list
        self._context_out_list = out_list

        self._wait_list = {}
        self._out_list = {}

    @property
    def wait_list(self):
        return frozenset(self._wait_list.keys())

    @property
    def out_list(self):
        return frozenset(self._out_list.keys())

    def waits_for(self, operation, /):
        """Add an operation to the wait list.
        """
        if not isinstance(operation, OpenCLEventProducing):
            raise TypeError
        elif operation is self:
            raise ValueError

        self._wait_list[operation] = len(self._wait_list)
        operation._out_list[self] = len(operation._out_list)


class OpenCLEventWaiting:
    """Helper class representing an OpenCL operation that waits on a list of events.
    """
    def __init__(self, wait_list):
        """Initialize an instance.
        """
        self._context_wait_list = wait_list

        self._wait_list = {}

    @property
    def wait_list(self):
        return frozenset(self._wait_list.keys())

    def waits_for(self, operation, /):
        """Add an operation to the wait list.
        """
        if not isinstance(operation, OpenCLEventProducing):
            raise TypeError

        self._wait_list[operation] = len(self._wait_list)
        operation._out_list[self] = len(operation._out_list)


def alloc_event_lists(self, registry, /, *args):
    """Allocate events lists for objects referenced in arguments, and set pointers to events.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    for arg in args:
        if not isinstance(arg, (OpenCLEventProducing, OpenCLEventWaiting)):
            raise TypeError
        elif not arg.wait_list.issubset(args):
            raise ValueError("Event wait list contains an object that is not in the argument list")
        elif isinstance(arg, OpenCLEventProducing) and not arg.out_list.issubset(args):
            raise ValueError("Output event pointer list contains an object that not in the argument list")

    # Create temporary array contexts
    wait_list_contexts = {}
    out_list_contexts = {}

    for arg in args:
        if isinstance(arg, OpenCLEventProducing) and arg.out_list:
            out_list_contexts[arg] = registry.new_context((None,) * len(arg.out_list),
                                                          key=registry.key(tmp_prefix='dptr_array'))

        if arg.wait_list:
            wait_list_contexts[arg] = registry.new_context((None,) * len(arg.wait_list),
                                                           key=registry.key(tmp_prefix='dptr_array'))

    # Assign event lists
    for arg in args:
        if arg in wait_list_contexts:
            arg._context_wait_list.num_events = wait_list_contexts[arg].length
            arg._context_wait_list.event = wait_list_contexts[arg].ptrs

        if arg in out_list_contexts:
            arg._context_out_list.num_event_ptrs = out_list_contexts[arg].length
            arg._context_out_list.event_ptr = out_list_contexts[arg].ptrs

    # Assign pointers to events
    for arg in args:
        if not isinstance(arg, OpenCLEventProducing):
            continue

        out_list_context = out_list_contexts[arg]

        for target_spec, out_list_index in arg._out_list.items():
            wait_list_context = wait_list_contexts[target_spec]
            wait_list_index = target_spec._wait_list[arg]

            out_list_context[out_list_index] = Context.weak_ref(wait_list_context.ptr[wait_list_index])

    # Delete temporary contexts
    for context in wait_list_contexts.values():
        registry.del_context(context)

    for context in out_list_contexts.values():
        registry.del_context(context)

