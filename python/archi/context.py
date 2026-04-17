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

import ctypes as c
from contextlib import contextmanager
from types import MappingProxyType, SimpleNamespace

import archi.ctypes as ac
from .object import Object, PrimitiveData, String


TypeAttributes = ac.archi_pointer_attr_t


def type_attributes_of(entity, /):
    """Get type attributes of an entity.
    """
    if entity is None:
        return ... # nothingness can be of any type

    elif isinstance(entity, Context):
        return type(entity).slot_attr()

    elif isinstance(entity, _ContextSlot):
        attr = type(entity._.context).slot_attr(
                entity._.name, entity._.indices, call=entity._.call_params is not None)
        if attr is None:
            raise RuntimeError(f"Call slot {_slot_str(entity._.name, entity._.indices)} of context '{entity._.context._.key}' doesn't return a value")

        return attr

    elif isinstance(entity, Object):
        return entity.attributes

    else:
        return None # unknown entity type


def type_attributes_compatible(attr1, attr2, /):
    """Check compatibility of type attributes.
    """
    if not isinstance(attr1, (type(...), TypeAttributes)):
        raise TypeError
    elif not isinstance(attr2, (type(...), TypeAttributes)):
        raise TypeError

    if attr1 is ... or attr2 is ...:
        return True
    else:
        return attr1.is_compatible_to(attr2)

##############################################################################

