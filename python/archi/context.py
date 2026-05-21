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
from types import MappingProxyType, NoneType, SimpleNamespace

import archi.ctypes as typ
from .object import Object, PrimitiveData, String


def _slot_str(name, indices, /):
    """Get the string specification of a (non-call) slot.
    """
    str_name = f"'{name}'" if name else ""
    str_indices = ("[" + ", ".join(str(index) for index in indices) + "]") if indices else ""

    return str_name + str_indices


def _slot_check(name, indices, /):
    """Check if slot name and indices are valid.
    """
    if not isinstance(name, str):
        raise TypeError
    elif not isinstance(indices, tuple):
        raise TypeError
    elif not all(isinstance(index, int) for index in indices):
        raise TypeError

##############################################################################

class TypeAttr(typ.archi_pointer_attr_t):
    """Type attributes with support of objects, contexts, and context slots.
    """
    UNSPECIFIED = object()

    @staticmethod
    def of(entity, /):
        """Get type attributes of an entity.
        """
        if entity is None:
            return TypeAttr.UNSPECIFIED

        elif isinstance(entity, Object):
            if not entity.is_function:
                if entity.tag is None:
                    return TypeAttr.primitive_data(entity.length, entity.stride, entity.alignment)
                else:
                    return TypeAttr.complex_data(entity.tag)
            else:
                return TypeAttr.function(entity.tag)

        elif isinstance(entity, Context):
            return entity.__class__.slot_attr()

        elif isinstance(entity, Context.Slot):
            context = Context.Slot.context_of(entity)

            if context is None:
                raise ValueError("Cannot get type attributes for {entity}: no context")

            slot_name = Context.Slot.name_of(entity)
            slot_indices = Context.Slot.indices_of(entity)
            slot_is_call = Context.Slot.is_call(entity)

            return context.__class__.slot_attr(slot_name, slot_indices, call=slot_is_call)

        else:
            return NotImplemented # unknown entity type

    @staticmethod
    def compatible(attr1, attr2, /):
        """Check compatibility of type attributes.
        """
        if attr1 is not TypeAttr.UNSPECIFIED and not isinstance(attr1, (NoneType, TypeAttr)):
            raise TypeError
        elif attr2 is not TypeAttr.UNSPECIFIED and not isinstance(attr2, (NoneType, TypeAttr)):
            raise TypeError

        if attr1 is None or attr2 is None: # either is absent
            return False
        elif attr1 is TypeAttr.UNSPECIFIED or attr2 is TypeAttr.UNSPECIFIED:
            return True

        return attr1.is_compatible_to(attr2)

##############################################################################

class _ContextTyping:
    """Interface of context slot typing.
    """
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
        """Obtain parameter list class of a call slot.

        Returns a (sub)class of Parameters.
        Raises KeyError if the call is not recognized.
        """
        _slot_check(name, indices)

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

        Returns a TypeAttr object, Ellipsis, or None.
        Raises KeyError if the slot is not recognized.
        """
        _slot_check(name, indices)

        if not isinstance(setter, bool):
            raise TypeError
        elif not isinstance(call, bool):
            raise TypeError

        if setter:
            if not name and not indices:
                raise ValueError("Empty slot cannot be set")
            elif call:
                raise ValueError("Call slot cannot be set")

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

        if attr is not TypeAttr.UNSPECIFIED and not isinstance(attr, (NoneType, TypeAttr)):
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
        _slot_check(name, indices)

        if not name and not indices:
            raise ValueError("Empty slot cannot be set")

        try:
            obj = cls._slot_object(value, name, indices)
        except TypeError:
            raise TypeError(f"{cls}: cannot assign {value} to slot {_slot_str(name, indices)}")

        if not isinstance(obj, (NoneType, Object)):
            raise TypeError

        return obj

    @classmethod
    def slot_unsettable(cls, /, name='', indices=()):
        """Check if a slot can be unset.

        Returns a boolean.
        """
        _slot_check(name, indices)

        if not name and not indices:
            return False

        return cls._slot_unsettable(name, indices)

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

        Returns a TypeAttr object, Ellipsis, or None.
        Raises KeyError if the slot is not recognized.
        """
        return TypeAttr.UNSPECIFIED # any slot is recognized by default

    @classmethod
    def _slot_object(cls, value, /, name, indices):
        """Construct an Object of appropriate type for a value to be assigned to the specified slot.

        This method is to be overridden in derived classes.

        Returns an Object instance or None.
        Raises TypeError if an Object of appropriate type cannot be constructed.
        """
        raise TypeError # don't know what to construct by default

    @classmethod
    def _slot_unsettable(cls, /, name, indices):
        """Check if a slot can be unset.

        This method is to be overridden in derived classes.

        Returns a boolean.
        """
        return True # all slots are unsettable by default


