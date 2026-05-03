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
# @brief Archipelago application contexts.

import ctypes as c
from types import MappingProxyType, SimpleNamespace

import archi.ctypes as ac
from .object import Object, PrimitiveData, String


TypeAttributes = ac.archi_pointer_attr_t


def type_attributes_of(entity, /):
    """Get type attributes of an entity.
    """
    if entity is None:
        return ... # nothingness has unspecified type

    elif isinstance(entity, Context):
        return entity.__class__.slot_attr()

    elif isinstance(entity, Context.Slot):
        context = Context.Slot.context_of(entity)
        slot_name = Context.Slot.name_of(entity)
        slot_indices = Context.Slot.indices_of(entity)

        attr = context.__class__.slot_attr(
                slot_name, slot_indices, call=Context.Slot.is_call(entity))

        if attr is None:
            raise RuntimeError(f"Call slot {_slot_str(slot_name, slot_indices)} of context '{Context.key_of(context)}' doesn't return a value")

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

    def __init_subclass__(cls, /):
        """Initialize a context interface subclass.
        """
        if cls.C_NAME is not None:
            if not isinstance(cls.C_NAME, str):
                raise TypeError
            elif not cls.C_NAME:
                raise ValueError

    def __init__(self, /):
        """Initialize a context instance.
        """
        fields = SimpleNamespace()

        fields.registry = None
        fields.key = None

        object.__setattr__(self, '_', fields)

    def __str__(self, /):
        return f"Context({vars(self._)})"

    def __getattr__(self, name, /):
        """Get a context slot.
        """
        return Context.Slot(self, name=name)

    def __getitem__(self, index, /):
        """Get a context slot.
        """
        if isinstance(index, int):
            return Context.Slot(self, indices=(index,))

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            return Context.Slot(self, indices=index)

        else:
            raise TypeError

    def __setattr__(self, name, value, /):
        """Set a context slot.
        """
        Context._set_slot(self, name=name, value=value)

    def __setitem__(self, index, value, /):
        """Set a context slot.
        """
        if isinstance(index, int):
            Context._set_slot(self, indices=(index,), value=value)

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            Context._set_slot(self, indices=index, value=value)

        else:
            raise TypeError

    def __delattr__(self, name, /):
        """Unset a context slot.
        """
        Context._unset_slot(self, name=name)

    def __delitem__(self, index, value, /):
        """Unset a context slot.
        """
        if isinstance(index, int):
            Context._unset_slot(self, indices=(index,))

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            Context._unset_slot(self, indices=index)

        else:
            raise TypeError

    def __call__(self, _=None, /, **params):
        """Invoke a call.
        """
        if not isinstance(_, (type(None), Parameters.Context)):
            raise TypeError

        call_params_cls = self.__class__.call_params_class()

        return Context.Slot(self, call_params=call_params_cls(_, **params))

    @staticmethod
    def registry_of(context, /):
        """Get the registry of a context.
        """
        if context is None:
            return None
        elif not isinstance(context, Context):
            raise TypeError

        return context._.registry

    @staticmethod
    def key_of(context, /):
        """Get the key of a context.
        """
        if context is None:
            return None
        elif not isinstance(context, Context):
            raise TypeError

        return context._.key

    @staticmethod
    def weak_ref(entity, /):
        """Create a weak reference slot.
        """
        if isinstance(entity, Context):
            return Context.Slot(entity, weak_ref=True)
        elif isinstance(entity, Context.Slot):
            return Context.Slot(Context.Slot.context_of(entity),
                                Context.Slot.name_of(entity), Context.Slot.indices_of(entity),
                                Context.Slot.call_params_of(entity), weak_ref=True)
        else:
            return entity

    @staticmethod
    def set(slot, value, /):
        """Assign a value to the specified slot.
        """
        if not isinstance(slot, Context.Slot):
            raise TypeError
        elif Context.Slot.is_call(slot):
            raise AttributeError

        Context._set_slot(Context.Slot.context_of(slot),
                          Context.Slot.name_of(slot), Context.Slot.indices_of(slot),
                          value)

    @staticmethod
    def unset(slot, /):
        """Unset a context slot.
        """
        if not isinstance(slot, Context.Slot):
            raise TypeError
        elif Context.Slot.is_call(slot):
            raise AttributeError

        Context._unset_slot(Context.Slot.context_of(slot),
                            Context.Slot.name_of(slot), Context.Slot.indices_of(slot))

    @classmethod
    def interface(cls, context_or_slot, /):
        """Create a representation of a context interface (from the specified context/slot).
        """
        if not isinstance(context_or_slot, (Context, Context.Slot)):
            raise TypeError

        attr = type_attributes_of(context_or_slot)

        if not type_attributes_compatible(
                attr, TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE)):
            raise TypeError(f"{context_or_slot} is not a context interface")

        if isinstance(context_or_slot, Context):
            return _ContextInterface(Context.Slot(context_or_slot), cls)

        elif isinstance(context_or_slot, Context.Slot):
            return _ContextInterface(context_or_slot, cls)

    @classmethod
    def interface_in(cls, library, /):
        """Create a representation of a context interface (from a plugin using the predefined symbol name).
        """
        slot = ContextInterfaceSymbol.slot(cls.C_NAME, library=library)
        slot = Context.Slot(Context.Slot.context_of(slot), name=Context.Slot.name_of(slot)) # optimization to avoid creation of a temporary context

        return _ContextInterface(slot, cls)

    @classmethod
    def init_params_class(cls, /):
        """Obtain initialization parameter list class.

        Returns a (sub)class of Parameters.
        """
        params_class = cls._init_params_class()

        if not issubclass(params_class, Parameters):
            raise TypeError

        return params_class

    @classmethod
    def call_params_class(cls, /, name='', indices=()):
        """Obtain parameter list class of a call.

        Returns a (sub)class of Parameters.
        Raises KeyError if the call is not recognized.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not isinstance(indices, tuple):
            raise TypeError
        elif not all(isinstance(index, int) for index in indices):
            raise TypeError

        try:
            params_class = cls._call_params_class(name, indices)
        except KeyError:
            raise KeyError(f"{cls}: call slot {_slot_str(name, indices)} is not recognized")

        if not issubclass(params_class, Parameters):
            raise TypeError

        return params_class

    @classmethod
    def slot_attr(cls, /, name='', indices=(), setter=False, call=False):
        """Get type attributes of a slot.

        Returns a TypeAttributes object, Ellipsis, or None.
        Raises KeyError if the slot is not recognized.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not isinstance(indices, tuple):
            raise TypeError
        elif not all(isinstance(index, int) for index in indices):
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
        elif not isinstance(indices, tuple):
            raise TypeError
        elif not all(isinstance(index, int) for index in indices):
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
    def is_slot_unsettable(cls, /, name='', indices=()):
        """Check if a slot can be unset.

        Returns a boolean.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not isinstance(indices, tuple):
            raise TypeError
        elif not all(isinstance(index, int) for index in indices):
            raise TypeError
        elif not name and not indices:
            raise ValueError("Cannot unset empty slot")

        return cls._is_slot_unsettable(name, indices)

    @classmethod
    def _init_params_class(cls, /):
        """Obtain initialization parameter list class.

        This method is to be overridden in derived classes.

        Returns a (sub)class of Parameters.
        """
        return Parameters # arbitrary parameters is recognized by default

    @classmethod
    def _call_params_class(cls, /, name, indices):
        """Obtain parameter list class of a call.

        This method is to be overridden in derived classes.

        Returns a (sub)class of Parameters.
        Raises KeyError if the call is not recognized.
        """
        return Parameters # any call with arbitrary parameters is recognized by default

    @classmethod
    def _slot_attr(cls, /, name, indices, setter, call):
        """Get type attributes of a slot.

        This method is to be overridden in derived classes.

        Returns a TypeAttributes object, Ellipsis, or None.
        Raises KeyError if the slot is not recognized.
        """
        return ... # any slot is recognized by default

    @classmethod
    def _slot_object(cls, value, /, name, indices):
        """Construct an Object of appropriate type for a value to be assigned to the specified slot.

        This method is to be overridden in derived classes.

        Returns an Object instance or None.
        Raises TypeError if an Object of appropriate type cannot be constructed.
        """
        raise TypeError # don't know what to construct by default

    @classmethod
    def _is_slot_unsettable(cls, /, name, indices):
        """Check if a slot can be unset.

        This method is to be overridden in derived classes.

        Returns a boolean.
        """
        return True # all slots are unsettable by default

    @staticmethod
    def _set_slot(context, /, name='', indices=(), value=None):
        """Check compatibility of source and target types, append the assignment operation to the list.
        """
        if context._.registry is None:
            raise RuntimeError
        elif not name and not indices:
            raise AttributeError("Cannot set an empty slot")

        if not context._.registry.owns(value):
            raise ValueError("The assigned value is not owned by the same registry")

        target_attr = context.__class__.slot_attr(name, indices, setter=True)
        source_attr = type_attributes_of(value)

        if source_attr is None:
            value = context.__class__.slot_object(value, name, indices)
            source_attr = value.attributes

        if not type_attributes_compatible(source_attr, target_attr):
            raise TypeError(f"Cannot assign value={value} to slot {_slot_str(name, indices)} of context '{context._.key}': types are incompatible")

        context._.registry.operations.set_slot(context._.key, name, indices, value)

    @staticmethod
    def _unset_slot(context, /, name='', indices=()):
        """Append the unassignment operation to the list.
        """
        if context._.registry is None:
            raise RuntimeError
        elif not name and not indices:
            raise AttributeError("Cannot unset an empty slot")

        if not context.__class__.is_slot_unsettable(name, indices):
            raise AttributeError(f"Slot {_slot_str(name, indices)} of context '{context._.key}' is not unsettable")

        context._.registry.operations.unset_slot(context._.key, name, indices)


class _ContextSlot:
    """Representation of a context slot.

    Instances of this class are not to be created directly.
    """
    def __init__(self, context, /, name='', indices=(), call_params=None, weak_ref=False):
        """Initialize a context slot instance.
        """
        if not isinstance(context, Context):
            raise TypeError
        elif not isinstance(name, str):
            raise TypeError
        elif not isinstance(indices, tuple):
            raise TypeError
        elif not all(isinstance(index, int) for index in indices):
            raise TypeError
        elif not isinstance(call_params, (type(None), Parameters)):
            raise TypeError
        elif not isinstance(weak_ref, bool):
            raise TypeError

        fields = SimpleNamespace()

        fields.context = context
        fields.name = name
        fields.indices = indices
        fields.call_params = call_params
        fields.weak_ref = weak_ref

        object.__setattr__(self, '_', fields)

    def __str__(self, /):
        return f"Context.Slot({vars(self._)})"

    def __getattr__(self, name, /):
        """Get a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        return _ContextSlot(self._.context, name=f'{self._.name}.{name}',
                            indices=self._.indices)

    def __getitem__(self, index, /):
        """Get a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        if isinstance(index, int):
            return _ContextSlot(self._.context, name=self._.name,
                                indices=self._.indices + (index,))

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            return _ContextSlot(self._.context, name=self._.name,
                                indices=self._.indices + index)

        else:
            raise TypeError

    def __setattr__(self, name, value, /):
        """Set a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        Context._set_slot(self._.context, name=f'{self._.name}.{name}',
                          indices=self._.indices, value=value)

    def __setitem__(self, index, value, /):
        """Set a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        if isinstance(index, int):
            Context._set_slot(self._.context, name=self._.name,
                              indices=self._.indices + (index,), value=value)

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            Context._set_slot(self._.context, name=self._.name,
                              indices=self._.indices + index, value=value)

        else:
            raise TypeError

    def __delattr__(self, name, /):
        """Unset a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        Context._unset_slot(self._.context, name=f'{self._.name}.{name}',
                            indices=self._.indices)

    def __delitem__(self, index, value, /):
        """Unset a context slot.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError

        if isinstance(index, int):
            Context._unset_slot(self._.context, name=self._.name,
                                indices=self._.indices + (index,))

        elif isinstance(index, tuple):
            if not all(isinstance(elt, int) for elt in index):
                raise TypeError

            Context._unset_slot(self._.context, name=self._.name,
                                indices=self._.indices + index)

        else:
            raise TypeError

    def __call__(self, _=None, /, **params):
        """Invoke a call.
        """
        if self._.call_params is not None or self._.weak_ref:
            raise AttributeError
        elif not isinstance(_, (type(None), Parameters.Context)):
            raise TypeError

        call_params_cls = self._.context.__class__.call_params_class(
                self._.name, self._.indices)

        return _ContextSlot(self._.context, name=self._.name, indices=self._.indices,
                            call_params=call_params_cls(_, **params))

    @staticmethod
    def context_of(slot, /):
        """Get the context of a slot.
        """
        if slot is None:
            return None
        elif not isinstance(slot, Context.Slot):
            raise TypeError

        return slot._.context

    @staticmethod
    def name_of(slot, /):
        """Get the name of a slot.
        """
        if slot is None:
            return None
        elif not isinstance(slot, Context.Slot):
            raise TypeError

        return slot._.name

    @staticmethod
    def indices_of(slot, /):
        """Get the indices of a slot.
        """
        if slot is None:
            return None
        elif not isinstance(slot, Context.Slot):
            raise TypeError

        return slot._.indices

    @staticmethod
    def call_params_of(slot, /):
        """Get the call parameters of a slot.
        """
        if slot is None:
            return None
        elif not isinstance(slot, Context.Slot):
            raise TypeError

        return slot._.call_params

    @staticmethod
    def is_call(slot, /):
        """Check if a slot is a call.
        """
        if slot is None:
            return False
        elif not isinstance(slot, Context.Slot):
            raise TypeError

        return slot._.call_params is not None

    @staticmethod
    def is_weak_ref(slot, /):
        """Check if a slot reference is weak.
        """
        if slot is None:
            return False
        elif not isinstance(slot, Context.Slot):
            raise TypeError

        return slot._.weak_ref

Context.Slot = _ContextSlot


def _slot_str(slot_name, slot_indices, /):
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
    def __init__(self, interface, params, /):
        """Initialize a context specification instance.
        """
        if not isinstance(interface, _ContextInterface):
            raise TypeError
        elif not isinstance(params, Parameters):
            raise TypeError

        self._interface_origin = interface.origin
        self._context_cls = interface.context_cls
        self._params = params

    def __str__(self, /):
        return f"_ContextSpec(interface={self.interface_origin}, context_class={self.context_cls}, params={self.params})"

    def is_a(self, cls, /):
        """Refine a created context class.
        """
        if not issubclass(cls, self.context_cls):
            raise TypeError(f"Can't refine {self.context_cls} to {cls}: isn't a subclass")

        self._context_cls = cls
        return self

    @property
    def interface_origin(self, /):
        """Obtain the inteface origin context/slot.
        """
        return self._interface_origin

    @property
    def context_cls(self, /):
        """Obtain the context class.
        """
        return self._context_cls

    @property
    def params(self, /):
        """Obtain the context initialization parameters.
        """
        return self._params


class _ContextInterface:
    """Representation of a context slot.

    Instances of this class are not to be created directly.
    """
    def __init__(self, context_or_slot, /, context_cls=Context):
        """Initialize a context interface instance.
        """
        if not isinstance(context_or_slot, (Context, Context.Slot)):
            raise TypeError
        elif not issubclass(context_cls, Context):
            raise TypeError

        self._origin = context_or_slot
        self._context_cls = context_cls

    def __str__(self, /):
        return f"_ContextInterface(origin={self.origin}, context_class={self.context_cls})"

    def __call__(self, _=None, /, **params):
        """Create a context specification instance.
        """
        return _ContextSpec(self, self.context_cls.init_params_class()(_, **params))

    @property
    def origin(self, /):
        """Obtain the inteface origin context/slot.
        """
        return self._origin

    @property
    def context_cls(self, /):
        """Obtain the context class.
        """
        return self._context_cls

##############################################################################

class Parameters:
    """Representation of a parameter list.

    This base class does not impose any restrictions on parameters.
    """
    class Context(Context):
        """Base context type for parameter lists.

        This class is derived automatically in subclasses of Parameters class.
        """
        C_NAME = 'plist'

        @classmethod
        def _call_params_class(cls, /, name, indices):
            if name or indices:
                raise KeyError

            return cls.init_params_class()

        @classmethod
        def _slot_attr(cls, /, name, indices, setter, call):
            if indices:
                raise KeyError

            if not name:
                if not call:
                    return TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__KVLIST)
                else:
                    return None

            return cls.init_params_class().param_attr(name)

        @classmethod
        def _slot_object(cls, value, /, name, indices):
            if indices:
                raise KeyError

            return cls.init_params_class().param_object(value, name)

        @classmethod
        def _is_slot_unsettable(cls, /, name, indices):
            return False

    def __init_subclass__(cls, /):
        """Initialize a subclass.
        """
        class ParametersSubclassContext(cls.Context):
            @classmethod
            def _init_params_class(_, /):
                return cls

        cls.Context = ParametersSubclassContext

    def __init__(self, _=None, /, **params):
        """Initialize a context parameter list instance.
        """
        if not isinstance(_, (type(None), self.__class__.Context)):
            raise TypeError

        self._params_base = _
        self._params_dynamic = {}
        self._params_static = {}

        for key, value in params.items():
            param_attr = self.__class__.param_attr(key)
            value_attr = type_attributes_of(value)

            if value_attr is None:
                value = self.__class__.param_object(value, key)
                value_attr = value.attributes

            if not type_attributes_compatible(value_attr, param_attr):
                raise TypeError(f"{self.__class__}: cannot assign value={value} to parameter '{key}': types are incompatible")

            if isinstance(value, (Context, Context.Slot)):
                self._params_dynamic[key] = value
            else: # Object or None
                self._params_static[key] = value

    def __str__(self, /):
        return f"Parameters(base={self.base_context}, dynamic={self.dynamic_params}, static={self.static_params})"

    @property
    def base_context(self, /):
        """Obtain the base parameter list context.
        """
        return self._params_base

    @property
    def base_context_key(self, /):
        """Obtain key of the base parameter list context.
        """
        return Context.key_of(self._params_base)

    @property
    def params(self, /):
        """Obtain dictionary of all parameters in the parameter list.
        """
        return MappingProxyType(self._params_dynamic | self._params_static)

    @property
    def dynamic_params(self, /):
        """Obtain dictionary of dynamic parameters in the parameter list.
        """
        return MappingProxyType(self._params_dynamic)

    @property
    def static_params(self, /):
        """Obtain dictionary of static parameters in the parameter list.
        """
        return MappingProxyType(self._params_static)

    @classmethod
    def param_attr(cls, /, key):
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
    def _param_attr(cls, /, key):
        """Get type attributes of a parameter.

        This method is to be overridden in derived classes.

        Returns a TypeAttributes object or Ellipsis.
        Raises KeyError if the parameter is not recognized.
        """
        return ... # any parameter is recognized by default

    @classmethod
    def _param_object(cls, value, /, key):
        """Construct an Object of appropriate type for a value to be assigned to the specified parameter.

        This method is to be overridden in derived classes.

        Returns an Object instance or None.
        Raises TypeError if an Object of appropriate type cannot be constructed.
        """
        raise TypeError # don't know what to construct by default

##############################################################################
# Built-in context types
##############################################################################

class ParametersBase(Parameters):
    """Base class for custom parameter lists.
    """
    @classmethod
    def _param_attr(cls, /, key):
        raise KeyError # no parameters recognized by default


class ContextBase(Context):
    """Base class for custom context types.
    """
    InitParameters = ParametersBase

    @classmethod
    def _init_params_class(cls, /):
        return cls.InitParameters

    @classmethod
    def _call_params_class(cls, /, name, indices):
        raise KeyError # no calls supported by default

    @classmethod
    def _slot_attr(cls, /, name, indices, setter, call):
        raise KeyError # no slots recognized by default

    @classmethod
    def _is_slot_unsettable(cls, /, name, indices):
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
    def _call_params_class(cls, /, name, indices):
        call = cls.CALL_SLOTS[name]

        if isinstance(call, dict):
            return call[len(indices)][1]
        elif not indices:
            return call[1]
        else:
            raise KeyError

    @classmethod
    def _slot_attr(cls, /, name, indices, setter, call):
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
    def _slot_object(cls, value, /, name, indices):
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
    def _param_attr(cls, /, key):
        attr = cls.PARAMS[key]

        if isinstance(attr, tuple):
            return attr[0]
        else:
            return attr

    @classmethod
    def _param_object(cls, value, /, key):
        attr = cls.PARAMS[key]

        if isinstance(attr, tuple):
            return attr[1](value) if value is not None else None
        else:
            raise TypeError

##############################################################################

def _make_size_t(value):
    if value < 0:
        raise ValueError
    return PrimitiveData(c.c_size_t(value))

_TYPE_DATA = TypeAttributes.complex_data()
_TYPE_FUNCTION = TypeAttributes.function()
_TYPE_BOOL = (TypeAttributes.from_type(c.c_char),
              lambda value: PrimitiveData(c.c_char(bool(value))))
_TYPE_INT = (TypeAttributes.from_type(c.c_int),
             lambda value: PrimitiveData(c.c_int(value)))
_TYPE_LONGLONG = (TypeAttributes.from_type(c.c_longlong),
                  lambda value: PrimitiveData(c.c_longlong(value)))
_TYPE_SIZE = (TypeAttributes.from_type(c.c_size_t), _make_size_t)
_TYPE_ATTR = (TypeAttributes.from_type(TypeAttributes),
              lambda value: PrimitiveData(TypeAttributes(value)))
_TYPE_DATA_PTR = TypeAttributes.from_type(c.c_void_p)
_TYPE_STRING = (TypeAttributes.from_type(c.c_char * 1),
                lambda value: String(value))

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

    CONTEXT_TYPE = TypeAttributes.complex_data(ac.ARCHI_POINTER_DATA_TAG__DPTR_ARRAY)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'length': _TYPE_SIZE}

    GETTER_SLOTS = {'': {1: _TYPE_DATA},
                    'ptr': {1: _TYPE_DATA_PTR},
                    'ptrs': {0: _TYPE_DATA_PTR,
                             1: _TYPE_DATA_PTR},
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
    def _slot_attr(cls, /, name, indices, setter, call):
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

    def _slot_attr(cls, /, name, indices, setter, call):
        if call:
            raise KeyError
        elif indices:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE

        return TypeAttributes.from_type(ac.archi_signal_handler_t)

    @classmethod
    def _is_slot_unsettable(cls, /, name, indices):
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
    def _call_params_class(cls, /, name, indices):
        if indices:
            raise KeyError

        if name.startswith('data.'):
            return cls.DataSymbolCallParameters
        elif name.startswith('function.'):
            return cls.FunctionSymbolCallParameters
        else:
            raise KeyError

    @classmethod
    def _slot_attr(cls, /, name, indices, setter, call):
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
    def _slot_attr(cls, /, name, indices, setter, call):
        if indices or call:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE

        return ...

    @classmethod
    def _is_slot_unsettable(cls, /, name, indices):
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
    def _slot_attr(cls, /, name, indices, setter, call):
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
    NAMESPACE_PREFIX = 'archi_'
    PREFIX = ''
    POSTFIX = ''
    TAG = 0

    def __init_subclass__(cls, /):
        """Initialize a subclass.
        """
        if not isinstance(cls.NAMESPACE_PREFIX, str):
            raise TypeError
        elif not isinstance(cls.PREFIX, str):
            raise TypeError
        elif not isinstance(cls.POSTFIX, str):
            raise TypeError
        elif not isinstance(cls.TAG, int):
            raise TypeError

        if cls.TAG < 0:
            raise ValueError

    @classmethod
    def full_name(cls, name, /):
        """Get the full symbol name including prefix and postfix,
        but not including the namespace prefix.
        """
        if not isinstance(name, str):
            raise TypeError

        return f'{cls.PREFIX}{name}{cls.POSTFIX}'

    @classmethod
    def attributes(cls, /):
        """Get type attributes.
        """
        raise NotImplementedError

    @classmethod
    def slot(cls, name, /, library, namespace=NAMESPACE_PREFIX):
        """Obtain the correctly typed symbol slot from the specified library.
        """
        if not isinstance(name, str):
            raise TypeError
        elif not name:
            raise ValueError
        elif not isinstance(library, LibraryContext):
            raise TypeError
        elif not isinstance(namespace, str):
            raise TypeError

        return getattr(cls._symbol_type(library),
                       namespace + cls.full_name(name))(tag=cls.TAG)

    @classmethod
    def _symbol_type(cls, /, library):
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
    def name(self, /):
        """Get symbol name.
        """
        return self._name

    def slot_of(self, library, /):
        """Obtain the correctly typed symbol slot from the specified library.
        """
        return self.__class__.slot(self.name, library=library)


class DataSymbol(_Symbol):
    """Representation of an arbitrary data symbol.
    """
    def __init_subclass__(cls, /):
        """Initialize a subclass.
        """
        super().__init_subclass__()

        if cls.TAG > TypeAttributes.DATA_TAG_MAX:
            raise ValueError

    @classmethod
    def attributes(cls, /):
        """Get type attributes.
        """
        return TypeAttributes.complex_data(cls.TAG)

    @classmethod
    def _symbol_type(cls, /, library):
        return library.data


class FunctionSymbol(_Symbol):
    """Representation of an arbitrary function symbol.
    """
    def __init_subclass__(cls, /):
        """Initialize a subclass.
        """
        super().__init_subclass__()

        if cls.TAG > TypeAttributes.FUNC_TAG_MAX:
            raise ValueError

    @classmethod
    def attributes(cls, /):
        """Get type attributes.
        """
        return TypeAttributes.function(cls.TAG)

    @classmethod
    def _symbol_type(cls, /, library):
        return library.function

### archi/context ###

class ContextInterfaceSymbol(DataSymbol):
    """Context interface symbol.
    """
    PREFIX = 'context_interface__'
    TAG = ac.ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE

### archi/aggr ###

class AggregateInterfaceSymbol(DataSymbol):
    """Aggregate type interface symbol.
    """
    PREFIX = 'aggr_interface__'
    TAG = ac.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE


class AggregateTypeSymbol(DataSymbol):
    """Aggregate type description symbol.
    """
    PREFIX = 'aggr_type__'
    TAG = ac.ARCHI_POINTER_DATA_TAG__AGGR_TYPE

### archi/exec ###

class DexgraphOperationFuncSymbol(FunctionSymbol):
    """DEG operation function symbol.
    """
    PREFIX = 'dexgraph_op__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION


class DexgraphOperationDataSymbol(DataSymbol):
    """DEG operation function data symbol.
    """
    PREFIX = 'dexgraph_op_data__'


class DexgraphTransitionFuncSymbol(FunctionSymbol):
    """DEG transition function symbol.
    """
    PREFIX = 'dexgraph_transition__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION


class DexgraphTransitionDataSymbol(DataSymbol):
    """DEG transition function data symbol.
    """
    PREFIX = 'dexgraph_transition_data__'

### archi/thread ###

class ThreadGroupWorkFuncSymbol(FunctionSymbol):
    """Thread group work function symbol.
    """
    PREFIX = 'thread_group_work__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__THREAD_WORK


class ThreadGroupWorkDataSymbol(DataSymbol):
    """Thread group work function data symbol.
    """
    PREFIX = 'thread_group_work_data__'


class ThreadGroupCallbackFuncSymbol(FunctionSymbol):
    """Thread group callback function symbol.
    """
    PREFIX = 'thread_group_callback__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__THREAD_WORK


class ThreadGroupCallbackDataSymbol(DataSymbol):
    """Thread group callback function data symbol.
    """
    PREFIX = 'thread_group_callback_data__'

### archi/signal ###

class SignalHandlerFuncSymbol(FunctionSymbol):
    """Signal handler function symbol.
    """
    PREFIX = 'signal_handler__'
    TAG = ac.ARCHI_POINTER_FUNC_TAG__SIGNAL_HANDLER


class SignalHandlerDataSymbol(DataSymbol):
    """Signal handler function data symbol.
    """
    PREFIX = 'signal_handler_data__'

### archi/memory ###

class MemoryInterfaceSymbol(DataSymbol):
    """Memory interface symbol.
    """
    PREFIX = 'memory_interface__'
    TAG = ac.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE


class MemoryAllocDataSymbol(DataSymbol):
    """Memory allocation function data symbol.
    """
    PREFIX = 'memory_alloc_data__'


class MemoryMapDataSymbol(DataSymbol):
    """Memory mapping function data symbol.
    """
    PREFIX = 'memory_map_data__'

