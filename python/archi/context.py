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
# @brief Archipelago application context & registry operations.

from contextlib import contextmanager
from types import MappingProxyType, SimpleNamespace
import ctypes as c

from .ctypes import archi_pointer_attr_t, ARCHI_POINTER_DATA_TAG__KVLIST, ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE
from .object import Object, PrimitiveData


TypeAttributes = archi_pointer_attr_t


class Context:
    """Representation of a context.

    This base class does not impose any restrictions on slots.
    """
    # Prefix of symbol name of the context interface
    C_INTERFACE_PREFIX = 'archi_context_interface__'

    # Main part of symbol name of the context interface
    C_NAME = None

    def __init_subclass__(cls):
        """Initialize a context interface subclass.
        """
        if cls.C_NAME is not None:
            if not isinstance(cls.C_NAME, str):
                raise TypeError
            elif not cls.C_NAME:
                raise ValueError

    def __init__(self):
        """Initialize a context instance.
        """
        fields = SimpleNamespace()

        fields.registry = None
        fields.key = None

        object.__setattr__(self, '_', fields)

    def __str__(self):
        return f"Context({self._})"

    def __getattr__(self, name):
        """Get a context slot.
        """
        return _ContextSlot(self, name, ())

    def __getitem__(self, index):
        """Get a context slot.
        """
        if isinstance(index, int):
            return _ContextSlot(self, '', (index,))

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            return _ContextSlot(self, '', index)

        else:
            raise TypeError

    def __setattr__(self, name, value):
        """Set a context slot.
        """
        Context._set(self, name, (), value)

    def __setitem__(self, index, value):
        """Set a context slot.
        """
        if isinstance(index, int):
            Context._set(self, '', (index,), value)

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            Context._set(self, '', index, value)

        else:
            raise TypeError

    def __delattr__(self, name):
        """Unset a context slot.
        """
        Context._unset(self, name, ())

    def __delitem__(self, index, value):
        """Unset a context slot.
        """
        if isinstance(index, int):
            Context._unset(self, '', (index,))

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            Context._unset(self, '', index)

        else:
            raise TypeError

    def __call__(self, _=None, /, **params):
        """Invoke a call.
        """
        if not isinstance(_, (type(None), Parameters.Context)):
            raise TypeError

        call_params_cls = type(self).call_params_class('', ())

        return _ContextSlot(self, '', (), call_params=call_params_cls(_, **params))

    @staticmethod
    def registry_of(context, /):
        """Get the registry a context belongs to.
        """
        if context is None:
            return None

        if not isinstance(context, Context):
            raise TypeError

        return context._.registry

    @staticmethod
    def key_of(context, /):
        """Get the key of a context.
        """
        if context is None:
            return None

        if not isinstance(context, Context):
            raise TypeError

        return context._.key

    @staticmethod
    def weak_ref(entity, /):
        """Mark the entity as requiring weak assignment operation (create weak reference).
        """
        if isinstance(entity, Context):
            return _ContextSlot(entity, '', (), weak_ref=True)
        elif isinstance(entity, _ContextSlot):
            return _ContextSlot(entity._.context, entity._.name, entity._.indices,
                                entity._.call_params, weak_ref=True)
        else:
            return entity

    @classmethod
    def interface(cls, context_or_slot=None, /, *, library=None):
        """Create a representation of a context interface.
        """
        if context_or_slot is not None and library is not None:
            raise ValueError

        if context_or_slot is not None:
            if not isinstance(context_or_slot, (Context, _ContextSlot)):
                raise TypeError

            attr = _get_entity_attr(context_or_slot)

            if isinstance(attr, TypeAttributes) and not attr.is_compatible_to(
                    TypeAttributes.complex_data(ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE)):
                raise TypeError(f"{context_or_slot} is not a context interface")

            if isinstance(context_or_slot, Context):
                return _ContextInterface(_ContextSlot(context_or_slot, '', ()), cls)
            elif isinstance(context_or_slot, _ContextSlot):
                if context_or_slot._.call_params is not None:
                    raise ValueError("Context interface cannot be obtained from a call")

                return _ContextInterface(context_or_slot, cls)

        elif library is not None:
            if not isinstance(library, Context):
                raise TypeError

            return _ContextInterface(getattr(library, cls.C_INTERFACE_PREFIX + cls.C_NAME), cls)

        else:
            raise ValueError

    @classmethod
    def init_params_class(cls):
        """Obtain initialization parameter list class.

        Returns a (sub)class of Parameters.
        """
        params_class = cls._init_params_class()

        if not issubclass(params_class, Parameters):
            raise TypeError

        return params_class

    @classmethod
    def call_params_class(cls, name='', indices=()):
        """Obtain parameter list class of a call.

        Returns a (sub)class of Parameters.
        Raises KeyError if the call is not recognized.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not isinstance(indices, tuple) or not all(isinstance(index, int) for index in indices):
            raise TypeError

        try:
            params_class = cls._call_params_class(name, indices)
        except KeyError:
            raise KeyError(f"{cls}: call slot {_slot_str(name, indices)} is not recognized")

        if not issubclass(params_class, Parameters):
            raise TypeError

        return params_class

    @classmethod
    def slot_attr(cls, name='', indices=(), setter=False, call=False):
        """Get type attributes of a slot.

        Returns a TypeAttributes object, Ellipsis, or None.
        Raises KeyError if the slot is not recognized.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not isinstance(indices, tuple) or not all(isinstance(index, int) for index in indices):
            raise TypeError
        elif not isinstance(setter, bool):
            raise TypeError
        elif not isinstance(call, bool):
            raise TypeError
        elif setter and call:
            raise ValueError("Setter slot cannot be a call slot simultaneously")
        elif setter and not name and not indices:
            raise ValueError("Setter slot cannot be empty")

        try:
            attr = cls._slot_attr(name, indices, setter, call)
        except KeyError:
            if setter:
                slot_type = "setter"
            elif call:
                slot_type = "call"
            else:
                slot_type = "getter"

            raise KeyError(f"{cls}: {slot_type} slot {_slot_str(name, indices)} is not recognized")

        if not isinstance(attr, (type(None), type(...), TypeAttributes)):
            raise TypeError
        elif attr is None and not call:
            if setter:
                slot_type = "setter"
            else:
                slot_type = "getter"

            raise TypeError(f"{cls}: {slot_type} slot {_slot_str(name, indices)} must have a type but doesn't")

        return attr

    @classmethod
    def slot_object(cls, value, /, name='', indices=()):
        """Construct an Object of appropriate type for a value to be assigned to the specified slot.

        Returns an Object instance or None.
        Raises TypeError if an Object of appropriate type cannot be constructed.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not isinstance(indices, tuple) or not all(isinstance(index, int) for index in indices):
            raise TypeError
        elif not name and not indices:
            raise ValueError("Setter slot cannot be empty")

        try:
            obj = cls._slot_object(value, name, indices)
        except TypeError:
            raise TypeError(f"{cls}: cannot assign {value} to slot {_slot_str(name, indices)}")

        if not isinstance(obj, (type(None), Object)):
            raise TypeError

        return obj

    @classmethod
    def slot_unsettable(cls, name='', indices=()):
        """Check if a slot can be unset.

        Returns a boolean.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not isinstance(indices, tuple) or not all(isinstance(index, int) for index in indices):
            raise TypeError
        elif not name and not indices:
            raise ValueError("Cannot unset empty slot")

        return cls._slot_unsettable(name, indices)

    @classmethod
    def _init_params_class(cls):
        """Obtain initialization parameter list class.

        This method is to be overridden in derived classes.

        Returns a (sub)class of Parameters.
        """
        return Parameters # arbitrary parameters is recognized by default

    @classmethod
    def _call_params_class(cls, name, indices):
        """Obtain parameter list class of a call.

        This method is to be overridden in derived classes.

        Returns a (sub)class of Parameters.
        Raises KeyError if the call is not recognized.
        """
        return Parameters # any call with arbitrary parameters is recognized by default

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        """Get type attributes of a slot.

        This method is to be overridden in derived classes.

        Returns a TypeAttributes object, Ellipsis, or None.
        Raises KeyError if the slot is not recognized.
        """
        return ... # any slot is recognized by default

    @classmethod
    def _slot_object(cls, value, name, indices):
        """Construct an Object of appropriate type for a value to be assigned to the specified slot.

        This method is to be overridden in derived classes.

        Returns an Object instance or None.
        Raises TypeError if an Object of appropriate type cannot be constructed.
        """
        raise TypeError # don't know what to construct by default

    @classmethod
    def _slot_unsettable(cls, name, indices):
        """Check if a slot can be unset.

        This method is to be overridden in derived classes.

        Returns a boolean.
        """
        return True # all slots are unsettable by default

    @staticmethod
    def _set(context, slot_name, slot_indices, value):
        """Check compatibility of source and target types, append the assignment operation to the list.
        """
        if context._.registry is None:
            raise RuntimeError
        elif not slot_name and not slot_indices:
            raise AttributeError("Cannot set an empty slot")

        target_attr = type(context).slot_attr(slot_name, slot_indices, setter=True)
        source_attr = _get_entity_attr(value)

        if source_attr is None:
            value = type(context).slot_object(value, slot_name, slot_indices)
            source_attr = TypeAttributes.from_object(value)

        if target_attr is not ... and source_attr is not ... \
                and not source_attr.is_compatible_to(target_attr):
            raise TypeError("Cannot assign value={value} to slot {_slot_str(slot_name, slot_indices)} of context '{context._.key}': types are incompatible")

        context._.registry._set_slot(context._.key, slot_name, slot_indices, value)

    @staticmethod
    def _unset(context, slot_name, slot_indices):
        """Append the unassignment operation to the list.
        """
        if context._.registry is None:
            raise RuntimeError
        elif not slot_name and not slot_indices:
            raise AttributeError("Cannot unset an empty slot")

        if not type(context).slot_unsettable(slot_name, slot_indices):
            raise AttributeError(f"Slot {_slot_str(slot_name, slot_indices)} of context '{context._.key}' is not unsettable")

        context._.registry._unset_slot(context._.key, slot_name, slot_indices)