class Context:
    """Representation of a context.

    This base class does not impose any restrictions on slots.
    """
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
        """Create a weak reference slot.
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

            attr = type_attributes_of(context_or_slot)

            if not type_attributes_compatible(
                    attr, TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE)):
                raise TypeError(f"{context_or_slot} is not a context interface")

            if isinstance(context_or_slot, Context):
                return _ContextInterface(_ContextSlot(context_or_slot, '', ()), cls)

            elif isinstance(context_or_slot, _ContextSlot):
                return _ContextInterface(context_or_slot, cls)

        elif library is not None:
            slot = ContextInterfaceSymbol.slot(cls.C_NAME, library=library)
            slot = _ContextSlot(slot._.context, slot._.name, ()) # optimization to avoid creation of a temporary context
            return _ContextInterface(slot, cls)

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
        source_attr = type_attributes_of(value)

        if source_attr is None:
            value = type(context).slot_object(value, slot_name, slot_indices)
            source_attr = value.attributes

        if not type_attributes_compatible(source_attr, target_attr):
            raise TypeError(f"Cannot assign value={value} to slot {_slot_str(slot_name, slot_indices)} of context '{context._.key}': types are incompatible")

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
                    return TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__KVLIST)
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
            value_attr = type_attributes_of(value)

            if value_attr is None:
                value = type(self).param_object(value, key)
                value_attr = value.attributes

            if not type_attributes_compatible(value_attr, param_attr):
                raise TypeError(f"{type(self)}: cannot assign value={value} to parameter '{key}': types are incompatible")

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

    def __init__(self, require_builtins=True, protect_builtins=True):
        """Initialize a context registry instance.
        """
        if not isinstance(require_builtins, bool):
            raise TypeError
        elif not isinstance(protect_builtins, bool):
            raise TypeError

        self.reset()

        if require_builtins:
            # Require built-in contexts
            self.require_context(Registry.KEY_REGISTRY, HashmapContext, protect=protect_builtins)
            self.require_context(Registry.KEY_EXECUTABLE, LibraryContext, protect=protect_builtins)
            self.require_context(Registry.KEY_INPUT_FILE, FileContext, protect=protect_builtins)
            self.require_context(Registry.KEY_SIGNAL_HANDLER, SignalHandlerDataHashmapContext, protect=protect_builtins)

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
        if key is None:
            raise KeyError("Cannot create a context without a key")
        elif not isinstance(key, str):
            raise TypeError
        elif key in self._contexts:
            raise KeyError(f"Context '{key}' is already in the registry")

        for typespec, method in type(self).ADD_METHOD.items():
            if isinstance(entity, typespec):
                context = method(self, key, entity)
                break
        else:
            raise TypeError(f"Can't create context from {entity}")

        if not isinstance(context, Context):
            raise TypeError

        context._.registry = self
        context._.key = key

        self._contexts[key] = context

    def __delitem__(self, key):
        """Remove a context from the registry.
        """
        if key is None:
            return
        elif not isinstance(key, str):
            raise TypeError
        elif key not in self._contexts:
            raise KeyError(f"Context '{key}' is not in the registry")
        elif self._prerequisites.get(key, False):
            raise KeyError(f"Prerequisite context '{key}' is protected from deletion")

        context = self._contexts[key]
        context._.registry = None
        context._.key = None

        del self._contexts[key]
        if key in self._prerequisites:
            del self._prerequisites[key]

        self._delete_context(key)

    def __call__(self, entity, /):
        """Invoke an entity (causing side effects only, context not created).
        """
        for typespec, method in type(self).INVOKE_METHOD.items():
            if isinstance(entity, typespec):
                if method is not None:
                    method(self, entity)
                break
        else:
            raise TypeError(f"Can't invoke {entity}")

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
        elif self._prerequisites.get(old, False):
            raise KeyError(f"Prerequisite context '{old}' is protected from key changing")

        self[new] = self[old]
        del self[old]

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

    def require_context(self, key, cls=Context, protect=True):
        """Require a context with the specified key to exist in the registry.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not issubclass(cls, Context):
            raise TypeError
        elif not isinstance(protect, bool):
            raise TypeError

        if key not in self._contexts:
            context = cls()
            context._.registry = self
            context._.key = key

            self._contexts[key] = context
            self._prerequisites[key] = protect

        else:
            context = self._contexts[key]

            if not isinstance(context, cls):
                raise TypeError(f"Required context '{key}' has type {type(context)} (want {cls})")

        return context

    def new_context(self, entity, /, key):
        """Create a new context and add it to the registry.
        """
        self[key] = entity
        return self[key]

    def del_context(self, context, /):
        """Delete a context from the registry.
        """
        if context is None:
            return

        if not isinstance(context, Context):
            raise TypeError
        elif context not in self:
            raise ValueError("Deleted context does not belong to the registry")

        del self[Context.key_of(context)]

    @contextmanager
    def deleted_context(self, key):
        """Obtain a context and delete it afterwards.
        """
        context = self[key]

        try:
            yield context
        finally:
            del self[key]

    @contextmanager
    def temp_context(self, entity, /, key):
        """Create a temporary context.
        """
        self[key] = entity

        with self.deleted_context(key) as context:
            yield context

    def operations(self):
        """Get the current list of operations.
        """
        return self._operations

    def reset(self):
        """Clear the context registry instance state.
        """
        self._operations = []
        self._contexts = {}
        self._prerequisites = {}

    def temp_key(self, prefix, rnd_len=4):
        """Generate a temporary context key with the specified prefix.
        """
        import random
        import string

        postfix = ''.join(random.choice(string.ascii_letters + string.digits) \
                for _ in range(rnd_len))

        return f'.{prefix}_{postfix}:{len(self._operations)}'

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
                if slot._.call_params is None:
                    temp_context_key = None

                    source_key = Context.key_of(slot._.context)
                    source_slot_name = slot._.name
                    source_slot_indices = slot._.indices
                else:
                    temp_context_key = self.temp_key(prefix='context_interface')

                    self._create_ptr(temp_context_key, slot)

                    source_key = temp_context_key
                    source_slot_name = ''
                    source_slot_indices = ()

                self._operations.append(('create_from', RegistryOpData_create_from.construct(
                    key=key,
                    source_key=source_key,
                    source_slot_name=source_slot_name,
                    source_slot_indices=source_slot_indices,
                    init_params_context_key=params_context_key,
                    init_params_list=params_list)))

                if temp_context_key is not None:
                    self._delete_context(temp_context_key)

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
            temp_context_key = self.temp_key(prefix='params')

            self._create_parameter_list(temp_context_key, params.base_context_key, params.static_params)

            try:
                for name, value in params.dynamic_params.items():
                    self._set_slot(temp_context_key, name, (), value)

                yield (temp_context_key, {})
            finally:
                self._delete_context(temp_context_key)

# Map of context creation methods (entity type(s) -> method)
Registry.ADD_METHOD = {Context: Registry._alias_context,
                       _ContextSpec: Registry._create_context,
                       Parameters: Registry._create_params,
                       (type(None), Object, _ContextSlot): Registry._create_ptr,
                       (tuple, list): Registry._create_dptr_array}

