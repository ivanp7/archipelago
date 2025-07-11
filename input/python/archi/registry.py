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
        """Create a context parameter list representation instance.
        """
        if _ is not None and not isinstance(_, Context):
            raise TypeError

        self._dparams = _
        self._sparams = params

    def dynamic_list(self) -> "Context":
        """Obtain the dynamic parameter list.
        """
        return self._dparams

    def static_list(self) -> "dict":
        """Obtain the static parameter list.
        """
        return self._sparams


class ContextSpec:
    """Representation of a context interface bundled with context initialization parameters.
    """
    def __init__(self, interface: "ContextInterface", params: "Parameters"):
        """Create a context specification instance.
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
        """Create a context interface instance.
        """
        if not isinstance(source, Context) and not isinstance(source, Context._Slot):
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
    class _Slot:
        """Representation of a context slot.
        """
        def __init__(self, context: "Context", name: "str" = '', indices: "list[int]" = []):
            """Create a context slot representation instance.
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

        def __getattr__(self, name: "str") -> "Context._Slot":
            """Obtain a context slot object.
            """
            if self._indices:
                raise RuntimeError("Indexed slot name is final and cannot be extended using dot (.)")

            return Context._Slot(self._context, f'{self._name}.{name}')

        def __getitem__(self, index: "int") -> "Context._Slot":
            """Obtain a context slot object.
            """
            if not isinstance(index, int):
                raise TypeError

            return Context._Slot(self._context, self._name, self._indices + [index])

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

        def __call__(self, _: "Context" = None, /, **params) -> "Context._Action":
            """Perform an action.
            """
            if _ is not None and not isinstance(_, Context):
                raise TypeError

            self._context._act(self._name, self._indices, dparams=_, sparams=params)
            return Context._Action(self._context, self._name, self._indices)

    class _Action:
        """Representation of a context action.
        """
        def __init__(self, context: "Context", name: "str", indices: "list[int]" = []):
            """Create a context action representation instance.
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
        """Create a context representation instance.
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

    def __getattr__(self, name: "str") -> "Context._Slot":
        """Obtain a context slot object.
        """
        return Context._Slot(self, name)

    def __getitem__(self, index: "int") -> "Context._Slot":
        """Obtain a context slot object.
        """
        if not isinstance(index, int):
            raise TypeError

        return Context._Slot(self, indices=[index])

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

    def __call__(self, _: "Context" = None, /, **params) -> "Context._Action":
        """Perform an action.
        """
        if _ is not None and not isinstance(_, Context):
            raise TypeError

        self._act('', [], dparams=_, sparams=params)
        return Context._Action(self, '', [])

    def _set(self, slot_name: "str", slot_indices: "list[int]", value):
        """Append a set() instruction to the list.
        """
        if not isinstance(slot_name, str):
            raise TypeError
        elif not isinstance(slot_indices, list) \
                or not all(isinstance(index, int) for index in slot_indices):
            raise TypeError

        if value is None or isinstance(value, CValue):
            self._registry._instruct(
                    InstructionType.SET_TO_VALUE,
                    key=self._key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    value=value)

        elif isinstance(value, Context):
            self._registry._instruct(
                    InstructionType.SET_TO_CONTEXT_DATA,
                    key=self._key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=value._key)

        elif isinstance(value, Context._Slot) or isinstance(value, Context._Action):
            self._registry._instruct(
                    InstructionType.SET_TO_CONTEXT_SLOT,
                    key=self._key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=value._context._key,
                    source_slot_name=value._name,
                    source_slot_indices=value._indices)

        else:
            raise TypeError("Cannot set slot to an unsupported value type")

    def _act(self, action_name: "str", action_indices: "list[int]",
             dparams: "Context", sparams: "dict"):
        """Append an act() instruction to the list.
        """
        if not isinstance(action_name, str):
            raise TypeError
        elif not isinstance(action_indices, list) \
                or not all(isinstance(index, int) for index in action_indices):
            raise TypeError
        elif dparams is not None and not isinstance(dparams, Context):
            raise TypeError
        elif not isinstance(sparams, dict) \
                or not all(isinstance(key, str) for key in sparams.keys()):
            raise TypeError

        self._registry._instruct(
                InstructionType.ACT,
                key=self._key,
                action_name=action_name,
                action_indices=action_indices,
                dparams_key=dparams._key if dparams is not None else None,
                sparams=sparams)

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
            """Create an registry instruction instance.
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
        if not isinstance(key, str):
            raise TypeError

        return Context(self, key)

    def __setitem__(self, key: "str", entity):
        """Initialize a context and insert it to the registry.
        """
        if not isinstance(key, str):
            raise TypeError

        if isinstance(entity, Parameters):
            dparams_key = entity.dynamic_list()._key \
                    if entity.dynamic_list() is not None else None

            self._instruct(
                    InstructionType.INIT_FROM_CONTEXT,
                    key=key,
                    interface_source_key=None,
                    dparams_key=dparams_key,
                    sparams=entity.static_list())

        elif isinstance(entity, ContextSpec):
            source = entity.interface().source()

            if isinstance(source, Context):
                dparams_key = entity.parameters().dynamic_list()._key \
                        if entity.parameters().dynamic_list() is not None else None

                self._instruct(
                        InstructionType.INIT_FROM_CONTEXT,
                        key=key,
                        interface_source_key=source._key,
                        dparams_key=dparams_key,
                        sparams=entity.parameters().static_list())

            elif isinstance(source, Context._Slot):
                dparams_key = entity.parameters().dynamic_list()._key \
                        if entity.parameters().dynamic_list() is not None else None

                self._instruct(
                        InstructionType.INIT_FROM_SLOT,
                        key=key,
                        interface_source_key=source._context._key,
                        interface_source_slot_name=source._name,
                        interface_source_slot_indices=source._indices,
                        dparams_key=dparams_key,
                        sparams=entity.parameters().static_list())

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

        elif isinstance(entity, Context._Slot) or isinstance(entity, Context._Action):
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
                if element is None or isinstance(element, CValue):
                    self._instruct(
                            InstructionType.SET_TO_VALUE,
                            key=key,
                            slot_name='',
                            slot_indices=[index],
                            value=element)

                elif isinstance(element, Context):
                    self._instruct(
                            InstructionType.SET_TO_CONTEXT_DATA,
                            key=key,
                            slot_name='',
                            slot_indices=[index],
                            source_key=element._key)

                elif isinstance(element, Context._Slot) or isinstance(element, Context._Action):
                    self._instruct(
                            InstructionType.SET_TO_CONTEXT_SLOT,
                            key=key,
                            slot_name='',
                            slot_indices=[index],
                            source_key=element._context._key,
                            source_slot_name=element._name,
                            source_slot_indices=element._indices)

                else:
                    raise TypeError("Cannot set array element to an unsupported value type")

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

    def reset(self):
        """Reset the list of instructions.
        """
        self._instructions = []

    def _instruct(self, type: "InstructionType", /, **fields):
        """Append an instruction to the list.
        """
        self._instructions.append(Registry._Instruction(type, **fields))