class _ContextSlot:
    """Representation of a context slot.

    Instances of this class are not to be created directly.
    """
    def __init__(self, context, name, indices, call_params=None, weak_ref=False):
        """Initialize a context slot instance.
        """
        fields = SimpleNamespace()

        fields.context = context
        fields.name = name
        fields.indices = indices
        fields.call_params = call_params
        fields.weak_ref = weak_ref

        object.__setattr__(self, '_', fields)

    def __str__(self):
        return f"_ContextSlot({self._})"

    def __getattr__(self, name):
        """Get a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        return _ContextSlot(self._.context, f'{self._.name}.{name}', self._.indices)

    def __getitem__(self, index):
        """Get a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        if isinstance(index, int):
            return _ContextSlot(self._.context, self._.name, self._.indices + (index,))

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            return _ContextSlot(self._.context, self._.name, self._.indices + index)

        else:
            raise TypeError

    def __setattr__(self, name, value):
        """Set a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        Context._set(self._.context, f'{self._.name}.{name}', self._.indices, value)

    def __setitem__(self, index, value):
        """Set a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        if isinstance(index, int):
            Context._set(self._.context, self._.name, self._.indices + (index,), value)

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            Context._set(self._.context, self._.name, self._.indices + index, value)

        else:
            raise TypeError

    def __delattr__(self, name):
        """Unset a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        Context._unset(self._.context, f'{self._.name}.{name}', self._.indices)

    def __delitem__(self, index, value):
        """Unset a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        if isinstance(index, int):
            Context._unset(self._.context, self._.name, self._.indices + (index,))

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            Context._unset(self._.context, self._.name, self._.indices + index)

        else:
            raise TypeError

    def __call__(self, _=None, /, **params):
        """Invoke a call.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError
        elif not isinstance(_, (type(None), Parameters.Context)):
            raise TypeError

        call_params_cls = type(self._.context).call_params_class(self._.name, self._.indices)

        return _ContextSlot(self._.context, self._.name, self._.indices,
                            call_params=call_params_cls(_, **params))


class _ContextSpec:
    """Representation of a created context specification.

    Instances of this class are not to be created directly.
    """
    def __init__(self, interface, params):
        """Initialize a context specification instance.
        """
        self._context_or_slot = interface._context_or_slot
        self._context_cls = interface._context_cls
        self._params = params

    def __str__(self):
        return f"_ContextSpec(interface={self._context_or_slot}, context_class={self._context_cls}, params={self._params})"

    def is_a(self, cls, /):
        """Refine a created context class.
        """
        if not issubclass(cls, self._context_cls):
            raise TypeError(f"Can't refine {self._context_cls} to {cls}: isn't a subclass")

        self._context_cls = cls
        return self


class _ContextInterface:
    """Representation of a context slot.

    Instances of this class are not to be created directly.
    """
    def __init__(self, context_or_slot, /, context_cls=Context):
        """Initialize a context interface instance.
        """
        self._context_or_slot = context_or_slot
        self._context_cls = context_cls

    def __str__(self):
        return f"_ContextInterface(origin={self._context_or_slot}, context_class={self._context_cls})"

    def __call__(self, _=None, /, **params):
        """Create a context specification instance.
        """
        return _ContextSpec(self, self._context_cls.init_params_class()(_, **params))

##############################################################################

class Parameters:
    """Representation of a parameter list.

    This base class does not impose any restrictions on parameters.
    """
    class Context(Context):
        """Base context type for parameter lists.

        This class is derived automatically in subclasses of Parameters class.
        """
        C_NAME = 'parameters'

        @classmethod
        def _call_params_class(cls, name, indices):
            if name or indices:
                raise KeyError

            return cls.init_params_class()

        @classmethod
        def _slot_attr(cls, name, indices, setter, call):
            if indices:
                raise KeyError

            if not name:
                if not call:
                    return TypeAttributes.complex_data(ARCHI_POINTER_DATA_TAG__KVLIST)
                else:
                    return None

            return cls.init_params_class().param_attr(name)

        @classmethod
        def _slot_object(cls, value, name, indices):
            if indices:
                raise KeyError

            return cls.init_params_class().param_object(value, name)

        @classmethod
        def _slot_unsettable(cls, name, indices):
            return False

    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        class ParametersSubclassContext(cls.Context):
            @classmethod
            def _init_params_class(_):
                return cls

        cls.Context = ParametersSubclassContext

    def __init__(self, _=None, /, **params):
        """Initialize a context parameter list instance.
        """
        if not isinstance(_, (type(None), type(self).Context)):
            raise TypeError

        self._params_base = _
        self._params_dynamic = {}
        self._params_static = {}

        for key, value in params.items():
            param_attr = type(self).param_attr(key)
            value_attr = _get_entity_attr(value)

            if value_attr is None:
                value = type(self).param_object(value, key)
                value_attr = TypeAttributes.from_object(value)

            if param_attr is not ... and value_attr is not ... \
                    and not value_attr.is_compatible_to(param_attr):
                raise TypeError("{cls}: cannot assign value={value} to parameter '{key}': types are incompatible")

            if isinstance(value, (Context, _ContextSlot)):
                self._params_dynamic[key] = value
            else: # Object or None
                self._params_static[key] = value

    def __str__(self):
        return f"Parameters(base={self.base_context}, dynamic={self.dynamic_params}, static={self.static_params})"

    @property
    def base_context(self):
        """Obtain the base parameter list context.
        """
        return self._params_base

    @property
    def base_context_key(self):
        """Obtain key of the base parameter list context.
        """
        return Context.key_of(self._params_base)

    @property
    def params(self):
        """Obtain dictionary of all parameters in the parameter list.
        """
        return MappingProxyType(self._params_dynamic | self._params_static)

    @property
    def dynamic_params(self):
        """Obtain dictionary of dynamic parameters in the parameter list.
        """
        return MappingProxyType(self._params_dynamic)

    @property
    def static_params(self):
        """Obtain dictionary of static parameters in the parameter list.
        """
        return MappingProxyType(self._params_static)

    @classmethod
    def param_attr(cls, key):
        """Get type attributes of a parameter.

        Returns a TypeAttributes object or Ellipsis.
        Raises KeyError if the parameter is not recognized.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not key:
            raise ValueError("Parameter key cannot be empty")

        try:
            attr = cls._param_attr(key)
        except KeyError:
            raise KeyError(f"{cls}: parameter '{key}' is not recognized")

        if not isinstance(attr, (type(...), TypeAttributes)):
            raise TypeError

        return attr

    @classmethod
    def param_object(cls, value, /, key):
        """Construct an Object of appropriate type for a value to be assigned to the specified parameter.

        Returns an Object instance or None.
        Raises TypeError if an Object of appropriate type cannot be constructed.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not key:
            raise ValueError("Parameter key cannot be empty")

        try:
            obj = cls._param_object(value, key)
        except KeyError:
            raise KeyError(f"{cls}: parameter '{key}' is not recognized")

        if not isinstance(obj, (type(None), Object)):
            raise TypeError

        return obj

    @classmethod
    def _param_attr(cls, key):
        """Get type attributes of a parameter.

        This method is to be overridden in derived classes.

        Returns a TypeAttributes object or Ellipsis.
        Raises KeyError if the parameter is not recognized.
        """
        return ... # any parameter is recognized by default

    @classmethod
    def _param_object(cls, value, key):
        """Construct an Object of appropriate type for a value to be assigned to the specified parameter.

        This method is to be overridden in derived classes.

        Returns an Object instance or None.
        Raises TypeError if an Object of appropriate type cannot be constructed.
        """
        raise TypeError # don't know what to construct by default

##############################################################################

class ParametersBase(Parameters):
    """Base class for custom parameter lists.
    """
    @classmethod
    def _param_attr(cls, key):
        raise KeyError # no parameters recognized by default


class ContextBase(Context):
    """Base class for custom context types.
    """
    InitParameters = ParametersBase

    @classmethod
    def _init_params_class(cls):
        return cls.InitParameters

    @classmethod
    def _call_params_class(cls, name, indices):
        raise KeyError # no calls supported by default

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        raise KeyError # no slots recognized by default

    @classmethod
    def _slot_unsettable(cls, name, indices):
        return False # all slots are unsettable by default

##############################################################################

class ContextWhitelist(ContextBase):
    """Context with whitelist of slots.
    """
    CONTEXT_TYPE = None
    GETTER_SLOTS = {}
    CALL_SLOTS = {}
    SETTER_SLOTS = {}

    @classmethod
    def _call_params_class(cls, name, indices):
        call = cls.CALL_SLOTS[name]

        if isinstance(call, dict):
            return call[len(indices)][1]
        elif not indices:
            return call[1]
        else:
            raise KeyError

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if setter:
            slot = cls.SETTER_SLOTS[name]
        elif call:
            slot = cls.CALL_SLOTS[name]
        elif name or indices:
            slot = cls.GETTER_SLOTS[name]
        else:
            slot = cls.CONTEXT_TYPE

        if isinstance(slot, dict):
            attr = slot[len(indices)]
        elif not indices:
            attr = slot
        else:
            raise KeyError

        if isinstance(attr, tuple):
            return attr[0]
        else:
            return attr

    @classmethod
    def _slot_object(cls, value, name, indices):
        slot = cls.SETTER_SLOTS[name]

        if isinstance(slot, dict):
            attr = slot[len(indices)]
        elif not indices:
            attr = slot
        else:
            raise KeyError

        if isinstance(attr, tuple):
            return attr[1](value) if value is not None else None
        else:
            raise TypeError


class ParametersWhitelist(ParametersBase):
    """Context with whitelist of parameters.
    """
    PARAMS = {}

    @classmethod
    def _param_attr(cls, key):
        attr = cls.PARAMS[key]

        if isinstance(attr, tuple):
            return attr[0]
        else:
            return attr

    @classmethod
    def _param_object(cls, value, key):
        attr = cls.PARAMS[key]

        if isinstance(attr, tuple):
            return attr[1](value) if value is not None else None
        else:
            raise TypeError

##############################################################################

_TYPE_DATA = TypeAttributes.complex_data()
_TYPE_DATA_PTR = TypeAttributes.from_type(c.c_void_p)
_TYPE_SIZE = (TypeAttributes.from_type(c.c_size_t), lambda value: PrimitiveData(c.c_size_t(value)))


class PointerContext(ContextWhitelist):
    """Arbitrary pointer.
    """
    C_NAME = 'pointer'

    CONTEXT_TYPE = ...

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': ...}

    GETTER_SLOTS = {'pointee': ...}
    SETTER_SLOTS = GETTER_SLOTS


class DataPointerArrayContext(ContextWhitelist):
    """Array of pointers to data.
    """
    C_NAME = 'dptr_array'

    CONTEXT_TYPE = _TYPE_DATA_PTR

    class InitParameters(ParametersWhitelist):
        PARAMS = {'length': _TYPE_SIZE}

    GETTER_SLOTS = {'': {1: _TYPE_DATA},
                    'ptr': {1: _TYPE_DATA_PTR},
                    'length': _TYPE_SIZE}

    SETTER_SLOTS = {'': {1: _TYPE_DATA},
                    'length': _TYPE_SIZE}

##############################################################################

class Registry:
    """Representation of a context registry.
    """
    # Input file contents key for lists of registry operations
    INPUT_FILE_KEY = 'reg_ops'

    # Keys of built-in contexts
    KEY_REGISTRY       = 'archi.registry'
    KEY_EXECUTABLE     = 'archi.executable'
    KEY_INPUT_FILE     = 'archi.input_file'
    KEY_SIGNAL_HANDLER = 'archi.signal_handler'

    def __init__(self):
        """Initialize a context registry instance.
        """
        self.reset()

    def __getitem__(self, key):
        """Obtain a context from the context registry.
        """
        if key is None:
            return None
        elif not isinstance(key, str):
            raise TypeError

        return self._contexts[key]

    def __setitem__(self, key, entity):
        """Create a context and insert it to the registry.
        """
        if key is not None:
            if not isinstance(key, str):
                raise TypeError
            elif key in self._contexts:
                raise KeyError(f"Context '{key}' is already in the registry")

            for typespec, method in type(self).ADD_METHOD.items():
                if isinstance(entity, typespec):
                    context = method(self, key, entity)
                    break
            else:
                raise TypeError("Can't create context from {entity}")

            if not isinstance(context, Context):
                raise TypeError

            context._.registry = self
            context._.key = key

            self._contexts[key] = context

        else:
            for typespec, method in type(self).EVAL_METHOD.items():
                if isinstance(entity, typespec):
                    if method is not None:
                        method(self, entity)
                    break
            else:
                raise TypeError("Can't evaluate {entity}")

    def __delitem__(self, key):
        """Remove a context from the registry.
        """
        if key is None:
            return
        elif not isinstance(key, str):
            raise TypeError
        elif key not in self._contexts:
            raise KeyError(f"Context '{key}' is not in the registry")

        context = self._contexts[key]
        context._.registry = None
        context._.key = None

        del self._contexts[key]
        if key in self._prerequisites:
            self._prerequisites.remove(key)

        self._delete_context(key)

    def __contains__(self, item):
        """Check if a context (key) is in the registry.
        """
        if item is None:
            return False
        elif isinstance(item, str):
            return item in self._contexts
        elif isinstance(item, Context):
            return Context.registry_of(item) is self \
                    and Context.key_of(item) in self._contexts \
                    and isinstance(self._contexts[Context.key_of(item)], type(item))
        else:
            raise TypeError

    def __iter__(self):
        """Return an iterator.
        """
        return iter(self._contexts)

    def contexts(self, cls=Context, /, is_prereq=True, is_new=True):
        """Obtain the dictionary of known contexts of the specified type.
        """
        if not issubclass(cls, Context):
            raise TypeError
        elif not is_prereq and not is_new:
            raise ValueError

        return {key: context for key, context in self._contexts.items() if isinstance(context, cls) \
                and ((is_prereq and key in self._prerequisites) \
                or (is_new and key not in self._prerequisites))}

    def change_key(self, new, old):
        """Replace key of a context.
        """
        if not isinstance(new, str) or not isinstance(old, str):
            raise TypeError

        self[new] = self[old]
        del self[old]

    def eval(self, entity, /):
        """Evaluate an entity (side effects only, not creating a context).
        """
        self[None] = entity

    def interface_of(self, item, /):
        """Create a representation of a context interface.
        """
        if isinstance(item, str):
            context = self[item]
            return _ContextInterface(context, type(context))
        elif isinstance(item, Context):
            if item not in self:
                raise KeyError(f"Context '{Context.key_of(item)}' is not in the registry")

            return _ContextInterface(item, type(item))
        else:
            raise TypeError

    def is_prerequisite(self, item, /):
        """Check if a context (key) was added to the registry by require_context().
        """
        if item not in self:
            return False

        if isinstance(item, str):
            return item in self._prerequisites
        elif isinstance(item, Context):
            return Context.key_of(item) in self._prerequisites
        else:
            raise TypeError

    def require_context(self, key, cls=Context):
        """Require a context with the specified key to exist in the registry.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not issubclass(cls, Context):
            raise TypeError

        if key in self._contexts:
            raise KeyError(f"Context '{key}' is already in the registry")

        context = cls()
        context._.registry = self
        context._.key = key

        self._contexts[key] = context
        self._prerequisites.add(key)

        return context

    def new_context(self, entity, /, key):
        """Create a new context and add it to the registry.
        """
        if key is None:
            raise KeyError

        self[key] = entity
        return self[key]

    def del_context(self, context, /):
        """Delete a context from the registry.
        """
        if context is None:
            return

        if not isinstance(context, Context):
            raise TypeError
        elif Context.registry_of(context) is not self:
            raise ValueError("Deleted context does not belong to the registry")

        del self[Context.key_of(context)]

    @contextmanager
    def deleted_context(self, key):
        """Obtain a context and delete it afterwards.
        """
        if key is None:
            raise KeyError

        context = self[key]

        try:
            yield context
        finally:
            del self[key]

    @contextmanager
    def temp_context(self, entity, /, key=None):
        """Create a temporary context.
        """
        if key is None:
            key = type(self).random_key()

        self[key] = entity

        with self.deleted_context(key) as context:
            yield context

    def operation_kvlist(self):
        """Get the current list of operations.
        """
        return self._operations

    def reset(self):
        """Clear the context registry instance state.
        """
        self._operations = []
        self._contexts = {}
        self._prerequisites = set()

    def _delete_context(self, key):
        """Append context deletion operation to the list.
        """
        from .object import RegistryOpData_delete

        self._operations.append(('delete', RegistryOpData_delete.construct(
            key=key)))

    def _alias_context(self, key, context):
        """Append context aliasing operation to the list.
        """
        from .object import RegistryOpData_alias

        self._operations.append(('alias', RegistryOpData_alias.construct(
            key=key,
            original_key=Context.key_of(context))))

        return type(context)()

    def _create_context(self, key, spec):
        """Append context creation operation(s) to the list.
        """
        from .object import RegistryOpData_create_as, RegistryOpData_create_from

        if isinstance(spec._context_or_slot, Context):
            with self._parameter_list(spec._params) as (params_context_key, params_list):
                self._operations.append(('create_as', RegistryOpData_create_as.construct(
                    key=key,
                    sample_key=Context.key_of(spec._context_or_slot),
                    init_params_context_key=params_context_key,
                    init_params_list=params_list)))

        elif isinstance(spec._context_or_slot, _ContextSlot):
            slot = spec._context_or_slot

            with self._parameter_list(spec._params) as (params_context_key, params_list):
                self._operations.append(('create_from', RegistryOpData_create_from.construct(
                    key=key,
                    source_key=Context.key_of(slot._.context),
                    source_slot_name=slot._.name,
                    source_slot_indices=slot._.indices,
                    init_params_context_key=params_context_key,
                    init_params_list=params_list)))

        else:
            raise TypeError

        return spec._context_cls()

    def _create_params(self, key, params):
        """Append parameter list creation operation(s) to the list.
        """
        with self._parameter_list(params) as (params_context_key, params_list):
            self._create_parameter_list(key, params_context_key, params_list)

        return type(params).Context()

    def _create_ptr(self, key, entity):
        """Append pointer creation operation(s) to the list.
        """
        from .object import RegistryOpData_create_ptr

        if isinstance(entity, (type(None), Object)):
            self._operations.append(('create_ptr', RegistryOpData_create_ptr.construct(
                key=key,
                pointee=entity)))

        elif isinstance(entity, _ContextSlot):
            self._operations.append(('create_ptr', RegistryOpData_create_ptr.construct(
                key=key,
                pointee=None)))

            self._set_slot(key, 'pointee', (), entity)

        else:
            raise TypeError

        return PointerContext()

    def _create_dptr_array(self, key, seq):
        """Append data pointer array create operation(s) to the list.
        """
        from .object import RegistryOpData_create_dptr_array

        self._operations.append(('create_dptr_array', RegistryOpData_create_dptr_array.construct(
            key=key,
            length=len(seq))))

        for index, entity in enumerate(seq):
            if entity is not None:
                self._set_slot(key, '', (index,), entity)

        return DataPointerArrayContext()

    def _unset_slot(self, context_key, slot_name, slot_indices):
        """Append slot unassignment operation to the list.
        """
        from .object import RegistryOpData_unassign

        self._operations.append(('unassign', RegistryOpData_unassign.construct(
            key=context_key,
            slot_name=slot_name,
            slot_indices=slot_indices)))

    def _set_slot(self, context_key, slot_name, slot_indices, value):
        """Append slot assignment operation to the list.
        """
        from .object import RegistryOpData_assign, RegistryOpData_assign_slot, RegistryOpData_assign_call

        if isinstance(value, (type(None), Object)):
            self._operations.append(('assign', RegistryOpData_assign.construct(
                key=context_key,
                slot_name=slot_name,
                slot_indices=slot_indices,
                value=value)))

        elif isinstance(value, Context):
            self._operations.append(('assign_slot', RegistryOpData_assign_slot.construct(
                key=context_key,
                slot_name=slot_name,
                slot_indices=slot_indices,
                source_key=Context.key_of(value),
                source_slot_name='',
                source_slot_indices=())))

        elif isinstance(value, _ContextSlot):
            if value._.call_params is None:
                self._operations.append(
                        ('assign_slot' if not value._.weak_ref else 'assign_slot_weak',
                         RegistryOpData_assign_slot.construct(
                             key=context_key,
                             slot_name=slot_name,
                             slot_indices=slot_indices,
                             source_key=Context.key_of(value._.context),
                             source_slot_name=value._.name,
                             source_slot_indices=value._.indices)))

            else:
                with self._parameter_list(value._.call_params) as (params_context_key, params_list):
                    self._operations.append(
                            ('assign_call' if not value._.weak_ref else 'assign_call_weak',
                             RegistryOpData_assign_call.construct(
                                 key=context_key,
                                 slot_name=slot_name,
                                 slot_indices=slot_indices,
                                 source_key=Context.key_of(value._.context),
                                 source_slot_name=value._.name,
                                 source_slot_indices=value._.indices,
                                 source_call_params_context_key=params_context_key,
                                 source_call_params_list=params_list)))

        else:
            raise TypeError

    def _call_slot(self, slot):
        """Append context call invokation operation to the list.
        """
        from .object import RegistryOpData_invoke

        if slot._.call_params is None:
            raise AttributeError(f"Slot {_slot_str(slot._.name, slot._.indices)} of context '{Context.key_of(slot._.context)}' is not called")

        with self._parameter_list(slot._.call_params) as (params_context_key, params_list):
            self._operations.append(('invoke', RegistryOpData_invoke.construct(
                key=Context.key_of(slot._.context),
                slot_name=slot._.name,
                slot_indices=slot._.indices,
                call_params_context_key=params_context_key,
                call_params_list=params_list)))

    def _create_parameter_list(self, key, params_context_key, params_list):
        """Append parameter list context creation operation to the list.
        """
        from .object import RegistryOpData_create_params

        self._operations.append(('create_params', RegistryOpData_create_params.construct(
            key=key,
            params_context_key=params_context_key,
            params_list=params_list)))

    @contextmanager
    def _parameter_list(self, params):
        """Prepare the temporary parameter list if needed and append necessary
        list forming operations.
        """
        if not params.dynamic_params:
            yield (params.base_context_key, params.static_params)
        else:
            temp_context_key = type(self).random_key(prefix='params')

            self._create_parameter_list(temp_context_key, params.base_context_key, params.static_params)

            try:
                for name, value in params.dynamic_params.items():
                    self._set_slot(temp_context_key, name, (), value)

                yield (temp_context_key, {})
            finally:
                self._delete_context(temp_context_key)

    @staticmethod
    def random_key(prefix='context', postfix_len=4):
        """Generate a random context key with the specified prefix.
        """
        import random
        import string

        postfix = ''.join(random.choice(string.ascii_letters + string.digits) \
                for _ in range(postfix_len))

        return f'.{prefix}_{postfix}'

