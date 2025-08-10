 #############################################################################
 # Copyright (C) 2023-2025 by Ivan Podmazov                                  #
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
# @brief Representation of Archipelago executable registry.

import enum
from contextlib import contextmanager

from .memory import CValue


class InstructionType(enum.Enum):
    """Enumeration of supported instruction types.
    """
    NOOP = 0
    INIT_FROM_CONTEXT = enum.auto()
    INIT_FROM_SLOT = enum.auto()
    INIT_POINTER = enum.auto()
    INIT_DATA_ARRAY = enum.auto()
    INIT_FUNC_ARRAY = enum.auto()
    COPY = enum.auto()
    DELETE = enum.auto()
    SET_TO_VALUE = enum.auto()
    SET_TO_CONTEXT_DATA = enum.auto()
    SET_TO_CONTEXT_SLOT = enum.auto()
    ACT = enum.auto()

###############################################################################

class Parameters:
    """Representation of a parameter list.
    """
    def __init__(self, _: "Context" = None, /, **params):
        """Initialize a context parameter list representation instance.
        """
        import random
        import string

        if _ is not None and not isinstance(_, Context):
            raise TypeError

        self._dparams_orig = _
        self._dparams_temp_key = '.params_' + ''.join(random.choice(string.ascii_letters + string.digits) \
                for char in range(16))

        self._sparams_dynamic = {}
        self._sparams_static = {}

        for key, value in params.items():
            if isinstance(value, Context) \
                    or isinstance(value, Context.Slot) or isinstance(value, Context.Action):
                self._sparams_dynamic[key] = value
            else:
                self._sparams_static[key] = value

    def dynamic_list_original(self) -> "Context":
        """Obtain the original dynamic parameter list.
        """
        return self._dparams_orig

    def dynamic_list_original_key(self) -> "Context":
        """Obtain key of the original dynamic parameter list.
        """
        return self._dparams_orig._key if self._dparams_orig is not None else None

    def dynamic_list_temporary_key(self) -> "str":
        """Obtain key of a temporary dynamic parameter list.
        """
        return self._dparams_temp_key

    def static_list_dynamic_values(self) -> "dict":
        """Obtain the static parameter list of dynamic values.
        """
        return self._sparams_dynamic

    def static_list_static_values(self) -> "dict":
        """Obtain the static parameter list of static values.
        """
        return self._sparams_static


class ContextSpec:
    """Representation of a context interface bundled with context initialization parameters.
    """
    def __init__(self, interface: "ContextInterface", params: "Parameters"):
        """Initialize a context specification instance.
        """
        if not isinstance(interface, ContextInterface):
            raise TypeError
        elif not isinstance(params, Parameters):
            raise TypeError

        self._interface = interface
        self._params = params

    def interface(self) -> "ContextInterface":
        """Obtain the context interface from the specification.
        """
        return self._interface

    def parameters(self) -> "Parameters":
        """Obtain the context parameter list from the specification.
        """
        return self._params


class ContextInterface:
    """Representation of a context interface.
    """
    def __init__(self, source):
        """Initialize a context interface instance.
        """
        if not isinstance(source, Context) \
                and not isinstance(source, Context.Slot) \
                and not isinstance(source, Context.Action):
            raise TypeError

        self._source = source

    def __call__(self, _: "Context" = None, /, **params) -> "ContextSpec":
        """Create a context specification instance.
        """
        return ContextSpec(self, Parameters(_, **params))

    def source(self):
        """Obtain the context interface source.
        """
        return self._source


