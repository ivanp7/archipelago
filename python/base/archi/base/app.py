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

import ctypes as c
import enum
from contextlib import contextmanager
from types import SimpleNamespace

from .memory import CValue
from .ctypes.base import (
        archi_array_layout_t,
        archi_pointer_flags_t,
        archi_pointer_t,
        archi_parameter_list_t,
        )


class TypeDescriptor:
    """The base class for description of value types.
    """
    def __init__(self):
        """Create a type descriptor instance.
        """
        raise NotImplementedError

    def is_type_compatible(self, another_type) -> 'bool':
        """Check compatibility of types.
        Returns True if the type is compatible, otherwise False.
        """
        return False # no type is compatible by default

    def construct_value(self, value):
        """Construct a value of the target type.
        Returns the value of the target type, or raises TypeError.
        """
        raise TypeError # unable to construct a value by default


class PrivateType(TypeDescriptor):
    """Description of private types.
    """
    def __init__(self, id: 'str'):
        """Create a type descriptor instance.
        """
        if not isinstance(id, str):
            raise TypeError

        self._id = id

    def __str__(self):
        return f"PrivateType(id='{self._id}')"

    def type_id(self) -> 'str':
        """Get private type identifier.
        """
        return self._id

    def is_type_compatible(self, another_type) -> 'bool':
        """Check compatibility of types.
        Returns True if types are compatible, otherwise False.
        """
        if not isinstance(another_type, PrivateType):
            return False

        return self.type_id() == another_type.type_id()


class PublicType(TypeDescriptor):
    """Description of public types.
    """
    def __init__(self, cls: 'type', constr=None):
        """Create a type descriptor instance.
        """
        if not isinstance(cls, type):
            raise TypeError
        elif constr is not None and not callable(constr):
            raise TypeError

        self._cls = cls
        self._constr = constr

    def __str__(self):
        return f"PublicType(cls={self._cls})"

    def type_class(self) -> 'type':
        """Get public type class.
        """
        return self._cls

    def type_constructor(self):
        """Get value constructor for the type.
        """
        return self._constr if self._constr is not None else self._cls

    def is_type_compatible(self, another_type) -> 'bool':
        """Check compatibility of types.
        Returns True if types are compatible, otherwise False.
        """
        if isinstance(another_type, type):
            return issubclass(another_type, self.type_class())
        elif isinstance(another_type, PublicType):
            return issubclass(another_type.type_class(), self.type_class())
        else:
            return False

    def construct_value(self, value):
        """Construct a value of the target type.
        Returns the value of the target type, or raises TypeError.
        """
        if isinstance(value, self._cls):
            return value
        else:
            return self.type_constructor()(value)

###############################################################################

