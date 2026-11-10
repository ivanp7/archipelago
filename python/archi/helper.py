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
        DexgraphOperationSeqContext,
        DexgraphNodeContext,
        ThreadGroupContext,
        AggregateInterfaceSymbol,
        AggregateTypeSymbol,
        DexgraphOperationDataSymbol,
        DexgraphTransitionDataSymbol,
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

### archi/exec ###

def new_dexgraph_operation_seq(registry, key, /,
                               name=None, operations=None):
    """Create a directed execution graph operation sequence.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if name is not None and not isinstance(name, str):
        raise TypeError

    if operations is not None:
        for operation in operations:
            if operation is not None:
                if not isinstance(operation, tuple):
                    raise TypeError
                elif len(operation) > 2:
                    raise ValueError

                if len(operation) == 1:
                    if operation[0] is not None and not TypeAttr.compatible(
                            TypeAttr.of(operation[0]),
                            TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ)):
                        raise TypeError
                elif len(operation) == 2:
                    if operation[0] is not None:
                        if not TypeAttr.compatible(
                                TypeAttr.of(operation[0]),
                                TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)):
                            raise TypeError

                        if operation[1] is not None and not TypeAttr.compatible(
                                TypeAttr.of(operation[1]), TypeAttr.complex_data()):
                            raise TypeError
                    else:
                        if operation[1] is not None and not TypeAttr.compatible(
                                TypeAttr.of(operation[1]),
                                TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ)):
                            raise TypeError

    params = {}
    if name is not None:
        params['name'] = name
    if operations is not None:
        params['length'] = PrimitiveData(c.c_size_t(len(operations)))

    I_DEXGRAPH_OPERATION_SEQ = DexgraphOperationSeqContext.interface_in(registry.BUILTIN.executable)
    sequence = registry.new_context(key, I_DEXGRAPH_OPERATION_SEQ(**params))

    if operations is not None:
        for i, operation in enumerate(operations):
            if operation is not None:
                if len(operation) == 1:
                    if operation[0] is not None:
                        registry(sequence.data[i] << operation[0])
                elif len(operation) == 2:
                    if operation[0] is not None:
                        registry(sequence.function[i] << operation[0])
                    if operation[1] is not None:
                        registry(sequence.data[i] << operation[1])

    return sequence


def new_dexgraph_node(registry, key, /,
                      name=None, sequence=None, transition=None):
    """Create a directed execution graph node.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if name is not None and not isinstance(name, str):
        raise TypeError

    if sequence is not None and not TypeAttr.compatible(
            TypeAttr.of(sequence), TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ)):
        raise TypeError

    if transition is not None:
        if not isinstance(transition, tuple):
            raise TypeError
        elif len(transition) > 2:
            raise ValueError

        if len(transition) == 1:
                if transition[0] is not None and not TypeAttr.compatible(
                        TypeAttr.of(transition[0]),
                        TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)):
                    raise TypeError
        elif len(transition) == 2:
            if transition[0] is not None:
                if not TypeAttr.compatible(
                        TypeAttr.of(transition[0]),
                        TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION)):
                    raise TypeError

                if transition[1] is not None and not TypeAttr.compatible(
                        TypeAttr.of(transition[1]), TypeAttr.complex_data()):
                    raise TypeError
            else:
                if transition[1] is not None and not TypeAttr.compatible(
                        TypeAttr.of(transition[1]),
                        TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)):
                    raise TypeError

    params = {}
    if name is not None:
        params['name'] = name
    if sequence is not None:
        params['sequence'] = sequence
    if transition is not None:
        if len(transition) == 1:
            if transition[0] is not None:
                params['transition_data'] = transition[0]
        elif len(transition) == 2:
            if transition[0] is not None:
                params['transition_func'] = transition[0]
            if transition[1] is not None:
                params['transition_data'] = transition[1]

    I_DEXGRAPH_NODE = DexgraphNodeContext.interface_in(registry.BUILTIN.executable)
    node = registry.new_context(key, I_DEXGRAPH_NODE(**params))

    return node


def new_dexgraph_branch_func_data(registry, key, /,
                                  condition_func=None, condition_data=None,
                                  target_false=None, target_true=None):
    """Create transition data for the branching function.
    """
    if not isinstance(registry, Registry):
        raise TypeError

    if condition_func is not None and not TypeAttr.compatible(
            TypeAttr.of(condition_func),
            TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_CONDITION)):
        raise TypeError

    if condition_data is not None and not TypeAttr.compatible(
            TypeAttr.of(condition_data), TypeAttr.complex_data()):
        raise TypeError

    if target_false is not None and not TypeAttr.compatible(
            TypeAttr.of(target_false),
            TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)):
        raise TypeError

    if target_true is not None and not TypeAttr.compatible(
            TypeAttr.of(target_true),
            TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)):
        raise TypeError

    branch_data = new_aggregate_object(registry, key, metadata=AggregateTypeSymbol.slot(
        DexgraphTransitionDataSymbol.full_name('branch'), registry.BUILTIN.executable))

    if condition_func is not None:
        registry(branch_data.member.function << condition_func)
    if condition_data is not None:
        registry(branch_data.member.data << condition_data)
    if target_false is not None:
        registry(branch_data.member.target[0] << target_false)
    if target_true is not None:
        registry(branch_data.member.target[1] << target_true)

    return branch_data

### archi/thread ###

def new_thread_group_dispatch_func_data(registry, key, /, thread_group=None,
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