class Context:
    """Representation of a context.
    """
    class Slot:
        """Representation of a context slot.
        """
        def __init__(self, context: "Context", name: "str" = '', indices: "list[int]" = []):
            """Initialize a context slot representation instance.
            """
            if not isinstance(context, Context):
                raise TypeError
            elif not isinstance(name, str):
                raise TypeError
            elif not isinstance(indices, list) \
                    or not all(isinstance(index, int) for index in indices):
                raise TypeError

            object.__setattr__(self, '_context', context)
            object.__setattr__(self, '_name', name)
            object.__setattr__(self, '_indices', indices)

        def __getattr__(self, name: "str") -> "Context.Slot":
            """Obtain a context slot object.
            """
            if self._indices:
                raise RuntimeError("Indexed slot name is final and cannot be extended using dot (.)")

            return Context.Slot(self._context, f'{self._name}.{name}')

        def __getitem__(self, index: "int") -> "Context.Slot":
            """Obtain a context slot object.
            """
            if not isinstance(index, int):
                raise TypeError

            return Context.Slot(self._context, self._name, self._indices + [index])

        def __setattr__(self, name: "str", value):
            """Perform a slot setting operation.
            """
            self._context._set(f'{self._name}.{name}', self._indices, value)

        def __setitem__(self, index: "int", value):
            """Perform a slot setting operation.
            """
            if not isinstance(index, int):
                raise TypeError

            self._context._set(self._name, self._indices + [index], value)

        def __call__(self, _: "Context" = None, /, **params) -> "Context.Action":
            """Perform an action.
            """
            if _ is not None and not isinstance(_, Context):
                raise TypeError

            self._context._act(self._name, self._indices, Parameters(_, **params))
            return Context.Action(self._context, self._name, self._indices)

    class Action:
        """Representation of a context action.
        """
        def __init__(self, context: "Context", name: "str", indices: "list[int]" = []):
            """Initialize a context action representation instance.
            """
            if not isinstance(context, Context):
                raise TypeError
            elif not isinstance(name, str):
                raise TypeError
            elif not isinstance(indices, list) \
                    or not all(isinstance(index, int) for index in indices):
                raise TypeError

            self._context = context
            self._name = name
            self._indices = indices

    def __init__(self, registry: "Registry", key: "str"):
        """Initialize a context representation instance.
        """
        if not isinstance(registry, Registry):
            raise TypeError
        elif not isinstance(key, str):
            raise TypeError

        object.__setattr__(self, '_registry', registry)
        object.__setattr__(self, '_key', key)

    def __enter__(self):
        """Context manager entry.
        """
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """Context manager exit.
        """
        if exc_type is None:
            self._registry._instruct(
                    InstructionType.DELETE,
                    key=self._key)

    def __getattr__(self, name: "str") -> "Context.Slot":
        """Obtain a context slot object.
        """
        return Context.Slot(self, name)

    def __getitem__(self, index: "int") -> "Context.Slot":
        """Obtain a context slot object.
        """
        if not isinstance(index, int):
            raise TypeError

        return Context.Slot(self, indices=[index])

    def __setattr__(self, name: "str", value):
        """Perform a slot setting operation.
        """
        self._set(name, [], value)

    def __setitem__(self, index: "int", value):
        """Perform a slot setting operation.
        """
        if not isinstance(index, int):
            raise TypeError

        self._set('', [index], value)

    def __call__(self, _: "Context" = None, /, **params) -> "Context.Action":
        """Perform an action.
        """
        if _ is not None and not isinstance(_, Context):
            raise TypeError

        self._act('', [], Parameters(_, **params))
        return Context.Action(self, '', [])

    def _set(self, slot_name: "str", slot_indices: "list[int]", value):
        """Append a set() instruction to the list.
        """
        self._registry._set_slot(self._key, slot_name, slot_indices, value)

    def _act(self, action_name: "str", action_indices: "list[int]", params: "Parameters"):
        """Append an act() instruction to the list.
        """
        self._registry._with_params(params, lambda dparams_key, sparams:
                                    self._registry._instruct(
                                        InstructionType.ACT,
                                        key=self._key,
                                        action_name=action_name,
                                        action_indices=action_indices,
                                        dparams_key=dparams_key,
                                        sparams=sparams))

    @staticmethod
    def key_of(context: "Context") -> "str":
        """Obtain key of a context.
        """
        if not isinstance(context, Context):
            raise TypeError

        return object.__getattr__(context, '_key')

    @staticmethod
    def registry_of(context: "Context") -> "str":
        """Obtain owning registry of a context.
        """
        if not isinstance(context, Context):
            raise TypeError

        return object.__getattr__(context, '_registry')

###############################################################################