class Context:
    """Representation of a context.

    This base class does not impose any restrictions on slots and actions.
    """
    class _Slot:
        """Representation of a context slot/action.
        """
        def __init__(self, context: 'Context', *, name: 'str' = '',
                     indices: 'list[int]' = [], is_action: 'bool' = False):
            """Initialize a context slot representation instance.
            """
            fields = SimpleNamespace()

            fields.context = context
            fields.name = name
            fields.indices = indices
            fields.is_action = is_action

            object.__setattr__(self, '_', fields)

        def __str__(self):
            return f"Context._Slot(context.key='{self._.context._.key}', name='{self._.name}', indices={self._.indices}, is_action={self._.is_action})"

        def __getattr__(self, name: 'str') -> 'Context._Slot':
            """Obtain a context slot object.
            """
            if self._.is_action or self._.indices:
                raise AttributeError

            return Context._Slot(self._.context, name=f'{self._.name}.{name}')

        def __getitem__(self, index: 'int') -> 'Context._Slot':
            """Obtain a context slot object.
            """
            if self._.is_action:
                raise AttributeError

            if isinstance(index, int):
                return Context._Slot(self._.context, name=self._.name,
                                     indices=self._.indices + [index])

            elif isinstance(index, tuple):
                if not all(isinstance(elt, int) for elt in index):
                    raise TypeError

                return Context._Slot(self._.context, name=self._.name,
                                     indices=self._.indices + list(index))

            else:
                raise TypeError

        def __setattr__(self, name: 'str', value):
            """Perform a slot setting operation.
            """
            if self._.is_action or self._.indices:
                raise AttributeError

            self._.context._set(f'{self._.name}.{name}', self._.indices, value)

        def __setitem__(self, index: 'int', value):
            """Perform a slot setting operation.
            """
            if self._.is_action:
                raise AttributeError

            if isinstance(index, int):
                indices = self._.indices + [index]

            elif isinstance(index, tuple):
                if not all(isinstance(elt, int) for elt in index):
                    raise TypeError

                indices = self._.indices + list(index)

            else:
                raise TypeError

            self._.context._set(self._.name, indices, value)

        def __call__(self, _: 'Context' = None, /, **params) -> 'Context._Slot':
            """Perform an action.
            """
            if not isinstance(_, (type(None), Parameters.Context)):
                raise TypeError

            self._.context._act(self._.name, self._.indices, _, params)

            return Context._Slot(self._.context, name=self._.name,
                                 indices=self._.indices, is_action=True)

    # Name of symbol of the context interface
    INTERFACE_SYMBOL = None

    # Descriptor of the context data type
    DATA_TYPE = None

    # Context initialization parameters class
    INIT_PARAMETERS_CLASS = None # will be set to Parameters later

    def __init_subclass__(cls):
        """Initialize a context interface subclass.
        """
        if cls.INTERFACE_SYMBOL is not None:
            if not isinstance(cls.INTERFACE_SYMBOL, str):
                raise TypeError
            elif not cls.INTERFACE_SYMBOL:
                raise ValueError

        if not isinstance(cls.DATA_TYPE, (type(None), TypeDescriptor)):
            raise TypeError

        if cls.INIT_PARAMETERS_CLASS is not None \
                and (not isinstance(cls.INIT_PARAMETERS_CLASS, type) \
                or not issubclass(cls.INIT_PARAMETERS_CLASS, Parameters)):
            raise TypeError

    def __str__(self):
        return f"Context(key='{self._.key}')"

    @classmethod
    def interface(cls, origin) -> 'Registry._ContextInterfaceOrigin':
        """Create a representation of context interface origin.
        """
        from .ctypes.context import archi_context_interface_t

        if isinstance(origin, Context):
            return Registry._ContextInterfaceOrigin(getattr(origin, cls.INTERFACE_SYMBOL), cls)
        elif isinstance(origin, Context._Slot):
            if origin._.is_action:
                raise ValueError

            try:
                slot_type = type(origin._.context).getter_slot_type(origin._.name, origin._.indices)
            except KeyError:
                _error_slot_unrecognized(origin._.name, origin._.indices)

            if slot_type is not None and not slot_type.is_type_compatible(archi_context_interface_t):
                raise TypeError(f"Slot '{origin._.name}'[{']['.join([str(i) for i in origin._.indices])}] type is not compatible to a context interface")

            return Registry._ContextInterfaceOrigin(origin, cls)
        else:
            raise TypeError

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a getter slot.

        This method is to be overridden in derived classes.

        Returns a TypeDescriptor object or None.
        Raises KeyError if the slot is not recognized.
        """
        return None # any slot is recognized by default, type is unknown

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a setter slot.

        This method is to be overridden in derived classes.

        Returns a TypeDescriptor object or None.
        Raises KeyError if the slot is not recognized.
        """
        return None # any slot is recognized by default, type is unknown

    @classmethod
    def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
        """Obtain parameter list class of an action.

        This method is to be overridden in derived classes.

        Returns a (sub)class of Parameters.
        Raises KeyError if the action is not recognized.
        """
        return Parameters # any action is recognized by default, the base parameters class is used

    def __init__(self):
        """Initialize a context representation instance.
        """
        fields = SimpleNamespace()

        fields.registry = None
        fields.key = None

        object.__setattr__(self, '_', fields)

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
        if not isinstance(_, (type(None), Parameters.Context)):
            raise TypeError

        self._act('', [], _, params)

        return Context._Slot(self._.context, is_action=True)

    def _set(self, slot_name: 'str', slot_indices: 'list[int]', value):
        """Append a set() instruction to the list.
        """
        if self._.registry is None:
            raise RuntimeError

        try:
            slot_type = type(self).setter_slot_type(slot_name, slot_indices)
        except KeyError:
            _error_slot_unrecognized(slot_name, slot_indices)

        if isinstance(value, Context):
            value_type = type(value).DATA_TYPE
        elif isinstance(value, Context._Slot):
            try:
                value_type = type(value._.context).getter_slot_type(value._.name, value._.indices)
            except KeyError:
                _error_slot_unrecognized(value._.name, value._.indices)
        else:
            if value is None:
                value_type = None
            elif isinstance(value, CValue):
                value_type = type(value.value())
            else:
                value_type = None
                value = CValue(slot_type.construct_value(value) \
                        if slot_type is not None else value)

        if slot_type is not None and value_type is not None \
                and not slot_type.is_type_compatible(value_type):
            raise TypeError(f"Slot '{slot_name}'[{']['.join([str(i) for i in slot_indices])}] type constraint is violated (want = {slot_type}, got = {value_type})")

        self._.registry._set_slot(self._.key, slot_name, slot_indices, value)

    def _act(self, action_name: 'str', action_indices: 'list[int]',
             parent_params: 'Context', params: 'dict'):
        """Append an act() instruction to the list.
        """
        if self._.registry is None:
            raise RuntimeError

        try:
            parameters_class = type(self).action_parameters_class(action_name, action_indices)
        except KeyError:
            _error_slot_unrecognized(action_name, action_indices, action=True)

        if not isinstance(parameters_class, type) or not issubclass(parameters_class, Parameters):
            raise TypeError

        parameter_list = parameters_class(parent_params, **params)

        self._.registry._invoke_action(self._.key, parameter_list, action_name, action_indices)