# Map of entity invokation methods (entity type(s) -> method or None)
Registry.INVOKE_METHOD = {_ContextSlot: Registry._call_slot}

##############################################################################
# Built-in context types
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
_TYPE_FUNCTION = TypeAttributes.function()
_TYPE_BOOL = (TypeAttributes.from_type(c.c_char), lambda value: PrimitiveData(c.c_char(bool(value))))
_TYPE_INT = (TypeAttributes.from_type(c.c_int), lambda value: PrimitiveData(c.c_int(value)))
_TYPE_LONGLONG = (TypeAttributes.from_type(c.c_longlong), lambda value: PrimitiveData(c.c_longlong(value)))
_TYPE_SIZE = (TypeAttributes.from_type(c.c_size_t), lambda value: PrimitiveData(c.c_size_t(value)))
_TYPE_ATTR = (TypeAttributes.from_type(TypeAttributes), lambda value: PrimitiveData(TypeAttributes(value)))
_TYPE_DATA_PTR = TypeAttributes.from_type(c.c_void_p)
_TYPE_STRING = (TypeAttributes.from_type(c.c_char * 1), lambda value: String(value))

### archi/context ###

class PointerContext(ContextWhitelist):
    """Arbitrary pointer.
    """
    C_NAME = 'pointer'

    CONTEXT_TYPE = ...

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': ...}

    GETTER_SLOTS = {'pointee': ...}
    SETTER_SLOTS = GETTER_SLOTS


class DataPointerContext(ContextWhitelist):
    """Pointer to data.
    """
    C_NAME = 'dpointer'

    CONTEXT_TYPE = _TYPE_DATA

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': _TYPE_DATA,
                  'writable': _TYPE_BOOL}

    GETTER_SLOTS = InitParameters.PARAMS
    SETTER_SLOTS = GETTER_SLOTS


class PrimitiveDataPointerContext(ContextWhitelist):
    """Pointer to primitive data.
    """
    C_NAME = 'pdpointer'

    CONTEXT_TYPE = _TYPE_DATA

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': _TYPE_DATA,
                  'offset': _TYPE_LONGLONG,
                  'offset_unit': _TYPE_SIZE,
                  'writable': _TYPE_BOOL,
                  'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE}

    class ShiftPtrCallParameters(ParametersWhitelist):
        PARAMS = {'offset': _TYPE_LONGLONG}

    class SetAttrCallParameters(ParametersWhitelist):
        PARAMS = {'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE}

    class CopyCallParameters(ParametersWhitelist):
        PARAMS = {'src': _TYPE_DATA,
                  'src_offset': _TYPE_SIZE,
                  'offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    class FillCallParameters(ParametersWhitelist):
        PARAMS = {'pattern': _TYPE_DATA,
                  'offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    GETTER_SLOTS = {'pointee': _TYPE_DATA,
                    '': {1: _TYPE_DATA},
                    'writable': _TYPE_BOOL,
                    'length': _TYPE_SIZE,
                    'stride': _TYPE_SIZE,
                    'size': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE}

    CALL_SLOTS = {'shift_ptr': (None, ShiftPtrCallParameters),
                  'set_attr': (None, SetAttrCallParameters),
                  'copy': (None, CopyCallParameters),
                  'fill': (None, FillCallParameters)}

    SETTER_SLOTS = {'pointee': _TYPE_DATA,
                    'writable': _TYPE_BOOL,
                    'length': _TYPE_SIZE,
                    'stride': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE}


class ComplexDataPointerContext(ContextWhitelist):
    """Pointer to complex data.
    """
    C_NAME = 'cdpointer'

    CONTEXT_TYPE = _TYPE_DATA

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': _TYPE_DATA,
                  'offset': _TYPE_LONGLONG,
                  'offset_unit': _TYPE_SIZE,
                  'writable': _TYPE_BOOL,
                  'tag': _TYPE_ATTR}

    GETTER_SLOTS = {'pointee': _TYPE_DATA,
                    'writable': _TYPE_BOOL,
                    'tag': _TYPE_ATTR}

    SETTER_SLOTS = {'pointee': _TYPE_DATA,
                    'writable': _TYPE_BOOL,
                    'tag': _TYPE_ATTR}


class FunctionPointerContext(ContextWhitelist):
    """Pointer to function.
    """
    C_NAME = 'fpointer'

    CONTEXT_TYPE = _TYPE_FUNCTION

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': _TYPE_FUNCTION,
                  'tag': _TYPE_ATTR}

    GETTER_SLOTS = {'pointee': _TYPE_FUNCTION,
                    'tag': _TYPE_ATTR}

    SETTER_SLOTS = {'pointee': _TYPE_FUNCTION,
                    'tag': _TYPE_ATTR}


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

