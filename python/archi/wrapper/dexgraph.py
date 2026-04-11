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
# @brief High-level wrapper for DEG node and node array contexts.

from contextlib import contextmanager
from types import MappingProxyType

from archi.ctypes import (
        ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION,
        ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION,
        )
from archi.context import Registry
from archi.builtin import DexgraphNodeContext, DexgraphNodeArrayContext


class DexgraphNode:
    """Directed execution graph node.
    """
    def __init__(self, name=None, sequence=(), transition=None):
        """Initialize a DEG node.
        """
        if not isinstance(name, (type(None), str)):
            raise TypeError
        elif not isinstance(sequence, tuple):
            raise TypeError

        for op in sequence:
            if not isinstance(op, (type(None), tuple)):
                raise TypeError
            elif op is not None and len(op) != 2:
                raise ValueError

        if not isinstance(transition, (type(None), tuple)):
            raise TypeError
        elif transition is not None and len(transition) != 2:
            raise ValueError

        self._name = name
        self._sequence = sequence
        self._transition = transition

        self._branches = {}

    def __getitem__(self, index):
        """Get node branch.
        """
        return self._branches[index]

    def __setitem__(self, index, node):
        """Set node branch.
        """
        if not isinstance(index, int):
            raise TypeError
        elif index < 0:
            raise ValueError
        elif not isinstance(node, DexgraphNode):
            raise TypeError

        self._branches[index] = node

    def __delitem__(self, index):
        """Delete node branch.
        """
        del self._branches[index]

    @property
    def name(self):
        return self._name

    @property
    def sequence(self):
        return self._sequence

    @property
    def transition(self):
        return self._transition

    @property
    def branches(self):
        return MappingProxyType(self._branches)

    @contextmanager
    def context(self, registry):
        """Context manager of a temporary DEG node context.
        """
        if not isinstance(registry, Registry):
            raise TypeError

        # Gather all nodes of the whole graph
        all_nodes = {self}

        nodes = set(self.branches.values())
        while nodes:
            all_nodes.update(nodes)

            nested_nodes = set()
            for node in nodes:
                nested_nodes.update(set(node.branches.values()))

            nodes = nested_nodes.difference(all_nodes)

            del nested_nodes
        del nodes

        # Prepare the branch tuples for each node
        node_branches = {}

        for node in all_nodes:
            if node.branches:
                branches = [None] * (max(node.branches.keys()) + 1)
            else:
                branches = []

            for index, branch in node.branches.items():
                branches[index] = branch

            node_branches[node] = tuple(branches)

            del branches

        del all_nodes

        # Create node array contexts for tuples of branches
        I_DEXGRAPH_NODE_ARRAY = DexgraphNodeArrayContext.interface(library=registry[Registry.KEY_EXECUTABLE])

        node_array_contexts = {}

        for branches in set(node_branches.values()):
            if branches:
                node_array_contexts[branches] = registry.new_context(
                        I_DEXGRAPH_NODE_ARRAY(num_nodes=len(branches)),
                        key=registry.temp_key(prefix='dexgraph_node_array', rnd_len=6))
            else:
                node_array_contexts[branches] = None

        del I_DEXGRAPH_NODE_ARRAY

        # Create node contexts, set operations and transitions
        I_DEXGRAPH_NODE = DexgraphNodeContext.interface(library=registry[Registry.KEY_EXECUTABLE])

        node_contexts = {}

        for node, branches in node_branches.items():
            # Create the node context
            params = {}

            if node.name is not None:
                params['name'] = node.name

            if node.transition is not None:
                if node.transition[0] is not None:
                    params['transition_func'] = node.transition[0]

                if node.transition[1] is not None:
                    params['transition_data'] = node.transition[1]

            context = registry.new_context(I_DEXGRAPH_NODE(sequence_length=len(node.sequence),
                                                           branches=node_array_contexts[branches],
                                                           **params),
                                           key=registry.temp_key(prefix='dexgraph_node', rnd_len=6))

            node_contexts[node] = context

            # Set node operations
            for index, operation in enumerate(node.sequence):
                if operation is not None:
                    if operation[0] is not None:
                        context.sequence.function[index] = operation[0]

                    if operation[1] is not None:
                        context.sequence.data[index] = operation[1]

        del I_DEXGRAPH_NODE

        # Fill node arrays
        for branches, context in node_array_contexts.items():
            for index, branch in enumerate(branches):
                if branch is not None:
                    context.node[index] = DexgraphNodeContext.weak_ref(node_contexts[branch])

        # Yield the entry node context
        try:
            yield node_contexts[self]
        finally:
            # Delete all contexts
            for context in node_array_contexts.values():
                registry.del_context(context)

            for context in node_contexts.values():
                registry.del_context(context)

    @staticmethod
    def operation_fn(slot):
        """Set operation function tag for a library slot.
        """
        return slot(function=True, tag=ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)

    @staticmethod
    def transition_fn(slot):
        """Set transition function tag for a library slot.
        """
        return slot(function=True, tag=ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION)