class Parameters:
    """Representation of a parameter list.

    This base class does not impose any restrictions on parameters.
    """
    class Context(Context):
        """Built-in context type for parameter lists.
        """
        INTERFACE_SYMBOL = 'archi_context_parameters_interface'

        DATA_TYPE = PublicType(archi_parameter_list_t)

        @classmethod
        def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
            """Obtain type of a getter slot.
            """
            if indices:
                raise KeyError

            return cls.INIT_PARAMETERS_CLASS.parameter_type(name)

        @classmethod
        def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
            """Obtain type of a setter slot.
            """
            if indices:
                raise KeyError

            return cls.INIT_PARAMETERS_CLASS.parameter_type(name)

        @classmethod
        def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
            """Obtain parameter list class of an action.
            """
            if name == '':
                if indices:
                    raise KeyError
                return cls.INIT_PARAMETERS_CLASS
            else:
                raise KeyError

    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        class context_class(cls.Context):
            INIT_PARAMETERS_CLASS = cls

        cls.Context = context_class

    def __init__(self, _: 'Context' = None, /, **params):
        """Initialize a context parameter list representation instance.
        """
        if not isinstance(_, (type(None), type(self).Context)):
            raise TypeError

        self._parent = _

        self._params_dynamic = {}
        self._params_static = {}

        for name, value in params.items():
            try:
                param_type = type(self).parameter_type(name)
            except KeyError:
                raise KeyError(f"Parameter '{name}' is not recognized")

            if isinstance(value, Context):
                value_type = type(value).DATA_TYPE
                self._params_dynamic[name] = value
            elif isinstance(value, Context._Slot):
                value_type = type(value._.context).getter_slot_type(value._.name, value._.indices)
                self._params_dynamic[name] = value
            elif value is None:
                value_type = None
                self._params_static[name] = None
            elif isinstance(value, CValue):
                value_type = type(value.value())
                self._params_static[name] = value
            else:
                value_type = None
                self._params_static[name] = CValue(param_type.construct_value(value) \
                        if param_type is not None else value)

            if param_type is not None and value_type is not None \
                    and not param_type.is_type_compatible(value_type):
                raise TypeError(f"Parameter '{name}' type constraint is violated (want = {param_type}, got = {value_type})")

        self._temp_key = ('.params_' + _random_string()) if self._params_dynamic else None

    def parent_list(self) -> 'Parameters.Context':
        """Obtain the parent parameter list context.
        """
        return self._parent

    def parent_list_key(self) -> 'str':
        """Obtain key of the parent parameter list.
        """
        return self._parent._.key if self._parent is not None else None

    def dynamic_part(self) -> 'dict':
        """Obtain dictionary of dynamic values (contexts and slots) in the parameter list.
        """
        from types import MappingProxyType

        return MappingProxyType(self._params_dynamic)

    def static_part(self) -> 'dict':
        """Obtain dictionary of static values (constants) in the parameter list.
        """
        from types import MappingProxyType

        return MappingProxyType(self._params_static)

    def temp_list_key(self) -> 'str':
        """Obtain key of a temporary parameter list.
        """
        return self._temp_key

    @classmethod
    def parameter_type(cls, name: 'str'):
        """Obtain type of a parameter.

        This method is to be overridden in derived classes.

        Returns a TypeDescriptor object or None.
        Raises KeyError if the parameter is not recognized.
        """
        return None # all parameters are recognized by default, no types are known