class Context(_ContextTyping):
    """Representation of a context.

    This base class does not impose any restrictions on slots.
    """
    __slots__ = ['_'] # (^_^)

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

    def __init__(self, key, /):
        """Initialize a context instance.
        """
        if not isinstance(key, str):
            raise TypeError("Context key must be a string")

        object.__setattr__(self, '_', SimpleNamespace(key=key))

    @staticmethod
    def key_of(context, /):
        """Retrieve the key of a context.
        """
        if context is None:
            return None
        elif not isinstance(context, Context):
            raise TypeError

        return object.__getattribute__(context, '_').key

    def __repr__(self, /):
        return f'{self.__class__.__name__}({repr(Context.key_of(self))})'

    def __eq__(self, other, /):
        if not other.__class__ is self.__class__:
            return False

        return object.__getattribute__(self, '_') == object.__getattribute__(other, '_')

    def __hash__(self, /):
        return hash((self.__class__,) + tuple(vars(object.__getattribute__(self, '_')).values()))

    def __getattr__(self, name, /):
        """Get a context slot.
        """
        return Context.Slot(context=self, name=name)

    def __getattribute__(self, name, /):
        """Get a context slot.
        """
        if not name.startswith('__'): # everything except Python machinery
            raise AttributeError

        return object.__getattribute__(self, name)

    def __setattr__(self, name, value, /):
        """Attribute setter method.
        """
        raise AttributeError

    def __delattr__(self, name, /):
        """Attribute deleter method.
        """
        raise AttributeError

    def __getitem__(self, subscript, /):
        """Get a context slot.
        """
        if isinstance(subscript, int):
            subscript = (subscript,)
        elif isinstance(subscript, tuple):
            if not all(isinstance(index, int) for index in subscript):
                raise TypeError
        else:
            raise TypeError

        return Context.Slot(context=self, indices=subscript)

    def __call__(self, _=None, /, **params):
        """Get a context slot call.
        """
        if not isinstance(_, (NoneType, Parameters.Context)):
            raise TypeError

        call_params_cls = self.__class__.call_params_class()
        return Context.Slot(context=self, call_params=call_params_cls(_, **params))

    @classmethod
    def interface(cls, context_or_slot, /):
        """Create a representation of a context interface (from the specified context/slot).
        """
        if isinstance(context_or_slot, Context):
            slot = Context.Slot(context=context_or_slot)
        elif isinstance(context_or_slot, Context.Slot):
            slot = context_or_slot
        else:
            raise TypeError

        return ContextInterface(slot, cls)

    @classmethod
    def interface_in(cls, library, /):
        """Create a representation of a context interface (from a plugin using the predefined symbol name).
        """
        slot = ContextInterfaceSymbol.slot(cls.C_NAME, library)

        # optimization by preventing creation of an unnecessary temporary context
        slot = Context.Slot(context=Context.Slot.context_of(slot), name=Context.Slot.name_of(slot))

        return ContextInterface(slot, cls)