### archi/aggr ###

class AggregateContext(ContextWhitelist):
    """Aggregate object.
    """
    C_NAME = 'aggregate'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__AGGR)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'interface': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE),
                  'metadata': _TYPE_DATA,
                  'fam_length': _TYPE_SIZE}

    GETTER_SLOTS = {'interface': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE),
                    'metadata': _TYPE_DATA,
                    'layout': TypeAttributes.from_type(ac.archi_layout_struct_t),
                    'size': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE,
                    'fam_stride': _TYPE_SIZE,
                    'fam_length': _TYPE_SIZE,
                    'full_size': _TYPE_SIZE,
                    'object': _TYPE_DATA}

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if not call:
            if name.startswith('member.'):
                return _TYPE_DATA
            elif name.startswith('ref.') and not setter:
                return ...

        return super()._slot_attr(name, indices, setter, call)

### archi/exec ###

class DexgraphNodeContext(ContextWhitelist):
    """Directed execution graph node.
    """
    C_NAME = 'dexgraph_node'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'name': _TYPE_STRING,
                  'sequence_length': _TYPE_SIZE,
                  'transition_func': TypeAttributes.function(ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                  'transition_data': _TYPE_DATA,
                  'branches': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}

    class ExecuteCallParameters(ParametersWhitelist):
        PARAMS = {'index': (TypeAttributes.from_type(ac.archi_dexgraph_branch_index_t),
                            lambda value: PrimitiveData(ac.archi_dexgraph_branch_index_t(value)))}

    GETTER_SLOTS = {'name': _TYPE_STRING,
                    'sequence.length': _TYPE_SIZE,
                    'sequence.function': {1: TypeAttributes.function(ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)},
                    'sequence.data': {1: _TYPE_DATA},
                    'transition.function': TypeAttributes.function(ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                    'transition.data': _TYPE_DATA,
                    'branches': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}

    CALL_SLOTS = {'execute': (None, ExecuteCallParameters)}

    SETTER_SLOTS = {'sequence.function': {1: TypeAttributes.function(ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)},
                    'sequence.data': {1: _TYPE_DATA},
                    'transition.function': TypeAttributes.function(ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                    'transition.data': _TYPE_DATA,
                    'branches': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}


class DexgraphNodeArrayContext(ContextWhitelist):
    """Directed execution graph node array.
    """
    C_NAME = 'dexgraph_node_array'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'num_nodes': _TYPE_SIZE}

    GETTER_SLOTS = {'num_nodes': _TYPE_SIZE,
                    'node': {1: TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)}}

    SETTER_SLOTS = {'node': {1: TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)}}

### archi/thread ###

class ThreadGroupContext(ContextWhitelist):
    """Thread group.
    """
    C_NAME = 'thread_group'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__THREAD_GROUP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttributes.from_type(ac.archi_thread_group_start_params_t),
                             lambda value: PrimitiveData(value)),
                  'num_threads': _TYPE_SIZE}

    GETTER_SLOTS = {'num_threads': _TYPE_SIZE}


class LockFreeQueueContext(ContextWhitelist):
    """Lock-free queue.
    """
    C_NAME = 'thread_lfqueue'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__THREAD_LFQUEUE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttributes.from_type(ac.archi_thread_lfqueue_alloc_params_t),
                             lambda value: PrimitiveData(value)),
                  'capacity': _TYPE_SIZE,
                  'elt_size': _TYPE_SIZE}

    GETTER_SLOTS = {'capacity': _TYPE_SIZE,
                    'elt_size': _TYPE_SIZE}

### archi/signal ###

class SignalHandlerDataHashmapContext(ContextBase):
    """Data for the signal meta-handler for multiple handler support.
    """
    C_NAME = 'signal_handler_data__hashmap'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__SIGNAL_HANDLER_DATA__HASHMAP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttributes.from_type(ac.archi_hashmap_alloc_params_t),
                             lambda value: PrimitiveData(value)),
                  'capacity': _TYPE_SIZE}

    def _slot_attr(cls, name, indices, setter, call):
        if call:
            raise KeyError
        elif indices:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE

        return TypeAttributes.from_type(ac.archi_signal_handler_t)

    @classmethod
    def _slot_unsettable(cls, name, indices):
        return True

