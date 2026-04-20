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

from contextlib import contextmanager
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
from archi.helper.aggr import aggregate_object_spec


def opencl_svm_interface(plugin):
    """Obtain memory interface for OpenCL SVM.
    """
    return MemoryInterfaceSymbol.slot(plugin, 'opencl_svm')


@contextmanager
def opencl_svm_alloc_data(registry, plugin, cl_context, /, read=False, write=False,
                          fine_grain=False, atomics=False, key=None):
    """Context manager for OpenCL SVM allocation data context.
    """
    if not read and not write:
        raise ValueError
    elif not fine_grain and atomics:
        raise ValueError

    with registry.temp_context(aggregate_object_spec(registry, plugin,
                                                     MemoryAllocDataSymbol.PREFIX + 'opencl_svm'),
                               key=key) as alloc_data:
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

        yield alloc_data


@contextmanager
def opencl_svm_map_data(registry, plugin, command_queue, read=False, write=False, invalidate=False):
    """Context manager for OpenCL SVM mapping data context.
    """
    if not read and not write:
        raise ValueError
    elif invalidate and (read or not write):
        raise ValueError

    with registry.temp_context(aggregate_object_spec(registry, plugin,
                                                     MemoryMapDataSymbol.PREFIX + 'opencl_svm'),
                               key=key) as map_data:
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

        yield map_data


class _OpenCLEvent:
    def __init__(self):
        self._out_list = {}

    @property
    def out_list(self):
        return frozenset(self._out_list.keys())


class OpenCLKernelExecution(_OpenCLEvent):
    """Helper class representing OpenCL kernel enqueueing function data.
    """
    def __init__(self, command_queue, kernel, global_work_size, global_work_offset=None, local_work_size=None):
        """Initialize kernel enqueueing function data.
        """
        # Perform necessary checks
        if command_queue is None or kernel is None:
            raise ValueError

        if global_work_size is None:
            if global_work_offset is not None or local_work_size is not None:
                raise ValueError
        else:
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

            if global_work_offset is not None:
                if isinstance(global_work_offset, int):
                    global_work_offset = (global_work_offset,)
                elif not isinstance(global_work_offset, tuple):
                    raise TypeError
                elif len(global_work_offset) != self._num_dimensions:
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
                elif len(local_work_size) != self._num_dimensions:
                    raise ValueError

                for elt in local_work_size:
                    if not isinstance(elt, int):
                        raise TypeError
                    elif elt < 0:
                        raise ValueError

        # Initialize the data object
        self._command_queue = command_queue
        self._kernel = kernel

        self._num_dimensions = len(global_work_size) if global_work_size is not None \
                else 1 # 0 dimensions is not allowed by clEnqueueNDRangeKernel()
        self._global_work_size = global_work_size
        self._global_work_offset = global_work_offset
        self._local_work_size = local_work_size

        self._wait_list = {}

        super().__init__(self)

    @property
    def command_queue(self):
        return self._command_queue

    @property
    def kernel(self):
        return self._kernel

    @property
    def num_dimensions(self):
        return self._num_dimensions

    @property
    def global_work_size(self):
        return self._global_work_size

    @property
    def global_work_offset(self):
        return self._global_work_offset

    @property
    def local_work_size(self):
        return self._local_work_size

    @property
    def wait_list(self):
        return frozenset(self._wait_list.keys())

    def wait_for(self, operation, /):
        """Add an operation to the wait list.
        """
        if not isinstance(operation, _OpenCLEvent):
            raise TypeError
        elif operation is self:
            raise ValueError

        self._wait_list[operation] = len(self._wait_list)
        operation._out_list[self] = len(operation._out_list)

    def _new_context(self, registry, plugin, /):
        """Get context specification to create a context.
        """
        context = registry.new_context(aggregate_object_spec(registry, plugin,
                                                             DexgraphOperationDataSymbol.PREFIX \
                                                                     + 'opencl_kernel_enqueue'),
                                       key=registry.temp_key(prefix='opencl_kernel_enqueue_data'))

        context.member.command_queue = self.command_queue
        context.member.kernel = self.kernel

        context.member.num_work_dimensions = PrimitiveData(c.c_size_t(self.num_dimensions))
        if self.global_work_offset is not None:
            context.member.work_offset_global = PrimitiveData(
                    (c.c_size_t * len(self.global_work_offset))(*self.global_work_offset))
        if self.global_work_size is not None:
            context.member.work_size_global = PrimitiveData(
                    (c.c_size_t * len(self.global_work_size))(*self.global_work_size))
        if self.local_work_size is not None:
            context.member.work_size_local = PrimitiveData(
                    (c.c_size_t * len(self.local_work_size))(*self.local_work_size))

        wait_list_context = registry.new_context((None,) * len(self._wait_list),
                                                 key=registry.temp_key())
        out_list_context = registry.new_context((None,) * len(self._out_list),
                                                key=registry.temp_key())

        context.member.wait_list.num_events = PrimitiveData(c.c_size_t(len(self._wait_list)))
        context.member.wait_list.event = wait_list_context.ptrs
        context.member.out_list.num_events = PrimitiveData(c.c_size_t(len(self._out_list)))
        context.member.out_list.event = out_list_context.ptrs

        return context, wait_list_context, out_list_context


class OpenCLEventWaiting:
    """Helper class representing OpenCL event waiting function data.
    """
    def __init__(self):
        """Initialize kernel waiting function data.
        """
        self._wait_list = {}

    @property
    def wait_list(self):
        return frozenset(self._wait_list.keys())

    def wait_for(self, operation, /):
        """Add an operation to the wait list.
        """
        if not isinstance(operation, _OpenCLEvent):
            raise TypeError

        self._wait_list[operation] = len(self._wait_list)
        operation._out_list[self] = len(operation._out_list)

    def _new_context(self, registry, plugin, /):
        """Get context specification to create a context.
        """
        context = registry.new_context(aggregate_object_spec(registry, plugin, 'opencl_event_array'),
                                       key=registry.temp_key(prefix='opencl_event_wait_data'))

        wait_list_context = registry.new_context((None,) * len(self._wait_list),
                                                 key=registry.temp_key())

        context.member.num_events = PrimitiveData(c.c_size_t(len(self._wait_list)))
        context.member.event = wait_list_context.ptrs

        return context, wait_list_context


@contextmanager
def opencl_operation_data(registry, plugin, /, *specs):
    """DEG operation function data for OpenCL operations.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(plugin, LibraryContext):
        raise TypeError
    elif not all(isinstance(spec, (_OpenCLEvent, OpenCLEventWaiting)) for spec in specs):
        raise TypeError

    for spec in specs:
        if not spec.wait_list.issubset(specs):
            raise ValueError("Event wait list has an object not in the argument list")
        elif isinstance(spec, _OpenCLEvent) and not spec.out_list.issubset(specs):
            raise ValueError("Output event pointer list has an object not in the argument list")

    # Create contexts
    data_contexts = {}
    wait_list_contexts = {}
    out_list_contexts = {}

    for spec in specs:
        if isinstance(spec, _OpenCLEvent):
            context, wait_list_context, out_list_context = spec._new_context(registry, plugin)
            out_list_contexts[spec] = out_list_context
        else:
            context, wait_list_context = spec._new_context(registry, plugin)

        data_contexts[spec] = context
        wait_list_contexts[spec] = wait_list_context

    # Assign event pointers
    for spec in specs:
        if not isinstance(spec, _OpenCLEvent):
            continue

        out_list_context = out_list_contexts[spec]

        for target_spec, out_list_index in spec.out_list.items():
            wait_list_context = wait_list_contexts[target_spec]
            wait_list_index = target._wait_list[spec]

            # The assignment
            out_list_context[out_list_index] = Context.weak_ref(wait_list_context.ptr[wait_list_index])

    # Yield the contexts
    try:
        yield tuple(data_contexts.values())
    finally:
        # Delete all contexts
        for context in out_list_contexts:
            registry.del_context(context)

        for context in wait_list_contexts:
            registry.del_context(context)

        for context in data_contexts:
            registry.del_context(context)