class _ContextSlot:
    """Representation of a context slot.
    """
    UNSET = object() # special designator constant for slot unsetting operation

    __slots__ = ['_'] # (^_^)

    def __init_subclass__(cls):
        raise TypeError("Subclasses of Context.Slot are not allowed")

    def __init__(self, /, context=None, name='', indices=(), call_params=None, weak_ref=False):
        """Initialize a context slot instance.
        """
        if not isinstance(context, (NoneType, Context)):
            raise TypeError

        _slot_check(name, indices)

        if not isinstance(call_params, (NoneType, Parameters)):
            raise TypeError
        elif not isinstance(weak_ref, bool):
            raise TypeError

        object.__setattr__(self, '_', SimpleNamespace(context=context,
                                                      name=name,
                                                      indices=indices,
                                                      call_params=call_params,
                                                      weak_ref=weak_ref))

    @staticmethod
    def context_of(slot, /):
        """Get the context of a slot.
        """
        if slot is None:
            return None
        elif not isinstance(slot, _ContextSlot):
            raise TypeError

        return object.__getattribute__(slot, '_').context

    @staticmethod
    def context_key_of(slot, /):
        """Get the context key of a slot.
        """
        return Context.key_of(_ContextSlot.context_of(slot))

    @staticmethod
    def name_of(slot, /):
        """Get the name of a slot.
        """
        if slot is None:
            return None
        elif not isinstance(slot, _ContextSlot):
            raise TypeError

        return object.__getattribute__(slot, '_').name

    @staticmethod
    def indices_of(slot, /):
        """Get the indices of a slot.
        """
        if slot is None:
            return None
        elif not isinstance(slot, _ContextSlot):
            raise TypeError

        return object.__getattribute__(slot, '_').indices

    @staticmethod
    def call_params_of(slot, /):
        """Get the call parameters of a slot.
        """
        if slot is None:
            return None
        elif not isinstance(slot, _ContextSlot):
            raise TypeError

        return object.__getattribute__(slot, '_').call_params

    @staticmethod
    def is_call(slot, /):
        """Check if a slot is a call.
        """
        if slot is None:
            return False
        elif not isinstance(slot, _ContextSlot):
            raise TypeError

        return object.__getattribute__(slot, '_').call_params is not None

    @staticmethod
    def is_weak_ref(slot, /):
        """Check if a slot reference is weak.
        """
        if slot is None:
            return False
        elif not isinstance(slot, _ContextSlot):
            raise TypeError

        return object.__getattribute__(slot, '_').weak_ref

    def __repr__(self, /):
        context = repr(_ContextSlot.context_of(self))
        name = repr(_ContextSlot.name_of(self))
        indices = repr(_ContextSlot.indices_of(self))
        call_params = repr(_ContextSlot.call_params_of(self))
        weak_ref = repr(_ContextSlot.is_weak_ref(self))

        return f'Context.Slot(context={context}, name={name}, indices={indices}, call_params={call_params}, weak_ref={weak_ref})'

    def __eq__(self, other, /):
        if not other.__class__ is self.__class__:
            return False

        return object.__getattribute__(self, '_') == object.__getattribute__(other, '_')

    def __hash__(self, /):
        return hash(tuple(vars(object.__getattribute__(self, '_')).values()))

    def __getattr__(self, name, /):
        """Get a context slot.
        """
        if _ContextSlot.is_call(self):
            raise AttributeError

        slot_context = _ContextSlot.context_of(self)
        slot_name = _ContextSlot.name_of(self)
        slot_indices = _ContextSlot.indices_of(self)
        slot_weak_ref = _ContextSlot.is_weak_ref(self)

        sep = '.' if slot_name and name else ''

        return _ContextSlot(slot_context,
                            name=f'{slot_name}{sep}{name}',
                            indices=slot_indices,
                            weak_ref=slot_weak_ref)

    def __getattribute__(self, name, /):
        """Get a context slot.
        """
        if not name.startswith('__'): # everything except Python machinery
            raise AttributeError

        return object.__getattribute__(self, name)

    def __setattr__(self, name, value, /):
        """Attribute setter method.
        """
        raise AttributeError

    def __delattr__(self, name, /):
        """Attribute deleter method.
        """
        raise AttributeError

    def __getitem__(self, subscript, /):
        """Get a context slot.
        """
        if _ContextSlot.is_call(self):
            raise LookupError

        if isinstance(subscript, int):
            subscript = (subscript,)
        elif isinstance(subscript, tuple):
            if not all(isinstance(elt, int) for elt in subscript):
                raise TypeError
        else:
            raise TypeError

        slot_context = _ContextSlot.context_of(self)
        slot_name = _ContextSlot.name_of(self)
        slot_indices = _ContextSlot.indices_of(self)
        slot_weak_ref = _ContextSlot.is_weak_ref(self)

        return _ContextSlot(slot_context,
                            name=slot_name,
                            indices=slot_indices + subscript,
                            weak_ref=slot_weak_ref)

    def __call__(self, _=None, /, **params):
        """Get a context slot call.
        """
        if _ContextSlot.is_call(self):
            raise AttributeError
        elif not isinstance(_, (NoneType, Parameters.Context)):
            raise TypeError

        slot_context = _ContextSlot.context_of(self)
        slot_name = _ContextSlot.name_of(self)
        slot_indices = _ContextSlot.indices_of(self)

        if slot_context is not None:
            call_params_cls = slot_context.__class__.call_params_class(slot_name, slot_indices)
        else:
            call_params_cls = Parameters

        return _ContextSlot(slot_context, name=slot_name, indices=slot_indices,
                            call_params=call_params_cls(_, **params))

    @staticmethod
    def weak_ref(entity, /):
        """Make weak reference to a context slot.
        """
        if isinstance(entity, Context):
            return _ContextSlot(context=entity, weak_ref=True)

        elif isinstance(entity, _ContextSlot):
            return _ContextSlot(context=Context.Slot.context_of(entity),
                                name=Context.Slot.name_of(entity),
                                indices=Context.Slot.indices_of(entity),
                                call_params=Context.Slot.call_params_of(entity),
                                weak_ref=True)

        else:
            return entity

    def __add__(self, other, /):
        """Merge context slots.
        """
        if other is None:
            return self

        if not isinstance(other, _ContextSlot):
            raise TypeError
        elif _ContextSlot.context_of(other) is not None:
            raise AttributeError
        elif _ContextSlot.is_call(self):
            raise AttributeError

        slot_context = _ContextSlot.context_of(self)
        slot_name = _ContextSlot.name_of(self)
        slot_indices = _ContextSlot.indices_of(self)
        slot_weak_ref = _ContextSlot.is_weak_ref(self)

        other_name = _ContextSlot.name_of(other)
        other_indices = _ContextSlot.indices_of(other)
        other_call_params = _ContextSlot.call_params_of(other)
        other_weak_ref = _ContextSlot.is_weak_ref(other)

        name_sep = '.' if slot_name or other_name else ''

        return _ContextSlot(context=slot_context,
                            name=f'{slot_name}{name_sep}{other_name}',
                            indices=slot_indices + other_indices,
                            call_params=other_call_params,
                            weak_ref=slot_weak_ref or other_weak_ref)

    def __invert__(self, /):
        """Unset context of a context slot.
        """
        return _ContextSlot(name=_ContextSlot.name_of(self),
                            indices=_ContextSlot.indices_of(self),
                            call_params=_ContextSlot.call_params_of(self),
                            weak_ref=_ContextSlot.is_weak_ref(self))

    def __rmatmul__(self, other, /):
        """Merge a context and a context slot.
        """
        if not isinstance(other, Context):
            raise TypeError
        elif _ContextSlot.context_of(self) is not None:
            raise AttributeError

        return _ContextSlot(context=other,
                            name=_ContextSlot.name_of(self),
                            indices=_ContextSlot.indices_of(self),
                            call_params=_ContextSlot.call_params_of(self),
                            weak_ref=_ContextSlot.is_weak_ref(self))

    def __lshift__(self, other, /):
        """Create an assignment chain representation.
        """
        return _ContextSlotAssignment(self, other)