### archi/memory ###

class MemoryContext(ContextWhitelist):
    """Memory handle.
    """
    C_NAME = 'memory'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__MEMORY)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'interface': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE),
                  'alloc_data': _TYPE_DATA,
                  'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE,
                  'ext_alignment': _TYPE_SIZE}

    GETTER_SLOTS = {'interface': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE),
                    'allocation': _TYPE_DATA,
                    'length': _TYPE_SIZE,
                    'stride': _TYPE_SIZE,
                    'size': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE,
                    'ext_alignment': _TYPE_SIZE}


class MemoryMappingContext(ContextWhitelist):
    """Memory mapping.
    """
    C_NAME = 'memory_mapping'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__MEMORY_MAPPING)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'memory': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__MEMORY),
                  'map_data': _TYPE_DATA,
                  'offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    GETTER_SLOTS = {'memory': TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__MEMORY),
                    'ptr': _TYPE_DATA,
                    'offset': _TYPE_SIZE,
                    'length': _TYPE_SIZE,
                    'size': _TYPE_SIZE}

### archi/file ###

class FileContext(ContextWhitelist):
    """File handle.
    """
    C_NAME = 'file'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__FILE_STREAM)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pathname': _TYPE_STRING,
                  'fd': (TypeAttributes.from_type(ac.archi_file_descriptor_t),
                         lambda value: PrimitiveData(ac.archi_file_descriptor_t(value))),
                  'params': TypeAttributes.from_type(ac.archi_file_open_params_t),
                  'size': _TYPE_SIZE,
                  'readable': _TYPE_BOOL,
                  'writable': _TYPE_BOOL,
                  'create': _TYPE_BOOL,
                  'exclusive': _TYPE_BOOL,
                  'truncate': _TYPE_BOOL,
                  'append': _TYPE_BOOL,
                  'flags': _TYPE_INT,
                  'mode': _TYPE_INT,
                  'stream': _TYPE_BOOL}

    class ReadCallParameters(ParametersWhitelist):
        PARAMS = {'dest': _TYPE_DATA,
                  'dest_offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    class WriteCallParameters(ParametersWhitelist):
        PARAMS = {'src': _TYPE_DATA,
                  'src_offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    class SyncCallParameters(ParametersWhitelist):
        PARAMS = {}

    GETTER_SLOTS = {'fd': (TypeAttributes.from_type(ac.archi_file_descriptor_t),
                           lambda value: PrimitiveData(ac.archi_file_descriptor_t(value))),
                    'offset': _TYPE_LONGLONG}

    CALL_SLOTS = {'read': (None, ReadCallParameters),
                  'write': (None, WriteCallParameters),
                  'sync': (None, SyncCallParameters)}

    SETTER_SLOTS = {'offset': _TYPE_LONGLONG,
                    'offset.end': _TYPE_LONGLONG,
                    'offset.shift': _TYPE_LONGLONG}


class FileMappingContext(ContextWhitelist):
    """File mapping.
    """
    C_NAME = 'file_mapping'

    CONTEXT_TYPE = TypeAttributes.complex_data()

    class InitParameters(ParametersWhitelist):
        PARAMS = {'fd': (TypeAttributes.from_type(ac.archi_file_descriptor_t),
                         lambda value: PrimitiveData(ac.archi_file_descriptor_t(value))),
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE,
                  'params': TypeAttributes.from_type(ac.archi_file_map_params_t),
                  'size': _TYPE_SIZE,
                  'offset': _TYPE_SIZE,
                  'ptr_support': _TYPE_BOOL,
                  'readable': _TYPE_BOOL,
                  'writable': _TYPE_BOOL,
                  'shared': _TYPE_BOOL,
                  'flags': _TYPE_INT}

    GETTER_SLOTS = {'length': _TYPE_SIZE,
                    'stride': _TYPE_SIZE,
                    'size': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE}

### archi/library ###

class LibraryContext(ContextBase):
    """Shared library.
    """
    C_NAME = 'library'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__LIBRARY_HANDLE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pathname': _TYPE_STRING,
                  'params': TypeAttributes.from_type(ac.archi_library_load_params_t),
                  'lazy': _TYPE_BOOL,
                  'global': _TYPE_BOOL,
                  'flags': _TYPE_INT}

    class DataSymbolCallParameters(ParametersWhitelist):
        PARAMS = {'tag': _TYPE_ATTR,
                  'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE}

    class FunctionSymbolCallParameters(ParametersWhitelist):
        PARAMS = {'tag': _TYPE_ATTR}

    @classmethod
    def _call_params_class(cls, name, indices):
        if indices:
            raise KeyError

        if name.startswith('data.'):
            return cls.DataSymbolCallParameters
        elif name.startswith('function.'):
            return cls.FunctionSymbolCallParameters
        else:
            raise KeyError

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if indices or setter:
            raise KeyError

        if not name:
            if not call:
                return cls.CONTEXT_TYPE
            else:
                raise KeyError

        if name.startswith('data.'):
            return _TYPE_DATA
        elif name.startswith('function.'):
            return _TYPE_FUNCTION
        else:
            raise KeyError

### archi/hashmap ###

class HashmapContext(ContextBase):
    """Hashmap.
    """
    C_NAME = 'hashmap'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__HASHMAP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': TypeAttributes.from_type(ac.archi_hashmap_alloc_params_t),
                  'capacity': _TYPE_SIZE}

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if indices or call:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE

        return ...

    @classmethod
    def _slot_unsettable(cls, name, indices):
        return True

### archi/env ###

class EnvVariableContext(ContextWhitelist):
    """Environment variable.
    """
    C_NAME = 'env_variable'

    CONTEXT_TYPE = _TYPE_STRING

    class InitParameters(ParametersWhitelist):
        PARAMS = {'default_value': _TYPE_STRING}

    SETTER_SLOTS = {'default_value': _TYPE_STRING}

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if not setter and not call:
            if indices:
                raise KeyError

            return _TYPE_STRING[0]
        else:
            return super()._slot_attr(name, indices, setter, call)

### archi/parser ###

class NumberParserContext(ContextWhitelist):
    """Number parser.
    """
    C_NAME = 'number_parser'

    CONTEXT_TYPE = _TYPE_DATA

    class InitParameters(ParametersWhitelist):
        PARAMS = {'base': _TYPE_INT,
                  'unsigned_char': _TYPE_STRING,
                  'unsigned_short': _TYPE_STRING,
                  'unsigned_int': _TYPE_STRING,
                  'unsigned_long': _TYPE_STRING,
                  'unsigned_long_long': _TYPE_STRING,
                  'signed_char': _TYPE_STRING,
                  'signed_short': _TYPE_STRING,
                  'signed_int': _TYPE_STRING,
                  'signed_long': _TYPE_STRING,
                  'signed_long_long': _TYPE_STRING,
                  'float': _TYPE_STRING,
                  'double': _TYPE_STRING,
                  'long_double': _TYPE_STRING,
                  'size_t': _TYPE_STRING,
                  'uint8_t': _TYPE_STRING,
                  'uint16_t': _TYPE_STRING,
                  'uint32_t': _TYPE_STRING,
                  'uint64_t': _TYPE_STRING,
                  'int8_t': _TYPE_STRING,
                  'int16_t': _TYPE_STRING,
                  'int32_t': _TYPE_STRING,
                  'int64_t': _TYPE_STRING}

### archi/timer ###

class TimerContext(ContextWhitelist):
    """Timer.
    """
    C_NAME = 'timer'

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__TIMER)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'name': _TYPE_STRING}

    class ResetCallParameters(ParametersWhitelist):
        PARAMS = {}

    CALL_SLOTS = {'reset': (None, ResetCallParameters)}

