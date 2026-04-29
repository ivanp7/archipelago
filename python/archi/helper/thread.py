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
# @brief Helpers for concurrent execution contexts.

import ctypes as c

from .object import PrimitiveData
from archi.context import (
        Registry,
        DexgraphOperationDataSymbol,
        )
from archi.helper.aggr import aggregate_object


def thread_group_dispatch_data(registry, thread_group, work_func, work_data,
                               work_size, /, work_offset=0, batch_size=0,
                               callback_func=None, callback_data=None, key=None):
    """Context manager for thread group dispatch operation data.
    """
    if not isinstance(work_size, int):
        raise TypeError
    elif work_size < 0:
        raise ValueError
    elif not isinstance(work_offset, int):
        raise TypeError
    elif work_offset < 0:
        raise ValueError
    elif not isinstance(batch_size, int):
        raise TypeError
    elif batch_size < 0:
        raise ValueError

    dispatch_data = aggregate_object(registry, registry[Registry.KEY_EXECUTABLE],
                                     DexgraphOperationDataSymbol.PREFIX + 'thread_group_dispatch',
                                     key=registry.key(key, tmp_prefix='thread_group_dispatch_data'))

    dispatch_data.member.thread_group = thread_group
    dispatch_data.member.work.function = work_func
    dispatch_data.member.work.data = work_data
    if callback_func is not None:
        dispatch_data.member.callback.function = callback_func
    if callback_data is not None:
        dispatch_data.member.callback.data = callback_data
    dispatch_data.member.param.offset = PrimitiveData(c.c_size_t(work_offset))
    dispatch_data.member.param.size = PrimitiveData(c.c_size_t(work_size))
    dispatch_data.member.param.batch_size = PrimitiveData(c.c_size_t(batch_size))

    return dispatch_data