Context.INIT_PARAMETERS_CLASS = Parameters
Parameters.Context.INIT_PARAMETERS_CLASS = Parameters

###############################################################################

class ParametersWhitelistable(Parameters):
    """Representation of a parameter list with the fixed set of recognized parameters.
    """
    PARAMETERS = {}

    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        if not isinstance(cls.PARAMETERS, dict):
            raise TypeError
        elif not all(isinstance(key, str) for key in cls.PARAMETERS.keys()):
            raise TypeError
        elif not all(isinstance(value, (type(None), TypeDescriptor)) for value in cls.PARAMETERS.values()):
            raise TypeError

        super().__init_subclass__()

    @classmethod
    def parameter_type(cls, name: 'str'):
        """Obtain type of a parameter.
        """
        return cls.PARAMETERS[name]


class ContextWhitelistable(Context):
    """Representation of a context with the fixed set of recognized slots and actions.
    """
    GETTER_SLOT_TYPES = {}
    SETTER_SLOT_TYPES = {}
    ACTION_PARAMETER_CLASSES = {}

    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        def check(var, fn):
            if not isinstance(var, dict):
                raise TypeError
            elif not all(isinstance(key, str) for key in var.keys()):
                raise TypeError
            elif not all(isinstance(value, dict) for value in var.values()):
                raise TypeError
            elif not all(all(isinstance(index, int) and index >= 0 for index in value.keys()) \
                    for value in var.values()):
                raise TypeError
            elif not all(all(fn(descr) for descr in value.values()) \
                    for value in var.values()):
                raise TypeError

        check(cls.GETTER_SLOT_TYPES,
              lambda descr: isinstance(descr, (type(None), TypeDescriptor)))
        check(cls.SETTER_SLOT_TYPES,
              lambda descr: isinstance(descr, (type(None), TypeDescriptor)))
        check(cls.ACTION_PARAMETER_CLASSES,
              lambda descr: isinstance(descr, type) and issubclass(descr, Parameters))

        super().__init_subclass__()

    @classmethod
    def getter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a getter slot.
        """
        return cls.GETTER_SLOT_TYPES[name][len(indices)]

    @classmethod
    def setter_slot_type(cls, name: 'str', indices: 'list[int]'):
        """Obtain type of a setter slot.
        """
        return cls.SETTER_SLOT_TYPES[name][len(indices)]

    @classmethod
    def action_parameters_class(cls, name: 'str', indices: 'list[int]'):
        """Obtain parameter list class of an action.
        """
        return cls.ACTION_PARAMETER_CLASSES[name][len(indices)]

###############################################################################

_TYPE_SIZE = PublicType(c.c_size_t)
_TYPE_ARRAY_LAYOUT = PublicType(archi_array_layout_t, lambda v: archi_array_layout_t(*v))
_TYPE_POINTER_FLAGS = PublicType(archi_pointer_flags_t)
_TYPE_POINTER = PublicType(archi_pointer_t)
_TYPE_VOID_P = PublicType(c.c_void_p)

class PointerContext(ContextWhitelistable):
    """Built-in context type for pointers.
    """
    class InitParameters(ParametersWhitelistable):
        """Pointer context initialization parameters.
        """
        PARAMETERS = {
                'value': None,
                'flags': _TYPE_POINTER_FLAGS,
                'layout': _TYPE_ARRAY_LAYOUT,
                'num_elements': _TYPE_SIZE,
                'element_size': _TYPE_SIZE,
                'element_alignment': _TYPE_SIZE,
                }

    class ActionCopyParameters(ParametersWhitelistable):
        """Pointer 'copy' action parameters.
        """
        PARAMETERS = {
                'source': None,
                'source_offset': _TYPE_SIZE,
                'num_elements': _TYPE_SIZE,
                }

    INTERFACE_SYMBOL = 'archi_context_pointer_interface'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            '': {1: None},
            'flags': {0: _TYPE_POINTER_FLAGS},
            'layout': {0: _TYPE_ARRAY_LAYOUT},
            'num_elements': {0: _TYPE_SIZE},
            'element_size': {0: _TYPE_SIZE},
            'element_alignment': {0: _TYPE_SIZE},
            'full_size': {0: _TYPE_SIZE},
            }

    SETTER_SLOT_TYPES = {
            'value': {0: None},
            '': {1: None},
            }

    ACTION_PARAMETER_CLASSES = {
            'update': {0: InitParameters},
            'copy': {0: ActionCopyParameters,
                     1: ActionCopyParameters},
            }


class ArrayContext(ContextWhitelistable):
    """Built-in context type for arrays of pointers.
    """
    class InitParameters(ParametersWhitelistable):
        """Array context initialization parameters.
        """
        PARAMETERS = {
                'num_elements': _TYPE_SIZE,
                'flags': _TYPE_POINTER_FLAGS,
                }

    INTERFACE_SYMBOL = 'archi_context_array_interface'

    DATA_TYPE = _TYPE_VOID_P

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            '': {1: None},
            'elements': {0: _TYPE_POINTER},
            'num_elements': {0: _TYPE_SIZE},
            }

    SETTER_SLOT_TYPES = {
            '': {1: None},
            }

###############################################################################

class Registry:
    """Representation of an Archipelago executable registry.
    """
    class InitializerTypeHandler:
        """The base class for handling initializers of custom types.
        """
        @classmethod
        def replace_initializer(cls, initializer):
            """Replace context initializer of the handled type.
            Returns an initializer of another type.
            """
            raise NotImplementedError

        @classmethod
        def prepare_context(cls, context: 'Context', initializer):
            """Prepare the created context using the original initializer.
            """
            pass

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
            return Registry._ContextSpec(self, self._cls.INIT_PARAMETERS_CLASS(_, **params))

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
            DELETE = enum.auto()
            COPY = enum.auto()
            INIT_PARAMETERS = enum.auto()
            INIT_POINTER = enum.auto()
            INIT_ARRAY = enum.auto()
            INIT_FROM_CONTEXT = enum.auto()
            INIT_FROM_SLOT = enum.auto()
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

    def __setitem__(self, key: 'str', initializer):
        """Create a context and insert it to the registry.
        """
        if not isinstance(key, str):
            raise TypeError
        elif key in self._contexts:
            raise KeyError(f"Context '{key}' is already in the registry")

        context = self._create_context(key, initializer)

        if context is not None:
            context._.registry = self
            context._.key = key

            self._contexts[key] = context
        else:
            if type(initializer) not in self._initializer_types:
                raise TypeError(f"Can't create a context from unknown type {type(initializer)}")

            handler = self._initializer_types[type(initializer)]

            self[key] = handler.replace_initializer(initializer)
            handler.prepare_context(self[key], initializer)

    def __delitem__(self, key: 'str'):
        """Finalize a context and remove it from the registry.
        """
        if key is None:
            return
        elif not isinstance(key, str):
            raise TypeError

        self._delete_context(key)
        del self._contexts[key]

    def noop(self):
        """Emit a NOOP instruction.
        """
        self._instruct(Registry._Instruction.Type.NOOP)

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
        context._.registry = self
        context._.key = key

        self._contexts[key] = context
        return context

    def new_context(self, value, /, key: 'str') -> 'Context':
        """Create a new context and return it.
        """
        self[key] = value
        return self[key]

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
            key = Registry.random_key()

        self[key] = value

        with self.del_context(key) as context:
            yield context

    def contexts(self, cls: 'type' = Context) -> 'dict[str, Context]':
        """Obtain the dictionary of known contexts of the specified type.
        """
        return {key: value for key, value in self._contexts.items() if isinstance(value, cls)}

    def register_initializer_type(self, initializer_type: 'type', handler: 'Registry.InitializerTypeHandler'):
        """Register an initializer type handler.
        """
        if not isinstance(initializer_type, type):
            raise TypeError
        elif not isinstance(handler, Registry.InitializerTypeHandler):
            raise TypeError

        if initializer_type in self._initializer_types:
            raise KeyError

        self._initializer_types[initializer_type] = handler

    def unregister_initializer_type(self, initializer_type: 'type'):
        """Unregister an initializer type handler.
        """
        if not isinstance(initializer_type, type):
            raise TypeError

        del self._initializer_types[initializer_type]

    def initializer_types(self) -> 'dict[type, Registry.InitializerTypeHandler]':
        """Obtain the dictionary of registered initializer type handlers.
        """
        from types import MappingProxyType

        return MappingProxyType(self._initializer_types)

    def reset(self):
        """Reset the list of instructions.
        """
        self._contexts = {}
        self._initializer_types = {}
        self._instructions = []

    def _instruct(self, type: 'Registry._Instruction.Type', /, **fields):
        """Append an instruction to the list.
        """
        self._instructions.append(Registry._Instruction(type, **fields))

    def _create_context(self, key: 'str', initializer) -> 'Context':
        """Emit instructions for creation of a context.
        Return a context representation object of the corresponding type.
        """
        if isinstance(initializer, Parameters):
            self._with_params(initializer, lambda dparams_key, sparams:
                              self._instruct(
                                  Registry._Instruction.Type.INIT_PARAMETERS,
                                  key=key,
                                  dparams_key=dparams_key,
                                  sparams=sparams))

            return type(initializer).Context()

        elif isinstance(initializer, Registry._ContextSpec):
            interface_origin = initializer._origin

            def func(dparams_key, sparams):
                if isinstance(interface_origin, Context):
                    self._instruct(
                            Registry._Instruction.Type.INIT_FROM_CONTEXT,
                            key=key,
                            interface_origin_key=interface_origin._.key,
                            dparams_key=dparams_key,
                            sparams=sparams)

                elif isinstance(interface_origin, Context._Slot):
                    self._instruct(
                            Registry._Instruction.Type.INIT_FROM_SLOT,
                            key=key,
                            interface_origin_key=interface_origin._.context._.key,
                            interface_origin_slot_name=interface_origin._.name,
                            interface_origin_slot_indices=interface_origin._.indices,
                            dparams_key=dparams_key,
                            sparams=sparams)

                else:
                    raise TypeError

            self._with_params(initializer._params, func)

            return initializer._cls()

        elif isinstance(initializer, Context):
            self._instruct(
                    Registry._Instruction.Type.COPY,
                    key=key,
                    original_key=initializer._.key)

            return type(initializer)()

        elif isinstance(initializer, Context._Slot):
            self._instruct(
                    Registry._Instruction.Type.INIT_POINTER,
                    key=key,
                    value=None)

            self._instruct(
                    Registry._Instruction.Type.SET_TO_CONTEXT_SLOT,
                    key=key,
                    slot_name='value',
                    slot_indices=[],
                    source_key=initializer._.context._.key,
                    source_slot_name=initializer._.name,
                    source_slot_indices=initializer._.indices)

            return PointerContext()

        elif isinstance(initializer, (type(None), CValue)):
            self._instruct(
                    Registry._Instruction.Type.INIT_POINTER,
                    key=key,
                    value=initializer)

            return PointerContext()

        elif isinstance(initializer, list):
            self._instruct(
                    Registry._Instruction.Type.INIT_ARRAY,
                    key=key,
                    num_elements=len(initializer),
                    flags=None)

            for index, element in enumerate(initializer):
                self._set_slot(key, '', [index], element)

            return ArrayContext()

        else:
            return None

    def _delete_context(self, key: 'str'):
        """Emit instruction for deletion of a context.
        """
        self._instruct(
                Registry._Instruction.Type.DELETE,
                key=key)

    def _set_slot(self, context_key: 'str', slot_name: 'str', slot_indices: 'list[int]', value):
        """Emit instructions for calling a context slot setter.
        """
        if isinstance(value, Context):
            self._instruct(
                    Registry._Instruction.Type.SET_TO_CONTEXT_DATA,
                    key=context_key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=value._.key)

        elif isinstance(value, Context._Slot):
            self._instruct(
                    Registry._Instruction.Type.SET_TO_CONTEXT_SLOT,
                    key=context_key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=value._.context._.key,
                    source_slot_name=value._.name,
                    source_slot_indices=value._.indices)

        else:
            if not isinstance(value, (type(None), CValue)):
                value = CValue(value)

            self._instruct(
                    Registry._Instruction.Type.SET_TO_VALUE,
                    key=context_key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    value=value)

    def _invoke_action(self, context_key: 'str', parameter_list: 'Parameters',
                       action_name: 'str', action_indices: 'list[int]'):
        """Emit instructions for calling a context action.
        """
        self._with_params(parameter_list, lambda dparams_key, sparams:
                          self._instruct(
                              Registry._Instruction.Type.ACT,
                              key=context_key,
                              action_name=action_name,
                              action_indices=action_indices,
                              dparams_key=dparams_key,
                              sparams=sparams))

    def _with_params(self, params: 'Parameters', func):
        """Prepare a temporary parameter list if needed and call a custom function before deleting the list.
        """
        if params.temp_list_key() is None:
            func(params.parent_list_key(), params._params_static)
        else:
            self._instruct(
                    Registry._Instruction.Type.INIT_PARAMETERS,
                    key=params.temp_list_key(),
                    dparams_key=params.parent_list_key(),
                    sparams=params._params_static)

            for key, value in reversed(params._params_dynamic.items()):
                self._set_slot(params.temp_list_key(), key, [], value)

            func(params.temp_list_key(), {})

            self._delete_context(params.temp_list_key())

    @staticmethod
    def registry_of(context: 'Context') -> 'Registry':
        """Obtain owning registry of a context.
        """
        if not isinstance(context, Context):
            raise TypeError

        return context._.registry

    @staticmethod
    def key_of(context: 'Context') -> 'str':
        """Obtain key of a context.
        """
        if not isinstance(context, Context):
            raise TypeError

        return context._.key

    @staticmethod
    def interface_of(context: 'Context') -> 'Registry._ContextInterfaceOrigin':
        """Create a representation of context interface origin.
        """
        if not isinstance(context, Context):
            raise TypeError

        return Registry._ContextInterfaceOrigin(context, type(context))

    @staticmethod
    def random_key(comment: 'str' = None) -> 'str':
        """Generate a random temporary context key with an optional comment in it.
        """
        return '.context' + (f'({comment})_' if comment is not None else '_') + _random_string()

###############################################################################

def _random_string(length: 'int' = 8) -> 'str':
    """Generate a random string.
    """
    import random
    import string

    return ''.join(random.choice(string.ascii_letters + string.digits) for char in range(length))


def _error_slot_unrecognized(name, indices, action=False):
    """Raise an exception for an unrecognized slot/action.
    """
    raise KeyError(("Action" if action else "Slot") +
                   f" '{name}[{']['.join([str(i) for i in indices])}]' is not recognized")

