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
# @brief Representation of the essential Archipelago application entities.

import enum
from contextlib import contextmanager

from .memory import CValue


class Registry:
    """Representation of an Archipelago executable registry.
    """
    class _ContextInterfaceOrigin:
        """Representation of an origin (context, slot) of a context interface.
        """
        def __init__(self, origin, cls: 'type' = Context):
            """Initialize a context interface instance.
            """
            self._origin = origin
            self._cls = cls

        def __call__(self, _: 'Context' = None, /, **params) -> 'Registry._ContextSpec':
            """Create a context specification instance.
            """
            return Registry._ContextSpec(self, self._cls.InitParametersClass(_, **params))

    class _ContextSpec:
        """Representation of a created context specification.
        """
        def __init__(self, interface_origin: 'Registry._ContextInterfaceOrigin', params: 'Parameters'):
            """Initialize a context specification instance.
            """
            self._origin = interface_origin._origin
            self._cls = interface_origin._cls
            self._params = params

        def is_a(self, cls: 'type', /) -> 'Registry._ContextSpec':
            """Refine a created context class.
            """
            if not isinstance(cls, type):
                raise TypeError
            elif not issubclass(cls, self._cls):
                raise TypeError(f"Cannot refine {self._cls} to {cls} -- isn't a subclass")

            self._cls = cls
            return self

    class _Instruction:
        """Representation of an registry instruction.

        This class is a private implementation detail.
        """
        class Type(enum.Enum):
            """Enumeration of supported instruction types.
            """
            NOOP = 0
            INIT_FROM_CONTEXT = enum.auto()
            INIT_FROM_SLOT = enum.auto()
            INIT_POINTER = enum.auto()
            INIT_ARRAY = enum.auto()
            COPY = enum.auto()
            DELETE = enum.auto()
            SET_TO_VALUE = enum.auto()
            SET_TO_CONTEXT_DATA = enum.auto()
            SET_TO_CONTEXT_SLOT = enum.auto()
            ACT = enum.auto()

        def __init__(self, _: 'Registry._Instruction.Type', /, **fields):
            """Initialize an registry instruction instance.
            """
            if not isinstance(_, Registry._Instruction.Type):
                raise TypeError

            self._type = _.value
            self._fields = fields

        def __getitem__(self, key: 'str'):
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

    def __init__(self):
        """Initialize a registry.
        """
        self.reset()

    def __getitem__(self, key: 'str') -> 'Context':
        """Obtain a context from the registry.
        """
        if key is None:
            return None
        elif not isinstance(key, str):
            raise TypeError

        return self._contexts[key]

    def __setitem__(self, key: 'str', entity):
        """Create a context and insert it to the registry.
        """
        if not isinstance(key, str):
            raise TypeError
        elif key in self._contexts:
            raise KeyError(f"Context '{key}' is already in the registry")

        context = self._create_context(entity)
        context._registry = self
        context._key = key

        self._contexts[key] = context

    def __delitem__(self, key: 'str'):
        """Finalize a context and remove it from the registry.
        """
        if key is None:
            return
        elif not isinstance(key, str):
            raise TypeError

        del self._contexts[key]
        self._delete_context(key)

    def require_context(self, key: 'str', cls: 'type' = Context) -> 'Context':
        """Require a context with the specified key to exist in the registry.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not isinstance(cls, type) or not issubclass(cls, Context):
            raise TypeError

        if key in self._contexts:
            raise KeyError(f"Context '{key}' is already in the registry")

        context = cls()
        context._registry = self
        context._key = key

        self._contexts[key] = context
        return context

    @contextmanager
    def del_context(self, key: 'str'):
        """Context manager for automatically deleting an existing context.
        """
        context = self[key]

        try:
            yield context
        finally:
            del self[key]

    @contextmanager
    def temp_context(self, value, /, key: 'str' = None):
        """Context manager for creating a temporary context.
        """
        if key is None:
            key = Registry.temp_key()

        self[key] = value

        with self.del_context(key) as context:
            yield context

    def contexts(self, cls: 'type' = Context) -> 'dict[str, Context]':
        """Obtain the dictionary of known contexts of the specified type.
        """
        return {key: value for key, value in self._contexts.items() if isinstance(value, cls)}

    def noop(self):
        """Emit a NOOP instruction.
        """
        self._instruct(Registry._Instruction.Type.NOOP)

    def reset(self):
        """Reset the list of instructions.
        """
        self._contexts = {}
        self._instructions = []

    def _instruct(self, type: 'Registry._Instruction.Type', /, **fields):
        """Append an instruction to the list.
        """
        self._instructions.append(Registry._Instruction(type, **fields))

    def _create_context(self, entity) -> 'Context':
        """Create a context of the corresponding type.
        """
        if isinstance(entity, Parameters):
            self._with_params(entity, lambda dparams_key, sparams:
                              self._instruct(
                                  Registry._Instruction.Type.INIT_FROM_CONTEXT,
                                  key=key,
                                  interface_origin_key=None,
                                  dparams_key=dparams_key,
                                  sparams=sparams))

            return type(entity).Context()

        elif isinstance(entity, Registry._ContextSpec):
            interface_origin = entity._origin

            def func(dparams_key, sparams):
                if isinstance(interface_origin, Context):
                    self._instruct(
                            Registry._Instruction.Type.INIT_FROM_CONTEXT,
                            key=key,
                            interface_origin_key=interface_origin._key,
                            dparams_key=dparams_key,
                            sparams=sparams)

                elif isinstance(interface_origin, Context._Slot):
                    self._instruct(
                            Registry._Instruction.Type.INIT_FROM_SLOT,
                            key=key,
                            interface_origin_key=interface_origin._context._key,
                            interface_origin_slot_name=interface_origin._name,
                            interface_origin_slot_indices=interface_origin._indices,
                            dparams_key=dparams_key,
                            sparams=sparams)

                else:
                    raise TypeError

            self._with_params(entity._params, func)

            return entity._cls()

        elif isinstance(entity, Context):
            self._instruct(
                    Registry._Instruction.Type.COPY,
                    key=key,
                    original_key=entity._key)

            return type(entity)()

        elif isinstance(entity, list):
            self._instruct(
                    Registry._Instruction.Type.INIT_ARRAY,
                    key=key,
                    num_elements=len(entity),
                    flags=None)

            for index, element in enumerate(entity):
                self._set_slot(key, '', [index], element)

            return ArrayContext()

        else:
            if isinstance(entity, Context._Slot):
                self._instruct(
                        Registry._Instruction.Type.INIT_POINTER,
                        key=key,
                        value=None)

                self._instruct(
                        Registry._Instruction.Type.SET_TO_CONTEXT_SLOT,
                        key=key,
                        slot_name='value',
                        slot_indices=[],
                        source_key=entity._context._key,
                        source_slot_name=entity._name,
                        source_slot_indices=entity._indices)

            elif entity is None or isinstance(entity, CValue):
                self._instruct(
                        Registry._Instruction.Type.INIT_POINTER,
                        key=key,
                        value=entity)

            else:
                self._instruct(
                        Registry._Instruction.Type.INIT_POINTER,
                        key=key,
                        value=CValue(entity))

            return PointerContext()

    def _delete_context(self, key: 'str'):
        """Append a context deletion instruction to the list.
        """
        self._instruct(
                Registry._Instruction.Type.DELETE,
                key=key)

    def _set_slot(self, context_key: 'str', slot_name: 'str', slot_indices: 'list[int]', value):
        """Append a slot setting instruction(s) to the list.
        """
        if isinstance(value, Context):
            self._instruct(
                    Registry._Instruction.Type.SET_TO_CONTEXT_DATA,
                    key=context_key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=value._key)

        elif isinstance(value, Context._Slot):
            self._instruct(
                    Registry._Instruction.Type.SET_TO_CONTEXT_SLOT,
                    key=context_key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=value._context._key,
                    source_slot_name=value._name,
                    source_slot_indices=value._indices)

        else:
            if value is not None and not isinstance(value, CValue):
                value = CValue(value)

            self._instruct(
                    Registry._Instruction.Type.SET_TO_VALUE,
                    key=context_key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    value=value)

    def _invoke_action(self, parameter_list: 'Parameters', action_name: 'str', action_indices: 'list[int]'):
        """Append an action invokation instruction(s) to the list.
        """
        self._with_params(parameter_list, lambda dparams_key, sparams:
                          self._instruct(
                              Registry._Instruction.Type.ACT,
                              key=self._key,
                              action_name=action_name,
                              action_indices=action_indices,
                              dparams_key=dparams_key,
                              sparams=sparams))

    def _with_params(self, params: 'Parameters', func):
        """Prepare a temporary parameter list if needed and invoke a custom function before deleting the list.
        """
        if params.temp_list_key() is None:
            func(params.parent_list_key(), params._params_static)

        else:
            self._instruct(
                    Registry._Instruction.Type.INIT_FROM_CONTEXT,
                    key=params.temp_list_key(),
                    interface_origin_key=None,
                    dparams_key=params.parent_list_key(),
                    sparams=params._params_static)

            for key, value in reversed(params._params_dynamic.items()):
                self._set_slot(params.temp_list_key(), key, [], value)

            func(params.temp_list_key(), {})

            self._instruct(
                    Registry._Instruction.Type.DELETE,
                    key=params.temp_list_key())

    @staticmethod
    def registry_of(context: 'Context') -> 'Registry':
        """Obtain owning registry of a context.
        """
        if not isinstance(context, Context):
            raise TypeError

        return context._registry

    @staticmethod
    def key_of(context: 'Context') -> 'str':
        """Obtain key of a context.
        """
        if not isinstance(context, Context):
            raise TypeError

        return context._key

    @staticmethod
    def interface_of(context: 'Context') -> 'Registry._ContextInterfaceOrigin':
        """Create a representation of context interface origin.
        """
        if not isinstance(context, Context):
            raise TypeError

        return Registry._ContextInterfaceOrigin(context, type(context))

    @staticmethod
    def temp_key(comment: 'str' = None) -> 'str':
        """Generate a random temporary context key with an optional comment in it.
        """
        return '.context' + (f'({comment})_' if comment is not None else '_') + _random_string()

###############################################################################

class Context:
    """Representation of a context.

    This base class does not impose any restrictions on slots and actions.
    """
    class _Slot:
        """Representation of a context slot/action.
        """
        def __init__(self, context: 'Context', *,
                     name: 'str' = '', indices: 'list[int]' = [], is_action: 'bool' = False):
            """Initialize a context slot representation instance.
            """
            object.__setattr__(self, '_context', context)
            object.__setattr__(self, '_name', name)
            object.__setattr__(self, '_indices', indices)
            object.__setattr__(self, '_is_action', is_action)

        def __getattr__(self, name: 'str') -> 'Context._Slot':
            """Obtain a context slot object.
            """
            if self._is_action or self._indices:
                raise AttributeError

            return Context._Slot(self._context, name=f'{self._name}.{name}')

        def __getitem__(self, index: 'int') -> 'Context._Slot':
            """Obtain a context slot object.
            """
            if self._is_action:
                raise AttributeError

            if isinstance(index, int):
                return Context._Slot(self._context, name=self._name, indices=self._indices + [index])

            elif isinstance(index, tuple):
                if not all(isinstance(elt, int) for elt in index):
                    raise TypeError

                return Context._Slot(self._context, name=self._name, indices=self._indices + list(index))

            else:
                raise TypeError

        def __setattr__(self, name: 'str', value):
            """Perform a slot setting operation.
            """
            if self._is_action or self._indices:
                raise AttributeError

            self._context._set(f'{self._name}.{name}', self._indices, value)

        def __setitem__(self, index: 'int', value):
            """Perform a slot setting operation.
            """
            if self._is_action:
                raise AttributeError

            if isinstance(index, int):
                indices = self._indices + [index]

            elif isinstance(index, tuple):
                if not all(isinstance(elt, int) for elt in index):
                    raise TypeError

                indices = self._indices + list(index)

            else:
                raise TypeError

            self._context._set(self._name, indices, value)

        def __call__(self, _: 'Context' = None, /, **params) -> 'Context._Slot':
            """Perform an action.
            """
            if _ is not None and not isinstance(_, Parameters.Context):
                raise TypeError

            self._context._act(self._name, self._indices, _, params)

            return Context._Slot(self._context, name=self._name, indices=self._indices, is_action=True)

    INTERFACE_SYMBOL = None
    InitParametersClass = Parameters

    def __init_subclass__(cls):
        """Initialize a context interface subclass.
        """
        if cls.INTERFACE_SYMBOL is not None and not isinstance(cls.INTERFACE_SYMBOL, str):
            raise TypeError
        elif cls.INTERFACE_SYMBOL is not None and not cls.INTERFACE_SYMBOL:
            raise ValueError
        elif not isinstance(cls.InitParametersClass, type) \
                or not issubclass(cls.InitParametersClass, Parameters):
            raise TypeError

    @classmethod
    def interface(cls, origin) -> 'Registry._ContextInterfaceOrigin':
        """Create a representation of context interface origin.
        """
        from .ctypes.base import archi_context_interface_t

        if isinstance(origin, Context):
            return Registry._ContextInterfaceOrigin(getattr(origin, cls.INTERFACE_SYMBOL), cls)

        elif isinstance(origin, Context._Slot):
            if origin._is_action:
                raise ValueError

            try:
                slot_type = type(origin._context).getter_slot_type(origin._name, origin._indices)
            except KeyError:
                raise KeyError(f"Slot '{origin._name}'[{']['.join([str(i) for i in origin._indices])}] is not recognized")

            if not _compatible_types(slot_type, archi_context_interface_t):
                raise TypeError(f"Slot '{origin._name}'[{']['.join([str(i) for i in origin._indices])}] type is not compatible to a context interface")

            return Registry._ContextInterfaceOrigin(origin, cls)

        else:
            raise TypeError

    @classmethod
    def context_type(cls):
        """Obtain type of the context value.

        This method is to be overridden in derived classes.

        Returns one of:
        - the context value type;
        - string identifier of the context type (for private types);
        - `...` if the context type is unknown.
        """
        return ...

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a getter slot.

        This method is to be overridden in derived classes.

        Returns one of:
        - the slot type;
        - string identifier of the slot type (for private types);
        - `...` if the slot type is unknown.
        Raises KeyError if the slot is not recognized.
        """
        return ... # any slot is recognized by default, type is unknown

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a setter slot.

        This method is to be overridden in derived classes.

        Returns a tuple of two values.
        The first returned value is the slot type as in getter_slot_type().
        The second returned value is None or a callable
        for converting a value into the slot type.
        Raises KeyError if the slot is not recognized.
        """
        return ..., None # any slot is recognized by default, type is unknown, no converter provided

    @classmethod
    def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
        """Obtain parameter list class of an action.

        This method is to be overridden in derived classes.

        Returns a class type derived from Parameters.
        Raises KeyError if the action is not recognized.
        """
        return Parameters # any action is recognized by default, the base parameters class is used

    def __init__(self):
        """Initialize a context representation instance.
        """
        object.__setattr__(self, '_registry', None)
        object.__setattr__(self, '_key', None)

    def __getattr__(self, name: 'str') -> 'Context._Slot':
        """Obtain a context slot object.
        """
        return Context._Slot(self, name=name)

    def __getitem__(self, index: 'int') -> 'Context._Slot':
        """Obtain a context slot object.
        """
        if isinstance(index, int):
            return Context._Slot(self, indices=[index])

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            return Context._Slot(self, indices=list(index))

        else:
            raise TypeError

    def __setattr__(self, name: 'str', value):
        """Perform a slot setting operation.
        """
        self._set(name, [], value)

    def __setitem__(self, index: 'int', value):
        """Perform a slot setting operation.
        """
        if isinstance(index, int):
            indices = [index]

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            indices = list(index)

        else:
            raise TypeError

        self._set('', indices, value)

    def __call__(self, _: 'Context' = None, /, **params) -> 'Context':
        """Perform an action.
        """
        if _ is not None and not isinstance(_, Parameters.Context):
            raise TypeError

        self._act('', [], _, params)

        return self

    def _set(self, slot_name: 'str', slot_indices: 'list[int]', value):
        """Append a set() instruction to the list.
        """
        if self._registry is None:
            raise RuntimeError

        try:
            slot_type, slot_converter = type(self).setter_slot_type(slot_name, slot_indices)
        except KeyError:
            raise KeyError(f"Slot '{slot_name}'[{']['.join([str(i) for i in slot_indices])}] is not recognized")

        if isinstance(value, Context):
            value_type = type(value).context_type()
        elif isinstance(value, Context._Slot):
            try:
                value_type = type(value._context).getter_slot_type(value._name, value._indices)
            except KeyError:
                raise KeyError(f"Slot '{value._name}'[{']['.join([str(i) for i in value._indices])}] is not recognized")
        else:
            if value is None:
                value_type = ...
            elif isinstance(value, CValue):
                value_type = type(value.value())
            else:
                value_type = ...
                value = CValue(slot_converter(value)) \
                        if slot_converter is not None else CValue(value)

        if not _compatible_types(slot_type, value_type):
            raise TypeError(f"Slot '{slot_name}'[{']['.join([str(i) for i in slot_indices])}] type constraint is violated (want = {slot_type}, got = {value_type})")

        self._registry._set_slot(self._key, slot_name, slot_indices, value)

    def _act(self, action_name: 'str', action_indices: 'list[int]',
             parent_params: 'Context', params: 'dict'):
        """Append an act() instruction to the list.
        """
        if self._registry is None:
            raise RuntimeError

        try:
            parameters_class = type(self).action_parameters_class(action_name, action_indices)
        except KeyError:
            raise KeyError(f"Action '{action_name}'[{']['.join([str(i) for i in action_indices])}] is not recognized")

        if not isinstance(parameters_class, type) or not issubclass(parameters_class, Parameters):
            raise TypeError

        parameter_list = parameters_class(parent_params, **params)

        self._registry._invoke_action(parameter_list, action_name, action_indices)

###############################################################################

class Parameters:
    """Representation of a parameter list.
    """
    class Context(Context):
        """Built-in context type for parameter lists.
        """
        INTERFACE_SYMBOL = 'archi_context_parameters_interface'

        @classmethod
        def context_type(cls):
            """Obtain type of the context value.
            """
            from .ctypes.base import archi_parameter_list_t

            return archi_parameter_list_t

        @classmethod
        def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
            """Obtain type of a getter slot.
            """
            if indices:
                raise KeyError

            return cls.InitParametersClass.parameter_type(name)[0]

        @classmethod
        def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
            """Obtain type of a setter slot.
            """
            if indices:
                raise KeyError

            return cls.InitParametersClass.parameter_type(name)

        @classmethod
        def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
            """Obtain parameter list class of an action.
            """
            if name == '_':
                if indices:
                    raise KeyError
                return cls.InitParametersClass
            else:
                raise KeyError

    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        class context_class(cls.Context):
            InitParametersClass = cls

        cls.Context = context_class

    def __init__(self, _: 'Context' = None, /, **params):
        """Initialize a context parameter list representation instance.
        """
        if _ is not None and not isinstance(_, type(self).Context):
            raise TypeError

        self._parent = _

        self._params_dynamic = {}
        self._params_static = {}

        for name, value in params.items():
            try:
                param_type, param_converter = type(self).parameter_type(name)
            except KeyError:
                raise KeyError(f"Parameter '{name}' is not recognized")

            if isinstance(value, Context):
                value_type = type(value).context_type()
                self._params_dynamic[name] = value
            elif isinstance(value, Context._Slot):
                value_type = type(value._context).getter_slot_type(value._name, value._indices)
                self._params_dynamic[name] = value
            elif value is None:
                value_type = ...
                self._params_static[name] = None
            elif isinstance(value, CValue):
                value_type = type(value.value())
                self._params_static[name] = value
            else:
                value_type = ...
                self._params_static[name] = CValue(param_converter(value)) \
                        if param_converter is not None else CValue(value)

            if not _compatible_types(param_type, value_type):
                raise TypeError(f"Parameter '{name}' type constraint is violated (want = {param_type}, got = {value_type})")

        self._temp_key = ('.params_' + _random_string()) if self._params_dynamic else None

    def parent_list(self) -> 'Parameters.Context':
        """Obtain the parent parameter list context.
        """
        return self._parent

    def parent_list_key(self) -> 'str':
        """Obtain key of the parent parameter list.
        """
        return self._parent._key if self._parent is not None else None

    def dynamic_part(self) -> 'dict':
        """Obtain dictionary of dynamic values (contexts and slots) in the parameter list.
        """
        return self._params_dynamic.copy()

    def static_part(self) -> 'dict':
        """Obtain dictionary of static values (constants) in the parameter list.
        """
        return self._params_static.copy()

    def temp_list_key(self) -> 'str':
        """Obtain key of a temporary parameter list.
        """
        return self._temp_key

    @staticmethod
    def parameter_type(name: 'str'):
        """Obtain type of a parameter.

        This method is to be overridden in derived classes.

        Returns a tuple of two values.
        The first returned value is one of:
        - the parameter type;
        - string identifier of the parameter type (for private types);
        - `...` if the parameter type is unknown.
        The second returned value is None or a callable
        for converting a value into the parameter type.
        Raises KeyError if the parameter is not recognized.
        """
        return ..., None # all parameters are recognized by default, no types are known

###############################################################################

class PointerContext(Context):
    """Built-in context type for pointers.
    """
    INTERFACE_SYMBOL = 'archi_context_pointer_interface'

    class InitParameters(Parameters):
        """Pointer context initialization parameters.
        """
        @staticmethod
        def parameter_type(name: 'str'):
            """Obtain type of a parameter.
            """
            import ctypes as c
            from .ctypes.base import archi_array_layout_t, archi_pointer_flags_t

            if name == 'value':
                return ..., None
            elif name == 'flags':
                return archi_pointer_flags_t, \
                        lambda value: archi_pointer_flags_t(value)
            elif name == 'layout':
                return archi_array_layout_t, \
                        lambda value: archi_array_layout_t(*value)
            elif name in ['num_elements', 'element_size', 'element_alignment']:
                return c.c_size_t, lambda value: c.c_size_t(value)
            else:
                raise KeyError

    InitParametersClass = PointerContext.InitParameters

    class ActionCopyParameters(Parameters):
        """Pointer 'copy' action parameters.
        """
        @staticmethod
        def parameter_type(name: 'str'):
            """Obtain type of a parameter.
            """
            import ctypes as c

            if name == 'source':
                return ..., None
            elif name in ['source_offset', 'num_elements']:
                return c.c_size_t, lambda value: c.c_size_t(value)
            else:
                raise KeyError

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a getter slot.
        """
        import ctypes as c
        from .ctypes.base import archi_array_layout_t, archi_pointer_flags_t

        if name == '':
            if len(indices) != 1:
                raise KeyError
            return ...
        elif name == 'flags':
            if indices:
                raise KeyError
            return archi_pointer_flags_t
        elif name == 'layout':
            if indices:
                raise KeyError
            return archi_array_layout_t
        elif name in ['num_elements', 'element_size', 'element_alignment', 'full_size']:
            if indices:
                raise KeyError
            return c.c_size_t
        else:
            raise KeyError

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a setter slot.
        """
        if name == 'value':
            if indices:
                raise KeyError
            return ..., None
        elif name == '':
            if len(indices) != 1:
                raise KeyError
            return ..., None
        else:
            raise KeyError

    @classmethod
    def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
        """Obtain parameter list class of an action.
        """
        if name == 'update':
            if indices:
                raise KeyError
            return cls.InitParameters
        elif name == 'copy':
            if len(indices) > 1:
                raise KeyError
            return cls.ActionCopyParameters
        else:
            raise KeyError


class ArrayContext(Context):
    """Built-in context type for arrays of pointers.
    """
    INTERFACE_SYMBOL = 'archi_context_array_interface'

    class InitParameters(Parameters):
        """Array context initialization parameters.
        """
        @staticmethod
        def parameter_type(name: 'str'):
            """Obtain type of a parameter.
            """
            import ctypes as c
            from .ctypes.base import archi_pointer_flags_t

            if name == 'num_elements':
                return c.c_size_t, lambda value: c.c_size_t(value)
            elif name == 'flags':
                return archi_pointer_flags_t, \
                        lambda value: archi_pointer_flags_t(value)
            else:
                raise KeyError

    InitParametersClass = ArrayContext.InitParameters

    @classmethod
    def context_type(cls):
        """Obtain type of the context value.
        """
        import ctypes as c

        return c.c_void_p

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a getter slot.
        """
        import ctypes as c
        from .ctypes.base import archi_pointer_t

        if name == '':
            if len(indices) != 1:
                raise KeyError
            return ...
        elif name == 'elements':
            if indices:
                raise KeyError
            return c.POINTER(archi_pointer_t)
        elif name == 'num_elements':
            if indices:
                raise KeyError
            return c.c_size_t
        else:
            raise KeyError

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a setter slot.
        """
        if name == '':
            if len(indices) != 1:
                raise KeyError
            return ..., None
        else:
            raise KeyError

###############################################################################

def _compatible_types(typ1, typ2) -> 'bool':
    """Compare two types for compatibility.
    """
    if not isinstance(typ1, type) and not isinstance(typ1, str) and typ1 is not Ellipsis:
        raise TypeError
    elif not isinstance(typ2, type) and not isinstance(typ2, str) and typ2 is not Ellipsis:
        raise TypeError

    if typ1 is Ellipsis or typ2 is Ellipsis:
        return True

    return typ1 == typ2

def _random_string(length: 'int' = 8) -> 'str':
    """Generate a random string.
    """
    import random
    import string

    return ''.join(random.choice(string.ascii_letters + string.digits) for char in range(length))

