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

from .object import PrimitiveData
from archi.context import (
        AggregateContext,
        AggregateInterfaceSymbol,
        AggregateTypeSymbol,
        DexgraphOperationDataSymbol,
        )
from .registry import Registry


### archi/aggr ###

def new_aggregate_object(registry, key, /, interface=None, metadata=None, fam_length=None):
    """Create an aggregate object context.

    Generic aggregate object interface is used by default.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    executable = registry[registry.__class__.KEY_EXECUTABLE]

    if interface is None:
        if metadata is None:
            raise ValueError('No aggregate type description is specified')

        interface = AggregateInterfaceSymbol.slot('generic', executable)

    params = {}
    if fam_length is not None:
        params['fam_length'] = fam_length

    I_AGGREGATE = AggregateContext.interface_in(executable)
    return registry.new_context(I_AGGREGATE(interface=interface, metadata=metadata, **params),
                                key=key)

### archi/thread ###

def new_thread_group_dispatch_data(registry, key, /, thread_group=None,
                                   work_func=None, work_data=None,
                                   callback_func=None, callback_data=None,
                                   work_offset=None, work_size=None, batch_size=None):
    """Create thread group dispatching function data.
    """
    if not isinstance(work_offset, (type(None), int)):
        raise TypeError
    elif work_offset is not None and work_offset < 0:
        raise ValueError
    elif not isinstance(work_size, (type(None), int)):
        raise TypeError
    elif work_size is not None and work_size < 0:
        raise ValueError
    elif not isinstance(batch_size, (type(None), int)):
        raise TypeError
    elif batch_size is not None and batch_size < 0:
        raise ValueError

    executable = registry[registry.__class__.KEY_EXECUTABLE]

    dispatch_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        DexgraphOperationDataSymbol.full_name('thread_group_dispatch'), executable))

    if thread_group is not None:
        dispatch_data.member.thread_group = thread_group
    if work_func is not None:
        dispatch_data.member.work.function = work_func
    if work_data is not None:
        dispatch_data.member.work.data = work_data
    if callback_func is not None:
        dispatch_data.member.callback.function = callback_func
    if callback_data is not None:
        dispatch_data.member.callback.data = callback_data

    if work_offset is not None:
        dispatch_data.member.param.offset = PrimitiveData(c.c_size_t(work_offset))
    if work_size is not None:
        dispatch_data.member.param.size = PrimitiveData(c.c_size_t(work_size))
    if batch_size is not None:
        dispatch_data.member.param.batch_size = PrimitiveData(c.c_size_t(batch_size))

    return dispatch_data

