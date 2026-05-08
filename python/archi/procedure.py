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
# @brief Construction of context registry operation sequences.

from types import SimpleNamespace

from .object import Object
from .context import Context
import archi.context as ctx
from .registry import Registry


class Procedure:
    """Higher level representation of a sequence of registry operations.
    """
    def __init__(self, registry, /, namespace=''):
        """Initialize a procedure.
        """
        if not isinstance(registry, Registry):
            raise TypeError
        elif not isinstance(namespace, str):
            raise TypeError

        self._registry = registry
        self._executable = registry.require_builtin(registry.__class__.KEY_EXECUTABLE)

        self._namespace = namespace

        self._contexts = {}

        self._active = False

    def __enter__(self, /):
        """Context manager enter function.
        """
        self.begin()
        return self

    def __exit__(self, /, exc_type, exc_value, traceback):
        """Context manager exit function.
        """
        self.end()

    def __getitem__(self, key, /):
        """Get a procedure context by key.
        """
        return self._contexts[key]

    def __contains__(self, key, /):
        """Check if a context with the specified key is provided by the procedure.
        """
        if not isinstance(key, str):
            raise TypeError

        return key in self._contexts

    def __len__(self, /):
        """Get number of contexts.
        """
        return len(self._contexts)

    def __iter__(self, /):
        """Return a context iterator.
        """
        return iter(self._contexts)

    def __reversed__(self, /):
        """Return a reversed context iterator.
        """
        return reversed(self._contexts)

    def key(self, key, /):
        """Get a context key prefixed by the procedure namespace.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not key:
            return self._namespace

        return f'{self._namespace}.{key}' if self._namespace else key

    def temp_key(self, prefix, /):
        """Get a temporary context key prefixed by the procedure namespace.
        """
        if not isinstance(prefix, str):
            raise TypeError
        elif not prefix:
            raise ValueError

        key = self.registry.operations.temp_key(prefix)
        return '~' + key[1:]

    @property
    def active(self, /):
        """Check if the procedure is active (began, but not ended).
        """
        return self._active

    @property
    def registry(self, /):
        """Obtain the context registry.
        """
        return self._registry

    @property
    def executable(self, /):
        """Get the library handle of the Archipelago executable.
        """
        return self._executable

    def begin(self, /):
        """Begin the procedure.
        """
        if self.active:
            raise RuntimeError("The procedure is already active -- nesting is not supported")
        elif not self._ready():
            raise RuntimeError("The procedure is not ready")

        self._active = True
        self._begin()

    def end(self, /):
        """End the procedure.
        """
        if not self.active:
            raise RuntimeError("The procedure is not active")

        self._end()
        self._active = False

    def reset(self, /):
        """Reset the procedure.
        """
        if self.active:
            raise RuntimeError("The procedure is active")

        self._reset()

    def _ready(self, /):
        """Check the readiness of the procedure object.

        This method is to be reimplemented in derived classes.
        """
        return True # always ready by default

    def _begin(self, /):
        """Implementation of the procedure beginning.

        This method is to be reimplemented in derived classes.
        """
        pass # do nothing by default

    def _end(self, /):
        """Implementation of the procedure end.

        This method is to be reimplemented in derived classes.
        """
        for context in self._contexts.values():
            self.registry.del_context(context)

        self._contexts.clear()

    def _reset(self, /):
        """Implementation of the procedure reset.

        This method is to be reimplemented in derived classes.
        """
        pass # do nothing by default

##############################################################################
# Built-in procedures
##############################################################################

### archi/exec ###

class DirectedExecutionGraph(Procedure):
    """Directed execution graph construction procedure.
    """
    def __init__(self, registry, /, namespace=''):
        """Initialize a procedure.
        """
        super().__init__(registry, namespace=namespace)

        self._nodes = {}

    def node(self, key, /, name=None, transition=None, *sequence):
        """Add a node to the graph.
        """
        if self.active:
            raise RuntimeError
        elif not isinstance(key, str):
            raise TypeError
        elif not key:
            raise ValueError
        elif key in self._nodes:
            raise KeyError(f"DEG node with key '{key}' exists already")
        elif not isinstance(name, (type(None), str)):
            raise TypeError

        if transition is not None:
            transition_func, transition_data = transition

            if not isinstance(transition_func, (type(None), Context, Context.Slot)):
                raise TypeError
            elif not isinstance(transition_data, (type(None), Object, Context, Context.Slot)):
                raise TypeError

            if not self.registry.owns(transition_func):
                raise ValueError
            elif not self.registry.owns(transition_data):
                raise ValueError

        else:
            transition_func = None
            transition_data = None

        sequence_func = []
        sequence_data = []
        for elt in sequence:
            seq_func, seq_data = elt

            if not isinstance(seq_func, (type(None), Context, Context.Slot)):
                raise TypeError
            elif not isinstance(seq_data, (type(None), Object, Context, Context.Slot)):
                raise TypeError

            if not self.registry.owns(seq_func):
                raise ValueError
            elif not self.registry.owns(seq_data):
                raise ValueError

            sequence_func.append(seq_func)
            sequence_data.append(seq_data)

        self._nodes[key] = SimpleNamespace(name=name,
                                           transition_func=transition_func,
                                           transition_data=transition_data,
                                           sequence_func=sequence_func,
                                           sequence_data=sequence_data,
                                           branches={})

        return self

    def node_branch(self, key, branch_idx, target_key, /):
        """Add a node branch link to the graph.
        """
        if self.active:
            raise RuntimeError
        elif not isinstance(key, str):
            raise TypeError
        elif not isinstance(target_key, str):
            raise TypeError
        elif not isinstance(branch_idx, int):
            raise TypeError
        elif branch_idx < 0:
            raise ValueError

        if key not in self._nodes:
            raise KeyError(f"No such node with key '{key}' in the graph")
        elif target_key not in self._nodes:
            raise KeyError(f"No such node with key '{target_key}' in the graph")

        if branch_idx in self._nodes[key].branches:
            raise IndexError(f"Branch #{branch_idx} already exists in node '{key}'")

        self._nodes[key].branches[branch_idx] = target_key

        return self

    def _begin(self, /):
        """Implementation of the procedure beginning.
        """
        # Create node branch tuples
        node_branches = {}

        for key, node in self._nodes.items():
            branches = [None] * (max(node.branches.keys()) + 1) if node.branches else []

            for index, target_key in node.branches.items():
                branches[index] = target_key

            node_branches[key] = tuple(branches)

        # Create node array contexts
        I_DEXGRAPH_NODE_ARRAY = ctx.DexgraphNodeArrayContext.interface_in(self.executable)

        node_array_contexts = {(): None}

        for branches in node_branches.values():
            if not branches:
                continue

            node_array_contexts[branches] = self.registry.new_context(
                    I_DEXGRAPH_NODE_ARRAY(num_nodes=len(branches)),
                    key=self.temp_key('node_array'))

        # Create node contexts
        I_DEXGRAPH_NODE = ctx.DexgraphNodeContext.interface_in(self.executable)

        node_contexts = {}

        for key, node in self._nodes.items():
            # Create a node context
            params = {}

            if node.name is not None:
                params['name'] = node.name

            if node.transition_func is not None:
                params['transition_func'] = node.transition_func

            if node.transition_data is not None:
                params['transition_data'] = node.transition_data

            context = node_contexts[key] = self.registry.new_context(
                    I_DEXGRAPH_NODE(sequence_length=len(node.sequence_func),
                                    branches=node_array_contexts[node_branches[key]],
                                    **params),
                    key=self.key(key))

            # Set node sequence operations
            for index, (seq_func, seq_data) in enumerate(zip(node.sequence_func, node.sequence_data)):
                if seq_func is not None:
                    context.sequence.function[index] = seq_func

                if seq_data is not None:
                    context.sequence.data[index] = seq_data

            del context

        # Fill node branch arrays
        for branches, context in node_array_contexts.items():
            for index, target_key in enumerate(branches):
                if target_key is not None:
                    context.node[index] = Context.weak_ref(node_contexts[target_key])

        # Delete node array contexts
        for context in node_array_contexts.values():
            self.registry.del_context(context)

        # Set the procedure contexts
        self._contexts = node_contexts

    def _reset(self, /):
        """Implementation of the procedure reset.
        """
        self._nodes.clear()

### archi/memory ###

class MemoryAllocations(Procedure):
    """Memory allocation and initialization procedure.
    """
    def __init__(self, registry, /, namespace='',
                 interface=None, alloc_data=None, map_data=None):
        """Initialize a procedure.
        """
        super().__init__(registry, namespace=namespace)

        if interface is None:
            if alloc_data is not None:
                raise ValueError
            elif map_data is not None:
                raise ValueError

            interface = ctx.MemoryInterfaceSymbol.slot('heap', self.executable)
        else:
            if not self.registry.owns(interface):
                raise ValueError
            elif not self.registry.owns(alloc_data):
                raise ValueError
            elif not self.registry.owns(map_data):
                raise ValueError

        self._interface = interface
        self._alloc_data = alloc_data
        self._map_data = map_data

        self._allocations = {}

    @property
    def interface(self, /):
        """Obtain the memory interface.
        """
        return self._interface

    @property
    def alloc_data(self, /):
        """Obtain the memory allocation data.
        """
        return self._alloc_data

    @property
    def map_data(self, /):
        """Obtain the memory mapping data.
        """
        return self._map_data

    def memory(self, key, /, length, stride, alignment, ext_alignment=None):
        """Add a memory allocation.
        """
        if self.active:
            raise RuntimeError
        elif not isinstance(key, str):
            raise TypeError
        elif not key:
            raise ValueError
        elif key in self._allocations:
            raise KeyError(f"Memory allocation '{key}' is added already")

        if not self.registry.owns(length):
            raise ValueError
        elif not self.registry.owns(stride):
            raise ValueError
        elif not self.registry.owns(alignment):
            raise ValueError
        elif not self.registry.owns(ext_alignment):
            raise ValueError

        self._allocations[key] = SimpleNamespace(length=length,
                                                 stride=stride,
                                                 alignment=alignment,
                                                 ext_alignment=ext_alignment,
                                                 init_fn=None)

        return self

    def init(self, key, contents, /):
        """Initialize a memory allocation by copying contents from source memory.
        """
        if self.active:
            raise RuntimeError
        elif not isinstance(key, str):
            raise TypeError
        elif not isinstance(contents, (Object, Context, Context.Slot)):
            raise TypeError
        elif not self.registry.owns(contents):
            raise ValueError

        if key not in self._allocations:
            raise KeyError(f"No such memory allocation with key '{key}'")
        elif self._allocations[key].init_fn is not None:
            raise RuntimeError(f"Memory allocation '{key}' is already initialized")

        def init_fn(mapping, /):
            I_PDPTR = ctx.PrimitiveDataPointerContext.interface_in(self.executable)

            with self.registry.temp_context(I_PDPTR(pointee=mapping, writable=True),
                                            key=self.temp_key('memory_map_ptr')) as mapping_ptr:
                self.registry(mapping_ptr.copy(src=contents))

        self._allocations[key].init_fn = init_fn

        return self

    def init_from_file(self, key, file, /):
        """Initialize a memory allocation by reading contents from source file.
        """
        if self.active:
            raise RuntimeError
        elif not isinstance(key, str):
            raise TypeError
        elif not isinstance(file, ctx.FileContext):
            raise TypeError
        elif not self.registry.owns(file):
            raise ValueError

        if key not in self._allocations:
            raise KeyError(f"No such memory allocation with key '{key}'")
        elif self._allocations[key].init_fn is not None:
            raise RuntimeError(f"Memory allocation '{key}' is already initialized")

        def init_fn(mapping, /):
            self.registry(file.read(dest=mapping.ptr))

        self._allocations[key].init_fn = init_fn

        return self

    def _begin(self, /):
        """Implementation of the procedure beginning.
        """
        # Create memory contexts
        I_MEMORY = ctx.MemoryContext.interface_in(self.executable)
        I_MEMORY_MAPPING = ctx.MemoryMappingContext.interface_in(self.executable)

        memory_contexts = {}

        for key, memory in self._allocations.items():
            params = {}
            if self.alloc_data is not None:
                params['alloc_data'] = self.alloc_data
            if memory.ext_alignment is not None:
                params['ext_alignment'] = memory.ext_alignment

            context = memory_contexts[key] = self.registry.new_context(
                    I_MEMORY(interface=self.interface,
                             length=memory.length, stride=memory.stride,
                             alignment=memory.alignment, **params),
                    key=self.key(key))

            # Initialize the memory
            if memory.init_fn is not None:
                params = {}
                if self.map_data is not None:
                    params['map_data'] = self.map_data

                with self.registry.temp_context(I_MEMORY_MAPPING(memory=context, **params),
                                                key=self.temp_key('memory_map')) as mapping:
                    memory.init_fn(mapping)

        # Set the procedure contexts
        self._contexts = memory_contexts

    def _reset(self, /):
        """Implementation of the procedure reset.
        """
        self._allocations.clear()

### archi/env ###

class EnvironmentVariables(Procedure):
    """Environment variable extraction and conversion procedure.
    """
    def __init__(self, registry, /, namespace=''):
        """Initialize a procedure.
        """
        super().__init__(registry, namespace=namespace)

        self._variables = {}

    def var(self, name, /, default=None, parse=None, base=None):
        """Add an environment variable.
        """
        if self.active:
            raise RuntimeError
        elif not isinstance(name, str):
            raise TypeError
        elif not name:
            raise ValueError
        elif name in self._variables:
            raise KeyError(f"Variable '{name}' is added already")

        if parse is None:
            if not isinstance(default, (type(None), str)):
                raise TypeError
            elif base is not None:
                raise ValueError

        else:
            if not isinstance(default, (type(None), int, float)):
                raise TypeError
            elif not isinstance(parse, str):
                raise TypeError
            elif not parse or parse == 'base':
                raise ValueError

            if base is not None:
                if not isinstance(base, int):
                    raise TypeError
                elif base < 2 or base > 36:
                    raise ValueError

        self._variables[name] = SimpleNamespace(default=default, parse=parse, base=base)

        return self

    def _begin(self, /):
        """Implementation of the procedure beginning.
        """
        # Collect default values into a set
        default_values = {(str(var.default) if var.default is not None else None)
                          for var in self._variables.values()}

        # Create environment variable contexts
        I_ENV_VARIABLE = ctx.EnvVariableContext.interface_in(self.executable)

        envvar_contexts = {}
        for default in default_values:
            params = {'default_value': default} if default is not None else {}

            envvar_contexts[default] = self.registry.new_context(
                    I_ENV_VARIABLE(**params),
                    key=self.temp_key('env_var'))

        # Create the dictionary of contexts
        I_NUMBER_PARSER = ctx.NumberParserContext.interface_in(self.executable)

        contexts = {}

        for name, var in self._variables.items():
            if var.parse is None:
                contexts[name] = self.registry.new_context(
                        getattr(envvar_contexts[var.default], name),
                        key=self.key(name))

            else:
                params = {var.parse: getattr(envvar_contexts[
                    str(var.default) if var.default is not None else None], name)}
                if var.base is not None:
                    params['base'] = base

                contexts[name] = self.registry.new_context(
                        I_NUMBER_PARSER(**params), key=self.key(name))

        # Delete environment variable contexts
        for context in envvar_contexts.values():
            self.registry.del_context(context)

        # Set the procedure contexts
        self._contexts = contexts

    def _reset(self, /):
        """Implementation of the procedure reset.
        """
        self._variables.clear()

