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
# @brief Context creation helpers.

import ctypes as c
from types import NoneType

import archi.ctypes as typ
from .object import PrimitiveData
from archi.context import (
        TypeAttr,
        Context,
        AggregateContext,
        ThreadGroupContext,
        AggregateInterfaceSymbol,
        AggregateTypeSymbol,
        DexgraphOperationDataSymbol,
        MemoryInterfaceSymbol,
        )
from .registry import Registry


### archi/aggr ###

def new_aggregate_object(registry, key, /, interface=None, metadata=None, fam_length=None):
    """Create an aggregate object context.

    Generic aggregate object interface is used by default.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if interface is None:
        if metadata is None:
            raise ValueError('No aggregate type description is specified')

        interface = AggregateInterfaceSymbol.slot('generic', registry.BUILTIN.executable)

    params = {}
    if fam_length is not None:
        params['fam_length'] = fam_length

    I_AGGREGATE = AggregateContext.interface_in(registry.BUILTIN.executable)
    return registry.new_context(key, I_AGGREGATE(interface=interface, metadata=metadata, **params))

### archi/thread ###

def new_thread_group_dispatch_data(registry, key, /, thread_group=None,
                                   work_func=None, work_data=None,
                                   callback_func=None, callback_data=None,
                                   work_offset=None, work_size=None, batch_size=None):
    """Create thread group dispatching function data.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if thread_group is not None and not TypeAttr.compatible(
            TypeAttr.of(thread_group),
            TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__THREAD_GROUP)):
        raise TypeError

    if work_func is not None and not TypeAttr.compatible(
            TypeAttr.of(work_func),
            TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__THREAD_WORK)):
        raise TypeError

    if work_data is not None and not TypeAttr.compatible(
            TypeAttr.of(work_data), TypeAttr.complex_data()):
        raise TypeError

    if callback_func is not None and not TypeAttr.compatible(
            TypeAttr.of(callback_func),
            TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__THREAD_CALLBACK)):
        raise TypeError

    if callback_data is not None and not TypeAttr.compatible(
            TypeAttr.of(callback_data), TypeAttr.complex_data()):
        raise TypeError

    if isinstance(work_offset, int):
        if work_offset < 0:
            raise ValueError

        work_offset = PrimitiveData(c.c_size_t(work_offset))
    elif work_offset is not None and not TypeAttr.compatible(
            TypeAttr.of(work_offset), TypeAttr.from_type(c.c_size_t)):
        raise TypeError

    if isinstance(work_size, int):
        if work_size < 0:
            raise ValueError

        work_size = PrimitiveData(c.c_size_t(work_size))
    elif work_size is not None and not TypeAttr.compatible(
            TypeAttr.of(work_size), TypeAttr.from_type(c.c_size_t)):
        raise TypeError

    if isinstance(batch_size, int):
        if batch_size < 0:
            raise ValueError

        batch_size = PrimitiveData(c.c_size_t(batch_size))
    elif batch_size is not None and not TypeAttr.compatible(
            TypeAttr.of(batch_size), TypeAttr.from_type(c.c_size_t)):
        raise TypeError

    dispatch_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        DexgraphOperationDataSymbol.full_name('thread_group_dispatch'), registry.BUILTIN.executable))

    if thread_group is not None:
        registry(dispatch_data.member.thread_group << thread_group)
    if work_func is not None:
        registry(dispatch_data.member.work.function << work_func)
    if work_data is not None:
        registry(dispatch_data.member.work.data << work_data)
    if callback_func is not None:
        registry(dispatch_data.member.callback.function << callback_func)
    if callback_data is not None:
        registry(dispatch_data.member.callback.data << callback_data)
    if work_offset is not None:
        registry(dispatch_data.member.param.offset << work_offset)
    if work_size is not None:
        registry(dispatch_data.member.param.size << work_size)
    if batch_size is not None:
        registry(dispatch_data.member.param.batch_size << batch_size)

    return dispatch_data

### archi/memory ###

def heap_memory_interface(executable, /):
    """Obtain heap memory interface.
    """
    return MemoryInterfaceSymbol.slot('heap', executable)

