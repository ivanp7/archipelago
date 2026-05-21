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
# @brief Context registry procedures.

from contextlib import contextmanager
from types import MappingProxyType, NoneType

import archi.ctypes as typ
from .object import Object, PrimitiveData
from .context import TypeAttr, Context
import archi.context as ctx
from .registry import Registry
from .helper import heap_memory_interface


class Procedure:
    """Base class for registry procedure -- high-level representation of a sequence of registry operations.
    """
    def __call__(self, registry, /, prefix=''):
        """Operate on the specified registry.
        """
        if not isinstance(registry, Registry):
            raise TypeError(f"{registry} is not a context registry")
        elif not isinstance(prefix, str):
            raise TypeError(f"Key prefix must be a string")

        return self._impl(registry, prefix)

    @contextmanager
    def finalized(self, other, registry, /, prefix=''):
        """Context manager for the procedure finalized by another procedure.
        """
        if not isinstance(other, (NoneType, Procedure)):
            raise TypeError

        yield self(registry, prefix)

        if other is not None:
            other(registry, prefix)

    def _impl(self, registry, /, prefix):
        """Implementation of the procedure.
        """
        raise NotImplementedError

##############################################################################
# Built-in procedures
##############################################################################

class ContextDeletionProcedure(Procedure):
    """Deletion of a group of contexts in the specified order.
    """
    def __init__(self, /, keys):
        """Initialize a procedure.
        """
        if not all(isinstance(key, str) for key in keys):
            raise TypeError

        self._keys = tuple(keys)

    @property
    def keys(self, /):
        """Get the tuple of deleted keys.
        """
        return self._keys

    def _impl(self, registry, /, prefix):
        """Implementation of the procedure.
        """
        for key in self.keys:
            del registry[Registry.key(key, prefix=prefix)]

##############################################################################
# Built-in procedure factories
##############################################################################

### archi/exec ###

class _DirectedExecutionGraphNode:
    """Description of a DEG node.
    """
    def __init__(self, /, name=None, sequence=None, transition=None, branches=None):
        """Initialize a DEG node.
        """
        if not isinstance(name, (NoneType, str)):
            raise TypeError
        elif not isinstance(branches, (NoneType, dict)):
            raise TypeError

        if transition is not None:
            transition_func, transition_data = transition

            if transition_func is not None and not TypeAttr.compatible(
                    TypeAttr.of(transition_func),
                    TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION)):
                raise TypeError
            elif transition_data is not None and not TypeAttr.compatible(
                    TypeAttr.of(transition_data), TypeAttr.complex_data()):
                raise TypeError

        if sequence is not None:
            for operation in sequence:
                if operation is None:
                    continue

                operation_func, operation_data = operation

                if operation_func is not None and not TypeAttr.compatible(
                        TypeAttr.of(operation_func),
                        TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)):
                    raise TypeError
                elif operation_data is not None and not TypeAttr.compatible(
                        TypeAttr.of(operation_data), TypeAttr.complex_data()):
                    raise TypeError

        if branches is not None:
            for branch_idx, branch_key in branches.items():
                if not isinstance(branch_idx, int):
                    raise TypeError
                elif branch_idx < 0:
                    raise ValueError
                elif not isinstance(branch_key, str):
                    raise TypeError

        self._name = name
        self._sequence = tuple(tuple(operation) if operation is not None else None \
                for operation in sequence) if sequence is not None else ()
        self._transition = tuple(transition) if transition is not None else None
        self._branches = branches.copy() if branches is not None else {}

        if branches is not None:
            branches_list = [None] * (max(branches.keys()) + 1) if branches else []
            for index, target_key in branches.items():
                branches_list[index] = target_key

            self._branches_tuple = tuple(branches_list)
        else:
            self._branches_tuple = ()

    @property
    def name(self, /):
        """Get node name.
        """
        return self._name

    @property
    def sequence(self, /):
        """Get node operation sequence.
        """
        return self._sequence

    @property
    def transition(self, /):
        """Get node transition.
        """
        return self._transition

    def branches(self, /, as_tuple=False):
        """Get dictionary of node branches.
        """
        return self._branches_tuple if as_tuple else MappingProxyType(self._branches)


class DirectedExecutionGraphProcedure(Procedure):
    """Directed execution graph construction procedure.
    """
    Node = _DirectedExecutionGraphNode

    def __init__(self, nodes, /):
        """Initialize a procedure.
        """
        if not isinstance(nodes, dict):
            raise TypeError

        keys = set(nodes.keys())

        for key, node in nodes.items():
            if not isinstance(key, str):
                raise TypeError
            elif not isinstance(node, self.__class__.Node):
                raise TypeError
            elif not keys.issuperset(node.branches().values()):
                raise KeyError("Node {repr(key)} refers to node(s) that are not in the dictionary of nodes")

        self._nodes = nodes.copy()

        self._cleanup = ContextDeletionProcedure(keys=tuple(nodes.keys()))

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

        Returns dictionary of node contexts.
        """
        I_DEXGRAPH_NODE_ARRAY = ctx.DexgraphNodeArrayContext.interface_in(registry.BUILTIN.executable)
        I_DEXGRAPH_NODE = ctx.DexgraphNodeContext.interface_in(registry.BUILTIN.executable)

        # Create node array contexts
        node_array_contexts = {(): None}

        for node in self.nodes.values():
            branches = node.branches(as_tuple=True)
            if branches in node_array_contexts:
                continue

            node_array_contexts[branches] = registry.new_context(
                    registry.temp_key('node_array', prefix=prefix),
                    I_DEXGRAPH_NODE_ARRAY(num_nodes=len(branches)))

        # Create node contexts
        node_contexts = {}

        for key, node in self.nodes.items():
            # Create a node context
            params = {}

            if node.name is not None:
                params['name'] = node.name

            if node.transition is not None:
                transition_func, transition_data = node.transition

                if transition_func is not None:
                    params['transition_func'] = transition_func
                if transition_data is not None:
                    params['transition_data'] = transition_data

            context = node_contexts[key] = registry.new_context(
                    Registry.key(key, prefix=prefix),
                    I_DEXGRAPH_NODE(sequence_length=len(node.sequence),
                                    branches=node_array_contexts[node.branches(as_tuple=True)],
                                    **params))

            # Set node sequence operations
            for index, operation in enumerate(node.sequence):
                if operation is None:
                    continue

                operation_func, operation_data = operation

                if operation_func is not None:
                    registry(context.sequence.function[index] << operation_func)
                if operation_data is not None:
                    registry(context.sequence.data[index] << operation_data)

        # Fill node branch arrays
        for branches, context in node_array_contexts.items():
            for index, target_key in enumerate(branches):
                if target_key is not None:
                    registry(context.node[index] << Context.Slot.weak_ref(node_contexts[target_key]))

        # Delete node array contexts
        for context in node_array_contexts.values():
            registry.delete(context)

        return node_contexts

### archi/memory ###

class _MemoryAllocation:
    """Description of a memory allocation.
    """
    @staticmethod
    def init_from_memory(contents, /):
        """Get the memory initialization function (contents copied from memory).
        """
        if not isinstance(contents, (Context, Context.Slot)):
            raise TypeError

        def func(registry, mapping, prefix, /):
            I_PDPTR = ctx.PrimitiveDataPointerContext.interface_in(registry.BUILTIN.executable)

            with registry.temp_context(registry.temp_key('memory_map_ptr', prefix=prefix),
                                       I_PDPTR(pointee=mapping, writable=True)) as mapping_ptr:
                registry(mapping_ptr.copy(src=contents))

        return func

    @staticmethod
    def init_from_file(file, /):
        """Get the memory initialization function (contents read from file).
        """
        if not isinstance(file, ctx.FileContext):
            raise TypeError

        def func(registry, mapping, prefix, /):
            registry(file.read(dest=mapping.ptr))

        return func

    def __init__(self, /, length, stride, alignment, ext_alignment=None, init_fn=None):
        """Initialize a memory allocation description.
        """
        if not isinstance(length, (int, Object, Context, Context.Slot)):
            raise TypeError
        elif not isinstance(stride, (int, Object, Context, Context.Slot)):
            raise TypeError
        elif not isinstance(alignment, (int, Object, Context, Context.Slot)):
            raise TypeError
        elif not isinstance(ext_alignment, (NoneType, int, Object, Context, Context.Slot)):
            raise TypeError
        elif init_fn is not None and not callable(init_fn):
            raise ValueError

        self._length = length
        self._stride = stride
        self._alignment = alignment
        self._ext_alignment = ext_alignment
        self._init_fn = init_fn

    @property
    def length(self, /):
        """Get memory length.
        """
        return self._length

    @property
    def stride(self, /):
        """Get memory stride.
        """
        return self._stride

    @property
    def alignment(self, /):
        """Get memory alignment.
        """
        return self._alignment

    @property
    def ext_alignment(self, /):
        """Get memory extended alignment.
        """
        return self._ext_alignment

    @property
    def init_fn(self, /):
        """Get memory initialization function.
        """
        return self._init_fn


class MemoryAllocationProcedure(Procedure):
    """Memory allocation and initialization procedure.
    """
    Allocation = _MemoryAllocation

    def __init__(self, allocations, /, interface=None, alloc_data=None, map_data=None):
        """Initialize a procedure.
        """
        if not isinstance(allocations, dict):
            raise TypeError

        for key, alloc in allocations.items():
            if not isinstance(key, str):
                raise TypeError
            elif not isinstance(alloc, self.__class__.Allocation):
                raise TypeError

        if interface is None:
            if alloc_data is not None:
                raise ValueError("Heap memory interface does not accept allocation data")
            elif map_data is not None:
                raise ValueError("Heap memory interface does not accept mapping data")
        else:
            if not isinstance(interface, (Context, Context.Slot)):
                raise TypeError
            elif not isinstance(alloc_data, (NoneType, Object, Context, Context.Slot)):
                raise TypeError
            elif not isinstance(map_data, (NoneType, Object, Context, Context.Slot)):
                raise TypeError

        self._allocations = allocations.copy()
        self._interface = interface
        self._alloc_data = alloc_data
        self._map_data = map_data

        self._cleanup = ContextDeletionProcedure(keys=tuple(allocations.keys()))

    @property
    def allocations(self, /):
        """Get the dictionary of allocation descriptions.
        """
        return MappingProxyType(self._allocations)

    @property
    def interface(self, /):
        """Get the memory interface.
        """
        return self._interface

    @property
    def alloc_data(self, /):
        """Get the memory allocation data.
        """
        return self._alloc_data

    @property
    def map_data(self, /):
        """Get the memory mapping data.
        """
        return self._map_data

    @property
    def cleanup(self, /):
        """Get the cleanup procedure.
        """
        return self._cleanup

    def _impl(self, registry, /, prefix):
        """Implementation of the procedure.

        Returns dictionary of memory contexts.
        """
        if self.interface is not None:
            interface = self.interface
        else:
            interface = heap_memory_interface(registry.BUILTIN.executable)

        I_MEMORY = ctx.MemoryContext.interface_in(registry.BUILTIN.executable)
        I_MEMORY_MAPPING = ctx.MemoryMappingContext.interface_in(registry.BUILTIN.executable)

        # Create memory contexts
        memory_contexts = {}

        for key, alloc in self.allocations.items():
            params = {}
            if self.alloc_data is not None:
                params['alloc_data'] = self.alloc_data
            if alloc.ext_alignment is not None:
                params['ext_alignment'] = alloc.ext_alignment

            context = memory_contexts[key] = registry.new_context(
                    Registry.key(key, prefix=prefix),
                    I_MEMORY(interface=interface, length=alloc.length,
                             stride=alloc.stride, alignment=alloc.alignment, **params))

            # Initialize the memory
            if alloc.init_fn is not None:
                params = {}
                if self.map_data is not None:
                    params['map_data'] = self.map_data

                with registry.temp_context(registry.temp_key('memory_map', prefix=prefix),
                                           I_MEMORY_MAPPING(memory=context, **params)) as mapping:
                    alloc.init_fn(registry, mapping, prefix)

        return memory_contexts

### archi/env ###

class _EnvironmentVariable:
    """Description of an environment variable.
    """
    def __init__(self, name, /, default=None, parse_as=None):
        """Initialize a variable description.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not name or '=' in name:
            raise ValueError

        if parse_as is None:
            if not isinstance(default, (NoneType, str)):
                raise TypeError

            base = None
        else:
            if isinstance(parse_as, tuple):
                parse_as, base = parse_as
            else:
                base = None

            if not isinstance(parse_as, str):
                raise TypeError
            elif not parse_as or parse_as == 'base':
                raise ValueError

            if base is not None:
                if not isinstance(base, int):
                    raise TypeError
                elif base < 2 or base > 36:
                    raise ValueError

        self._name = name
        self._default = default
        self._parse_as = parse_as
        self._base = base

    @property
    def name(self, /):
        """Get the environment variable name.
        """
        return self._name

    @property
    def default(self, /):
        """Get the default value of the environment variable.
        """
        return self._default

    @property
    def parse_as(self, /):
        """Get the parsed type name of the environment variable.
        """
        return self._parse_as

    @property
    def base(self, /):
        """Get the base of parsed integer type of the environment variable.
        """
        return self._base


class EnvironmentVariablesProcedure(Procedure):
    """Environment variables value parsing procedure.
    """
    Variable = _EnvironmentVariable

    def __init__(self, variables, /):
        """Initialize a procedure.
        """
        if not isinstance(variables, dict):
            raise TypeError

        for key, var in variables.items():
            if not isinstance(key, str):
                raise TypeError
            elif not isinstance(var, self.__class__.Variable):
                raise TypeError

        self._variables = variables.copy()

        self._cleanup = ContextDeletionProcedure(keys=tuple(variables.keys()))

    @property
    def variables(self, /):
        """Get the dictionary of environment variable descriptions.
        """
        return self._variables

    @property
    def cleanup(self, /):
        """Get the cleanup procedure.
        """
        return self._cleanup

    def _impl(self, registry, /, prefix):
        """Implementation of the procedure.

        Returns dictionary of value contexts.
        """
        I_ENV_VARIABLE = ctx.EnvVariableContext.interface_in(registry.BUILTIN.executable)
        I_NUMBER_PARSER = ctx.NumberParserContext.interface_in(registry.BUILTIN.executable)

        # Create environment variable contexts
        envvar_contexts = {}

        for var in self.variables.values():
            default = str(var.default) if var.default is not None else None
            if default in envvar_contexts:
                continue

            params = {'default_value': default} if default is not None else {}

            envvar_contexts[default] = registry.new_context(
                    registry.temp_key('env_variable', prefix=prefix),
                    I_ENV_VARIABLE(**params))

        # Create value contexts
        value_contexts = {}

        for key, var in self.variables.items():
            default = str(var.default) if var.default is not None else None
            slot = getattr(envvar_contexts[default], var.name)

            if var.parse_as is None:
                value_contexts[key] = registry.new_context(
                        Registry.key(key, prefix=prefix), slot)

            else:
                params = {var.parse_as: slot}
                if var.base is not None:
                    params['base'] = base

                value_contexts[key] = registry.new_context(
                        Registry.key(key, prefix=prefix),
                        I_NUMBER_PARSER(**params))

        # Delete environment variable contexts
        for context in envvar_contexts.values():
            registry.delete(context)

        return value_contexts