class Registry:
    """Representation of an Archipelago executable registry.
    """
    class _Instruction:
        """Representation of an registry instruction.
        """
        def __init__(self, _: "InstructionType", /, **fields):
            """Initialize an registry instruction instance.
            """
            if not isinstance(_, InstructionType):
                raise TypeError

            self._type = _.value
            self._fields = fields

        def __getitem__(self, key):
            """Get instruction field value.
            """
            return self._fields[key]

        def type(self):
            """Get instruction type.
            """
            return self._type

        def fields(self):
            """Get dictionary of instruction fields.
            """
            return self._fields

    # keys of built-in contexts
    KEY_REGISTRY    = 'archi.registry'
    KEY_EXECUTABLE  = 'archi.executable'
    KEY_INPUT_FILE  = 'archi.input_file'
    KEY_SIGNAL      = 'archi.signal'

    def __init__(self):
        """Initialize a registry.
        """
        self._instructions = []

    def __getitem__(self, key: "str") -> "Context":
        """Obtain a context from the registry.
        """
        if key is None:
            return None
        elif not isinstance(key, str):
            raise TypeError

        return Context(self, key)

    def __setitem__(self, key: "str", entity):
        """Create a context and insert it to the registry.
        """
        if not isinstance(key, str):
            raise TypeError

        if isinstance(entity, Parameters):
            self._with_params(entity, lambda dparams_key, sparams:
                              self._instruct(
                                  InstructionType.INIT_FROM_CONTEXT,
                                  key=key,
                                  interface_source_key=None,
                                  dparams_key=dparams_key,
                                  sparams=sparams))

        elif isinstance(entity, ContextSpec):
            source = entity.interface().source()

            def func(dparams_key, sparams):
                if isinstance(source, Context):
                    self._instruct(
                            InstructionType.INIT_FROM_CONTEXT,
                            key=key,
                            interface_source_key=source._key,
                            dparams_key=dparams_key,
                            sparams=sparams)

                elif isinstance(source, Context.Slot) or isinstance(source, Context.Action):
                    self._instruct(
                            InstructionType.INIT_FROM_SLOT,
                            key=key,
                            interface_source_key=source._context._key,
                            interface_source_slot_name=source._name,
                            interface_source_slot_indices=source._indices,
                            dparams_key=dparams_key,
                            sparams=sparams)

            self._with_params(entity.parameters(), func)

        elif entity is None or isinstance(entity, CValue):
            self._instruct(
                    InstructionType.INIT_POINTER,
                    key=key,
                    value=entity)

        elif isinstance(entity, Context):
            self._instruct(
                    InstructionType.COPY,
                    key=key,
                    original_key=entity._key)

        elif isinstance(entity, Context.Slot) or isinstance(entity, Context.Action):
            self._instruct(
                    InstructionType.INIT_POINTER,
                    key=key,
                    value=None)

            self._instruct(
                    InstructionType.SET_TO_CONTEXT_SLOT,
                    key=key,
                    slot_name='value',
                    slot_indices=[],
                    source_key=entity._context._key,
                    source_slot_name=entity._name,
                    source_slot_indices=entity._indices)

        elif isinstance(entity, list):
            self._instruct(
                    InstructionType.INIT_DATA_ARRAY,
                    key=key,
                    num_elements=len(entity),
                    flags=None)

            for index, element in enumerate(entity):
                self._set_slot(key, '', [index], element)

        else:
            raise TypeError("Cannot add an entity of unsupported type to a registry")

    def __delitem__(self, key: "str"):
        """Finalize a context and remove it from the registry.
        """
        if not isinstance(key, str):
            raise TypeError

        self._instruct(
                InstructionType.DELETE,
                key=key)

    @contextmanager
    def context(self, value, /, key: "str" = None):
        """Context manager for creating a temporary context.
        """
        if key is None:
            key = Registry.temp_key()

        self[key] = value
        try:
            yield self[key]
        finally:
            del self[key]

    def noop(self):
        """Emit a NOOP instruction.
        """
        self._instruct(
                InstructionType.NOOP)

    def reset(self):
        """Reset the list of instructions.
        """
        self._instructions = []

    def _instruct(self, type: "InstructionType", /, **fields):
        """Append an instruction to the list.
        """
        self._instructions.append(Registry._Instruction(type, **fields))

    def _set_slot(self, key: "str", slot_name: "str", slot_indices: "list[int]", value):
        """Append a setter instruction corresponding to the value type to the list.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not isinstance(slot_name, str):
            raise TypeError
        elif not isinstance(slot_indices, list) \
                or not all(isinstance(index, int) for index in slot_indices):
            raise TypeError

        if value is None or isinstance(value, CValue):
            self._instruct(
                    InstructionType.SET_TO_VALUE,
                    key=key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    value=value)

        elif isinstance(value, Context):
            self._instruct(
                    InstructionType.SET_TO_CONTEXT_DATA,
                    key=key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=value._key)

        elif isinstance(value, Context.Slot) or isinstance(value, Context.Action):
            self._instruct(
                    InstructionType.SET_TO_CONTEXT_SLOT,
                    key=key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=value._context._key,
                    source_slot_name=value._name,
                    source_slot_indices=value._indices)

        else:
            raise TypeError("Cannot set slot to an unsupported value type")

    def _with_params(self, params: "Parameters", func):
        """Prepare a temporary parameter list if needed and invoke a custom function before deleting the list.
        """
        if not params.static_list_dynamic_values():
            func(params.dynamic_list_original_key(), params.static_list_static_values())
            return

        self._instruct(
                InstructionType.INIT_FROM_CONTEXT,
                key=params.dynamic_list_temporary_key(),
                interface_source_key=None,
                dparams_key=params.dynamic_list_original_key(),
                sparams=params.static_list_static_values())

        for key, value in reversed(params.static_list_dynamic_values().items()):
            self._set_slot(params.dynamic_list_temporary_key(), key, [], value)

        func(params.dynamic_list_temporary_key(), {})

        self._instruct(
                InstructionType.DELETE,
                key=params.dynamic_list_temporary_key())

    @staticmethod
    def temp_key(comment: "str" = None) -> "str":
        """Generate a random temporary context key with an optional comment in it.
        """
        import random
        import string

        return '.context' + (f'({comment})_' if comment is not None else '_') \
                + ''.join(random.choice(string.ascii_letters + string.digits) \
                for char in range(16))