# Map of context creation methods (entity type(s) -> method)
Registry.ADD_METHOD = {Context: Registry._alias_context,
                       _ContextSpec: Registry._create_context,
                       Parameters: Registry._create_params,
                       (type(None), Object, _ContextSlot): Registry._create_ptr,
                       (tuple, list): Registry._create_dptr_array}

# Map of entity evaluation methods (entity type(s) -> method or None)
Registry.EVAL_METHOD = {_ContextSlot: Registry._call_slot}

##############################################################################

def _get_entity_attr(entity, /):
    """Get type attributes of an entity.
    """
    if entity is None:
        return TypeAttributes()

    elif isinstance(entity, Context):
        return type(entity).slot_attr()

    elif isinstance(entity, _ContextSlot):
        attr = type(entity._.context).slot_attr(
                entity._.name, entity._.indices, call=entity._.call_params is not None)
        if attr is None:
            raise TypeError(f"Call slot {_slot_str(entity._.name, entity._.indices)} of context '{entity._.context._.key}' doesn't return a value")

        return attr

    elif isinstance(entity, Object):
        return TypeAttributes.from_object(entity)

    else:
        return None # unknown entity type


def _slot_str(slot_name='', slot_indices=()):
    """Get the string representation of a slot.
    """
    if slot_name:
        str_name = "'" + slot_name + "'"
    else:
        str_name = ""

    if slot_indices:
        str_indices = "[" + ", ".join(str(index) for index in slot_indices) + "]"
    else:
        str_indices = ""

    return str_name + str_indices