# Context slot is a nested class of Context
Context.Slot = _ContextSlot


class _ContextSlotAssignment:
    """Proxy representation of a context slot assignment chain.
    """
    def __init__(self, lhs, rhs, /):
        """Initialize an assignment chain.
        """
        if isinstance(lhs, Context.Slot):
            if isinstance(rhs, _ContextSlotAssignment):
                self._chain = (lhs,) + rhs.chain
            else:
                self._chain = (lhs, rhs)

        elif isinstance(lhs, _ContextSlotAssignment):
            if not isinstance(lhs.chain[-1], Context.Slot):
                raise TypeError("Non Context.Slot object cannot be on the left hand side of a slot assignment")

            if isinstance(rhs, _ContextSlotAssignment):
                self._chain = lhs.chain + rhs.chain
            else:
                self._chain = lhs.chain + (rhs,)

        else:
            raise TypeError("Unknown type on the left hand side of a slot assignment")

    def __repr__(self, /):
        return ' << '.join(repr(entity) for entity in self.chain)

    def __lshift__(self, other, /):
        """Create an assignment chain representation.
        """
        return _ContextSlotAssignment(self, other)

    @property
    def chain(self, /):
        """Get the assignment chain.
        """
        return self._chain

##############################################################################

class _ParameterTyping:
    """Interface of context parameters typing.
    """
    @classmethod
    def param_attr(cls, /, key):
        """Get type attributes of a parameter.

        Returns a TypeAttr object or Ellipsis.
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

        if attr is not TypeAttr.UNSPECIFIED and not isinstance(attr, TypeAttr):
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

        if not isinstance(obj, (NoneType, Object)):
            raise TypeError

        return obj

    @classmethod
    def _param_attr(cls, /, key):
        """Get type attributes of a parameter.

        This method is to be overridden in derived classes.

        Returns a TypeAttr object or Ellipsis.
        Raises KeyError if the parameter is not recognized.
        """
        return TypeAttr.UNSPECIFIED # any parameter is recognized by default

    @classmethod
    def _param_object(cls, value, /, key):
        """Construct an Object of appropriate type for a value to be assigned to the specified parameter.

        This method is to be overridden in derived classes.

        Returns an Object instance or None.
        Raises TypeError if an Object of appropriate type cannot be constructed.
        """
        raise TypeError # don't know what to construct by default


class ParametersContext(Context):
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
                return TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__KVLIST)
            else:
                return None

        return cls.init_params_class().param_attr(name)

    @classmethod
    def _slot_object(cls, value, /, name, indices):
        if indices:
            raise KeyError

        return cls.init_params_class().param_object(value, name)

    @classmethod
    def _slot_unsettable(cls, /, name, indices):
        return False


class Parameters(_ParameterTyping):
    """Representation of a parameter list.

    This base class does not impose any restrictions on parameters.
    """
    Context = ParametersContext

    def __init_subclass__(cls, /):
        """Initialize a subclass.
        """
        class _ParametersContext(cls.Context):
            @classmethod
            def _init_params_class(_, /):
                return cls

        cls.Context = _ParametersContext

    def __init__(self, _=None, /, **params):
        """Initialize a context parameter list instance.
        """
        if not isinstance(_, (NoneType, self.__class__.Context)):
            raise TypeError

        self._context = _
        self._params = params

    def __repr__(self, /):
        params = [f'{name}={repr(value)}' for name, value in self.dict.items()]
        if self.base_context is not None:
            params = [repr(self.base_context)] + params

        return f'{self.__class__.__name__}({', '.join(params)})'

    def __eq__(self, other, /):
        if not other.__class__ is self.__class__:
            return False

        return self.base_context == other.base_context \
                and self.dict == other.dict

    def __hash__(self, /):
        return hash((self.__class__, self.base_context) + tuple(self.dict.items()))

    @property
    def base_context(self, /):
        """Obtain the base parameter list context.
        """
        return self._context

    @property
    def dict(self, /):
        """Obtain dictionary of parameters in the parameter list.
        """
        return MappingProxyType(self._params)

##############################################################################

class ContextInterface:
    """Representation of a context interface.
    """
    def __init_subclass__(cls):
        raise TypeError("Subclasses of ContextInterface are not allowed")

    def __init__(self, origin, /, context_cls=Context):
        """Initialize a context interface instance.
        """
        if not isinstance(origin, (Context, Context.Slot)):
            raise TypeError
        elif not issubclass(context_cls, Context):
            raise TypeError

        self._origin = origin
        self._context_cls = context_cls

    def __repr__(self, /):
        return f"ContextInterface({self.origin}, context_cls={self.context_cls.__name__})"

    def __call__(self, _=None, /, **params):
        """Create a context specification instance.
        """
        return _ContextSpec(self.context_cls, self.origin,
                            self.context_cls.init_params_class()(_, **params))

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


class _ContextSpec:
    """Proxy representation of a created context specification.

    Instances of this class are not to be created directly.
    """
    def __init__(self, context_cls, interface_origin, params, /):
        """Initialize a context specification instance.
        """
        if not issubclass(context_cls, Context):
            raise TypeError
        elif not isinstance(interface_origin, (Context, Context.Slot)):
            raise TypeError
        elif not isinstance(params, Parameters):
            raise TypeError

        self._context_cls = context_cls
        self._interface_origin = interface_origin
        self._params = params

    def __repr__(self, /):
        return f"_ContextSpec({self.context_cls.__name__}, {self.interface_origin}, {self.params})"

    def is_a(self, cls, /):
        """Refine a created context class.
        """
        if not issubclass(cls, self.context_cls):
            raise TypeError(f"Can't refine {self.context_cls} to {cls}: isn't a subclass")

        self._context_cls = cls
        return self

    @property
    def context_cls(self, /):
        """Obtain the context class.
        """
        return self._context_cls

    @property
    def interface_origin(self, /):
        """Obtain the inteface origin context/slot.
        """
        return self._interface_origin

    @property
    def params(self, /):
        """Obtain the context initialization parameters.
        """
        return self._params

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
    def _slot_unsettable(cls, /, name, indices):
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
# Built-in context types
##############################################################################

def _make_size_t(value):
    if value < 0:
        raise ValueError
    return PrimitiveData(c.c_size_t(value))

_TYPE_DATA = TypeAttr.complex_data()
_TYPE_FUNCTION = TypeAttr.function()
_TYPE_BOOL = (TypeAttr.from_type(c.c_char),
              lambda value: PrimitiveData(c.c_char(bool(value))))
_TYPE_INT = (TypeAttr.from_type(c.c_int),
             lambda value: PrimitiveData(c.c_int(value)))
_TYPE_LONGLONG = (TypeAttr.from_type(c.c_longlong),
                  lambda value: PrimitiveData(c.c_longlong(value)))
_TYPE_SIZE = (TypeAttr.from_type(c.c_size_t), _make_size_t)
_TYPE_ATTR = (TypeAttr.from_type(TypeAttr),
              lambda value: PrimitiveData(TypeAttr(value)))
_TYPE_DATA_PTR = TypeAttr.from_type(c.c_void_p)
_TYPE_STRING = (TypeAttr.from_type(c.c_char * 1),
                lambda value: String(value))

### archi/context ###

class PointerContext(ContextWhitelist):
    """Arbitrary pointer.
    """
    C_NAME = 'pointer'

    CONTEXT_TYPE = TypeAttr.UNSPECIFIED

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': TypeAttr.UNSPECIFIED}

    GETTER_SLOTS = {'pointee': TypeAttr.UNSPECIFIED}
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

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DPTR_ARRAY)

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

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__AGGR)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'interface': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE),
                  'metadata': _TYPE_DATA,
                  'fam_length': _TYPE_SIZE}

    GETTER_SLOTS = {'interface': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE),
                    'metadata': _TYPE_DATA,
                    'layout': TypeAttr.from_type(typ.archi_layout_struct_t),
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
                return _TYPE_DATA if not setter else TypeAttr.UNSPECIFIED
            elif name.startswith('ref.') and not setter:
                return TypeAttr.UNSPECIFIED

        return super()._slot_attr(name, indices, setter, call)

### archi/exec ###

class DexgraphNodeContext(ContextWhitelist):
    """Directed execution graph node.
    """
    C_NAME = 'dexgraph_node'

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'name': _TYPE_STRING,
                  'sequence_length': _TYPE_SIZE,
                  'transition_func': TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                  'transition_data': _TYPE_DATA,
                  'branches': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}

    class ExecuteCallParameters(ParametersWhitelist):
        PARAMS = {'index': (TypeAttr.from_type(typ.archi_dexgraph_branch_index_t),
                            lambda value: PrimitiveData(typ.archi_dexgraph_branch_index_t(value)))}

    GETTER_SLOTS = {'name': _TYPE_STRING,
                    'sequence.length': _TYPE_SIZE,
                    'sequence.function': {1: TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)},
                    'sequence.data': {1: _TYPE_DATA},
                    'transition.function': TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                    'transition.data': _TYPE_DATA,
                    'branches': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}

    CALL_SLOTS = {'execute': (None, ExecuteCallParameters)}

    SETTER_SLOTS = {'sequence.function': {1: TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)},
                    'sequence.data': {1: _TYPE_DATA},
                    'transition.function': TypeAttr.function(typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                    'transition.data': _TYPE_DATA,
                    'branches': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}


class DexgraphNodeArrayContext(ContextWhitelist):
    """Directed execution graph node array.
    """
    C_NAME = 'dexgraph_node_array'

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'num_nodes': _TYPE_SIZE}

    GETTER_SLOTS = {'num_nodes': _TYPE_SIZE,
                    'node': {1: TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)}}

    SETTER_SLOTS = {'node': {1: TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)}}

### archi/thread ###

class ThreadGroupContext(ContextWhitelist):
    """Thread group.
    """
    C_NAME = 'thread_group'

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__THREAD_GROUP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttr.from_type(typ.archi_thread_group_start_params_t),
                             lambda value: PrimitiveData(value)),
                  'num_threads': _TYPE_SIZE}

    GETTER_SLOTS = {'num_threads': _TYPE_SIZE}


class LockFreeQueueContext(ContextWhitelist):
    """Lock-free queue.
    """
    C_NAME = 'thread_lfqueue'

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__THREAD_LFQUEUE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttr.from_type(typ.archi_thread_lfqueue_alloc_params_t),
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

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__SIGNAL_HANDLER_DATA__HASHMAP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttr.from_type(typ.archi_hashmap_alloc_params_t),
                             lambda value: PrimitiveData(value)),
                  'capacity': _TYPE_SIZE}

    def _slot_attr(cls, /, name, indices, setter, call):
        if call:
            raise KeyError
        elif indices:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE

        return TypeAttr.from_type(typ.archi_signal_handler_t)

    @classmethod
    def _slot_unsettable(cls, /, name, indices):
        return True

### archi/memory ###

class MemoryContext(ContextWhitelist):
    """Memory handle.
    """
    C_NAME = 'memory'

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__MEMORY)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'interface': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE),
                  'alloc_data': _TYPE_DATA,
                  'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE,
                  'ext_alignment': _TYPE_SIZE}

    GETTER_SLOTS = {'interface': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE),
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

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__MEMORY_MAPPING)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'memory': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__MEMORY),
                  'map_data': _TYPE_DATA,
                  'offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    GETTER_SLOTS = {'memory': TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__MEMORY),
                    'ptr': _TYPE_DATA,
                    'offset': _TYPE_SIZE,
                    'length': _TYPE_SIZE,
                    'size': _TYPE_SIZE}

### archi/file ###

class FileContext(ContextWhitelist):
    """File handle.
    """
    C_NAME = 'file'

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__FILE_STREAM)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pathname': _TYPE_STRING,
                  'fd': (TypeAttr.from_type(typ.archi_file_descriptor_t),
                         lambda value: PrimitiveData(typ.archi_file_descriptor_t(value))),
                  'params': TypeAttr.from_type(typ.archi_file_open_params_t),
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

    GETTER_SLOTS = {'fd': (TypeAttr.from_type(typ.archi_file_descriptor_t),
                           lambda value: PrimitiveData(typ.archi_file_descriptor_t(value))),
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

    CONTEXT_TYPE = TypeAttr.complex_data()

    class InitParameters(ParametersWhitelist):
        PARAMS = {'fd': (TypeAttr.from_type(typ.archi_file_descriptor_t),
                         lambda value: PrimitiveData(typ.archi_file_descriptor_t(value))),
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE,
                  'params': TypeAttr.from_type(typ.archi_file_map_params_t),
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

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__LIBRARY_HANDLE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pathname': _TYPE_STRING,
                  'params': TypeAttr.from_type(typ.archi_library_load_params_t),
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

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__HASHMAP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': TypeAttr.from_type(typ.archi_hashmap_alloc_params_t),
                  'capacity': _TYPE_SIZE}

    @classmethod
    def _slot_attr(cls, /, name, indices, setter, call):
        if indices or call:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE

        return TypeAttr.UNSPECIFIED

    @classmethod
    def _slot_unsettable(cls, /, name, indices):
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

    CONTEXT_TYPE = TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__TIMER)

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

        return getattr(cls._symbol_type_slot(library),
                       namespace + cls.full_name(name))(tag=cls.TAG)

    @classmethod
    def _symbol_type_slot(cls, /, library):
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

        if cls.TAG > TypeAttr.DATA_TAG_MAX:
            raise ValueError

    @classmethod
    def attributes(cls, /):
        """Get type attributes.
        """
        return TypeAttr.complex_data(cls.TAG)

    @classmethod
    def _symbol_type_slot(cls, /, library):
        return library.data


class FunctionSymbol(_Symbol):
    """Representation of an arbitrary function symbol.
    """
    def __init_subclass__(cls, /):
        """Initialize a subclass.
        """
        super().__init_subclass__()

        if cls.TAG > TypeAttr.FUNC_TAG_MAX:
            raise ValueError

    @classmethod
    def attributes(cls, /):
        """Get type attributes.
        """
        return TypeAttr.function(cls.TAG)

    @classmethod
    def _symbol_type_slot(cls, /, library):
        return library.function

### archi/context ###

class ContextInterfaceSymbol(DataSymbol):
    """Context interface symbol.
    """
    PREFIX = 'context_interface__'
    TAG = typ.ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE

### archi/aggr ###

class AggregateInterfaceSymbol(DataSymbol):
    """Aggregate type interface symbol.
    """
    PREFIX = 'aggr_interface__'
    TAG = typ.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE


class AggregateTypeSymbol(DataSymbol):
    """Aggregate type description symbol.
    """
    PREFIX = 'aggr_type__'
    TAG = typ.ARCHI_POINTER_DATA_TAG__AGGR_TYPE

### archi/exec ###

class DexgraphOperationFuncSymbol(FunctionSymbol):
    """DEG operation function symbol.
    """
    PREFIX = 'dexgraph_op__'
    TAG = typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION


class DexgraphOperationDataSymbol(DataSymbol):
    """DEG operation function data symbol.
    """
    PREFIX = 'dexgraph_op_data__'


class DexgraphTransitionFuncSymbol(FunctionSymbol):
    """DEG transition function symbol.
    """
    PREFIX = 'dexgraph_transition__'
    TAG = typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION


class DexgraphTransitionDataSymbol(DataSymbol):
    """DEG transition function data symbol.
    """
    PREFIX = 'dexgraph_transition_data__'

### archi/thread ###

class ThreadGroupWorkFuncSymbol(FunctionSymbol):
    """Thread group work function symbol.
    """
    PREFIX = 'thread_group_work__'
    TAG = typ.ARCHI_POINTER_FUNC_TAG__THREAD_WORK


class ThreadGroupWorkDataSymbol(DataSymbol):
    """Thread group work function data symbol.
    """
    PREFIX = 'thread_group_work_data__'


class ThreadGroupCallbackFuncSymbol(FunctionSymbol):
    """Thread group callback function symbol.
    """
    PREFIX = 'thread_group_callback__'
    TAG = typ.ARCHI_POINTER_FUNC_TAG__THREAD_WORK


class ThreadGroupCallbackDataSymbol(DataSymbol):
    """Thread group callback function data symbol.
    """
    PREFIX = 'thread_group_callback_data__'

### archi/signal ###

class SignalHandlerFuncSymbol(FunctionSymbol):
    """Signal handler function symbol.
    """
    PREFIX = 'signal_handler__'
    TAG = typ.ARCHI_POINTER_FUNC_TAG__SIGNAL_HANDLER


class SignalHandlerDataSymbol(DataSymbol):
    """Signal handler function data symbol.
    """
    PREFIX = 'signal_handler_data__'

### archi/memory ###

class MemoryInterfaceSymbol(DataSymbol):
    """Memory interface symbol.
    """
    PREFIX = 'memory_interface__'
    TAG = typ.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE


class MemoryAllocDataSymbol(DataSymbol):
    """Memory allocation function data symbol.
    """
    PREFIX = 'memory_alloc_data__'


class MemoryMapDataSymbol(DataSymbol):
    """Memory mapping function data symbol.
    """
    PREFIX = 'memory_map_data__'