##############################################################################
# Built-in symbol types
##############################################################################

class _Symbol:
    """Representation of an arbitrary symbol.
    """
    PREFIX = ''
    POSTFIX = ''
    TAG = 0

    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        if not isinstance(cls.PREFIX, str):
            raise TypeError
        elif not isinstance(cls.POSTFIX, str):
            raise TypeError
        elif not isinstance(cls.TAG, int):
            raise TypeError

        if cls.TAG < 0:
            raise ValueError

    @classmethod
    def attributes(cls):
        """Get type attributes.
        """
        raise NotImplementedError

    @classmethod
    def slot(cls, name, /, library):
        """Obtain the correctly typed symbol slot from the specified library.
        """
        if not isinstance(library, LibraryContext):
            raise TypeError
        elif not isinstance(name, str):
            raise TypeError
        elif not name:
            raise ValueError

        return getattr(cls._symbol_type(library), f'{cls.PREFIX}{name}{cls.POSTFIX}')(tag=cls.TAG)

    @classmethod
    def _symbol_type(cls, library):
        raise NotImplementedError

    def __init__(self, name, /):
        """Initialize a symbol representation instance.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not name:
            raise ValueError

        self._name = name

    @property
    def name(self):
        """Get symbol name.
        """
        return self._name

    def slot_of(self, library):
        """Obtain the correctly typed symbol slot from the specified library.
        """
        return type(self).slot(self._name, library=library)


class DataSymbol(_Symbol):
    """Representation of an arbitrary data symbol.
    """
    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        super().__init_subclass__()

        if cls.TAG > TypeAttributes.DATA_TAG_MAX:
            raise ValueError

    @classmethod
    def attributes(cls):
        """Get type attributes.
        """
        return TypeAttributes.complex_data(cls.TAG)

    @classmethod
    def _symbol_type(cls, library):
        return library.data


class FunctionSymbol(_Symbol):
    """Representation of an arbitrary function symbol.
    """
    def __init_subclass__(cls):
        """Initialize a subclass.
        """
        super().__init_subclass__()

        if cls.TAG > TypeAttributes.FUNC_TAG_MAX:
            raise ValueError

    @classmethod
    def attributes(cls):
        """Get type attributes.
        """
        return TypeAttributes.function(cls.TAG)

    @classmethod
    def _symbol_type(cls, library):
        return library.function

### archi/context ###

class ContextInterfaceSymbol(DataSymbol):
    """Context interface symbol.
    """
    PREFIX = 'archi_context_interface__'
    TAG = ac.ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE

### archi/aggr ###

class AggregateInterfaceSymbol(DataSymbol):
    """Aggregate type interface symbol.
    """
    PREFIX = 'archi_aggr_interface__'
    TAG = ac.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE


class AggregateTypeSymbol(DataSymbol):
    """Aggregate type description symbol.
    """
    PREFIX = 'archi_aggr_type__'
    TAG = ac.ARCHI_POINTER_DATA_TAG__AGGR_TYPE

### archi/exec ###

class DexgraphOperationFuncSymbol(FunctionSymbol):
    """DEG operation function symbol.
    """
    PREFIX = 'archi_dexgraph_op__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION


class DexgraphOperationDataSymbol(DataSymbol):
    """DEG operation function data symbol.
    """
    PREFIX = 'archi_dexgraph_op_data__'


class DexgraphTransitionFuncSymbol(FunctionSymbol):
    """DEG transition function symbol.
    """
    PREFIX = 'archi_dexgraph_transition__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION


class DexgraphTransitionDataSymbol(DataSymbol):
    """DEG transition function data symbol.
    """
    PREFIX = 'archi_dexgraph_transition_data__'

### archi/thread ###

class ThreadGroupWorkFuncSymbol(FunctionSymbol):
    """Thread group work function symbol.
    """
    PREFIX = 'archi_thread_group_work__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__THREAD_WORK


class ThreadGroupWorkDataSymbol(DataSymbol):
    """Thread group work function data symbol.
    """
    PREFIX = 'archi_thread_group_work_data__'


class ThreadGroupCallbackFuncSymbol(FunctionSymbol):
    """Thread group callback function symbol.
    """
    PREFIX = 'archi_thread_group_callback__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__THREAD_WORK


class ThreadGroupCallbackDataSymbol(DataSymbol):
    """Thread group callback function data symbol.
    """
    PREFIX = 'archi_thread_group_callback_data__'

### archi/signal ###

class SignalHandlerFuncSymbol(FunctionSymbol):
    """Signal handler function symbol.
    """
    PREFIX = 'archi_signal_handler__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__SIGNAL_HANDLER


class SignalHandlerDataSymbol(DataSymbol):
    """Signal handler function data symbol.
    """
    PREFIX = 'archi_signal_handler_data__'

### archi/memory ###

class MemoryInterfaceSymbol(DataSymbol):
    """Memory interface symbol.
    """
    PREFIX = 'archi_memory_interface__'
    TAG = ac.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE

