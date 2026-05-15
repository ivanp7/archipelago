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
# @brief Archipelago application context registry.

from contextlib import contextmanager
from types import NoneType, SimpleNamespace

import archi.ctypes as typ
from .object import Object, RegistryOpList
import archi.object as obj
from .context import (
        TypeAttr,
        Context,
        _ContextSlotAssignment,
        ContextInterface,
        _ContextSpec,
        Parameters,
        )
import archi.context as ctx


class _RegistryImpl:
    """Implementation of registry operations.
    """
    def __init__(self, op_list, get_fn, /):
        """Initialize a registry operations implementation instance.
        """
        self._ops = op_list
        self._get_fn = get_fn

    @property
    def op_list(self, /):
        """Get the list of operations.
        """
        return self._ops

    @property
    def num_ops(self, /):
        """Get the number of operations in the list.
        """
        return len(self.op_list.list)

    def temp_key(self, label, /, prefix='', rnd_len=6, rnd_chars=None):
        """Generate a temporary context key with the specified label plus randomized part.
        """
        from random import choice
        from string import ascii_letters, digits

        if not isinstance(label, str):
            raise TypeError
        elif not label:
            raise ValueError
        elif not isinstance(prefix, str):
            raise TypeError
        elif rnd_len < 0:
            raise ValueError
        elif not isinstance(rnd_chars, (NoneType, str)):
            raise TypeError

        if rnd_chars is None:
            rnd_chars = ascii_letters + digits

        # generate a key until it's unique: guard against unwanted collisions
        rnd = ''.join(choice(rnd_chars) for _ in range(rnd_len))
        key = Registry.key(f'~{label}:{rnd} @ {self.num_ops}', prefix)

        try:
            self.context(key)
        except KeyError:
            return key
        else:
            raise KeyError("Failed to generate a unique temporary context key")

    def context(self, key, /):
        """Obtain the context with the specified key.
        """
        return self._get_fn(key)

    def check_context(self, context, /):
        """Check if a context exists and have the correct type.
        """
        if context is None:
            raise ValueError(f"No context is specified")

        reg_context = self.context(Context.key_of(context))

        if context != reg_context:
            raise TypeError(f"Context mismatch: {context} is not {reg_context}")

    def delete_context(self, key, /):
        """Delete a context.
        """
        self.op_list.op_delete(key)

    def alias_context(self, key, context, /):
        """Create a context alias.
        """
        self.check_context(context)

        original_key = Context.key_of(context)

        if key != original_key:
            self.op_list.op_alias(
                    key=key,
                    original_key=original_key)

            return context.__class__(key)
        else:
            return context

    def create_context(self, key, spec, /):
        """Create an arbitrary context.
        """
        with self.parameters(spec.params) as (params_key, params_list):
            if isinstance(spec.interface_origin, Context):
                self.check_context(spec.interface_origin)

                sample_key = Context.key_of(spec.interface_origin)

                self.op_list.op_create_as(
                        key=key,
                        sample_key=sample_key,
                        params_key=params_key,
                        params_list=params_list)

            elif isinstance(spec.interface_origin, Context.Slot):
                slot = spec.interface_origin

                self.check_context(Context.Slot.context_of(slot))

                source_key = Context.Slot.context_key_of(slot)

                if not TypeAttr.compatible(
                        TypeAttr.of(slot),
                        TypeAttr.complex_data(typ.ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE)):
                    raise TypeError(f"{slot} is not a context interface")

                if not Context.Slot.is_call(slot):
                    source_slot_name = Context.Slot.name_of(slot)
                    source_slot_indices = Context.Slot.indices_of(slot)
                else:
                    source_key = self.temp_key('interface')
                    self.create_ptr_context(source_key, slot)

                    source_slot_name = ''
                    source_slot_indices = ()

                self.op_list.op_create_from(
                        key=key,
                        source_key=source_key,
                        source_slot_name=source_slot_name,
                        source_slot_indices=source_slot_indices,
                        params_key=params_key,
                        params_list=params_list)

                if Context.Slot.is_call(slot):
                    self.delete_context(source_key)

            else:
                raise TypeError # should not happen

        return spec.context_cls(key)

    def create_plist_context(self, key, params, /):
        """Create a parameter list context.
        """
        with self.parameters(params) as (params_key, params_list):
            self.op_list.op_create_plist(
                    key=key,
                    params_key=params_key,
                    params_list=params_list)

        return params.__class__.Context(key)

    def create_ptr_context(self, key, pointee, /):
        """Create a pointer context.
        """
        ptr_context = ctx.PointerContext(key)

        if isinstance(pointee, (NoneType, Object)):
            self.op_list.op_create_ptr(
                    key=key,
                    pointee=pointee)

        elif isinstance(pointee, Context.Slot):
            self.op_list.op_create_ptr(
                    key=key,
                    pointee=None)

            self._set_slot(ptr_context.pointee, pointee)

        else:
            raise TypeError # should not happen

        return ptr_context

    def create_dptr_array_context(self, key, seq, /):
        """Create a data pointer array context.
        """
        dptr_array_context = ctx.DataPointerArrayContext(key)

        self.op_list.op_create_dptr_array(
                key=key,
                length=len(seq))

        for index, entity in enumerate(seq):
            if entity is not None:
                self._set_slot(dptr_array_context[index], entity)

        return dptr_array_context

    def unset_slot(self, slot, /):
        """Unset a context slot.
        """
        context = Context.Slot.context_of(slot)

        self.check_context(context)
        if Context.Slot.is_call(slot):
            raise AttributeError("{slot} is a call slot and cannot be unset")

        key = Context.Slot.context_key_of(slot)
        slot_name = Context.Slot.name_of(slot)
        slot_indices = Context.Slot.indices_of(slot)

        if not context.__class__.slot_unsettable(slot_name, slot_indices):
            raise AttributeError("{slot} is not unsettable")

        self.op_list.op_unassign(
                key=key,
                slot_name=slot_name,
                slot_indices=slot_indices)

    def set_slot(self, slot, entity, /):
        """Assign an entity to a context slot.
        """
        self.check_context(Context.Slot.context_of(slot))

        self._set_slot(slot, entity)

    def _set_slot(self, slot, entity, /):
        """Assign an entity to a context slot (without context check).
        """
        context = Context.Slot.context_of(slot)

        key = Context.Slot.context_key_of(slot)
        slot_name = Context.Slot.name_of(slot)
        slot_indices = Context.Slot.indices_of(slot)

        slot_attr = context.__class__.slot_attr(slot_name, slot_indices, setter=True)
        entity_attr = TypeAttr.of(entity)

        if entity_attr is NotImplemented:
            entity = context.__class__.slot_object(entity, slot_name, slot_indices)
            entity_attr = TypeAttr.of(entity)

        if not TypeAttr.compatible(entity_attr, slot_attr):
            raise TypeError(f"{slot}: incompatible assigned entity type")

        if isinstance(entity, (NoneType, Object)):
            self.op_list.op_assign(
                    key=key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    value=entity)

        elif isinstance(entity, Context):
            self.check_context(entity)

            source_key = Context.key_of(entity)

            self.op_list.op_assign_slot(
                    key=key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    source_key=source_key,
                    source_slot_name='',
                    source_slot_indices=(),
                    weak_ref=False)

        elif isinstance(entity, Context.Slot):
            self.check_context(Context.Slot.context_of(entity))

            source_key = Context.Slot.context_key_of(entity)
            source_slot_name = Context.Slot.name_of(entity)
            source_slot_indices = Context.Slot.indices_of(entity)
            source_slot_weak_ref = Context.Slot.is_weak_ref(entity)

            if not Context.Slot.is_call(entity):
                self.op_list.op_assign_slot(
                        key=key,
                        slot_name=slot_name,
                        slot_indices=slot_indices,
                        source_key=source_key,
                        source_slot_name=source_slot_name,
                        source_slot_indices=source_slot_indices,
                        weak_ref=source_slot_weak_ref)
            else:
                with self.parameters(Context.Slot.call_params_of(entity)) \
                        as (params_key, params_list):
                    self.op_list.op_assign_call(
                            key=key,
                            slot_name=slot_name,
                            slot_indices=slot_indices,
                            source_key=source_key,
                            source_slot_name=source_slot_name,
                            source_slot_indices=source_slot_indices,
                            params_key=params_key,
                            params_list=params_list,
                            weak_ref=source_slot_weak_ref)

        else:
            raise TypeError(f"Cannot assign {entity} to {slot}")

    def call_slot(self, slot, /):
        """Invoke a context slot call, discarding the return value.
        """
        context = Context.Slot.context_of(slot)

        self.check_context(context)
        if not Context.Slot.is_call(slot):
            raise AttributeError("{slot} is not a call slot")

        key = Context.Slot.context_key_of(slot)
        slot_name = Context.Slot.name_of(slot)
        slot_indices = Context.Slot.indices_of(slot)

        context.__class__.slot_attr(slot_name, slot_indices, call=True) # check if the slot exists

        with self.parameters(Context.Slot.call_params_of(slot)) as (params_key, params_list):
            self.op_list.op_invoke(
                    key=key,
                    slot_name=slot_name,
                    slot_indices=slot_indices,
                    params_key=params_key,
                    params_list=params_list)

    @contextmanager
    def parameters(self, params, /):
        """Parameter list context manager.
        Creates a temporary parameter list when necessary.
        """
        if params.base_context is not None:
            self.check_context(params.base_context)

        base_context_key = Context.key_of(params.base_context)
        dynamic_params = {}
        static_params = {}

        for key, value in params.dict.items():
            param_attr = params.__class__.param_attr(key)
            value_attr = TypeAttr.of(value)

            if value_attr is NotImplemented:
                value = params.__class__.param_object(value, key)
                value_attr = TypeAttr.of(value)

            if not TypeAttr.compatible(value_attr, param_attr):
                raise TypeError(f"{params}: incompatible value type for parameter {repr(key)}")

            if isinstance(value, (Context, Context.Slot)):
                dynamic_params[key] = value
            else:
                static_params[key] = value

        if not dynamic_params:
            yield (base_context_key, static_params)
        else:
            temp_context_key = self.temp_key('params')
            temp_context = params.__class__.Context(temp_context_key)

            self.op_list.op_create_plist(temp_context_key, base_context_key, static_params)

            for key, value in dynamic_params.items():
                self._set_slot(getattr(temp_context, key), value)

            yield (temp_context_key, {})

            self.delete_context(temp_context_key)


class Registry:
    """Representation of a context registry.
    """
    # Built-in contexts
    BUILTIN = SimpleNamespace(registry=ctx.HashmapContext('archi.registry'),
                              executable=ctx.LibraryContext('archi.executable'),
                              input_file=ctx.FileMappingContext('archi.input_file'),
                              signal_handler=ctx.SignalHandlerDataHashmapContext('archi.signal_handler'))

    # Input file contents key for lists of registry operations
    INPUT_FILE_KEY = 'reg_ops'

    # Implementation class
    IMPL = _RegistryImpl

    def __init__(self, /, operations=None, require=None):
        """Initialize a context registry instance.
        """
        if not isinstance(operations, (NoneType, RegistryOpList)):
            raise TypeError

        if operations is None:
            operations = RegistryOpList()

        self._contexts = {}
        self._prereq = {}

        self._impl = self.__class__.IMPL(operations, lambda key: self[key])

        if require is not None:
            for context in require:
                self.require(context)

    def copy(self, /):
        """Create a copy of the registry.
        """
        registry = self.__class__()

        registry._contexts = self._contexts.copy()
        registry._prereq = self._prereq.copy()

        registry.operations.list[:] = self.operations.list

        return registry

    @property
    def operations(self, /):
        """Get the current list of operations.
        """
        return self._impl.op_list

    @staticmethod
    def key(key, /, prefix=''):
        """Get a key with the specified prefix.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not isinstance(prefix, str):
            raise TypeError

        sep = '.' if prefix else ''
        return f'{prefix}{sep}{key}'

    def temp_key(self, label, /, prefix='', **kwargs):
        """Generate a temporary context key.
        """
        return self._impl.temp_key(label, prefix=prefix, **kwargs)

    def __getitem__(self, key, /):
        """Obtain a context from the context registry.
        """
        if key is None:
            return None

        self._check_key_used(key)

        return self._contexts[key]

    def __setitem__(self, key, entity, /):
        """Create a context and insert it to the registry.
        """
        self._check_key_available(key)

        context = self._create_context(key, entity)

        if context is NotImplemented:
            raise TypeError(f"Cannot create context from {entity}")

        if not isinstance(context, Context):
            raise TypeError(f"{context} is not a context")
        elif Context.key_of(context) != key:
            raise KeyError(f"{context} key is not {repr(key)}")

        self._contexts[key] = context

    def __delitem__(self, key, /):
        """Remove a context from the registry.
        """
        if key is None:
            return

        self._check_key_used(key)
        self._check_key_not_protected(key)

        self._impl.delete_context(key)

        if key in self._prereq:
            del self._prereq[key]
        del self._contexts[key]

    def __call__(self, operation, /):
        """Perform a registry operation with context slots.

        Accepts call slots and slot assignment chains (slot1 << slot2 << ... << slotN << entity).
        A call slot is invoked, its return value is discarded.
        In assignment chains, the order of operations is right-to-left:
            slotN   << entity
            slotN-1 << slotN
            ...
            slot2 << slot3
            slot1 << slot2

        The rightmost entity can be a value, a context, a getter slot, or a call slot.
        The leftmost slot can only be a setter slot.
        Other (middle) slots must be both getter and setter slots simultaneously.

        If the rightmost entity has the special value of `Context.Slot.UNSET`,
        all other slots in the chain are getting unset.
        """
        if self._operate(operation) is NotImplemented:
            raise TypeError(f"Unrecognized registry operation {operation}")

    def __contains__(self, key, /):
        """Check if a context with the specified key is in the registry.
        """
        return key in self._contexts

    def is_prereq(self, key, /):
        """Check if a context key was added to the registry by require().
        """
        return key in self._prereq

    def is_protected(self, key, /):
        """Check if a context is protected from deletion or rekeying.
        """
        return self._prereq.get(key, False)

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

    def contexts(self, /, cls=Context, new=True, prereq_unprot=True, prereq_prot=True):
        """Obtain the dictionary of known contexts of the specified type.
        """
        if not issubclass(cls, Context):
            raise TypeError
        elif not isinstance(new, bool):
            raise TypeError
        elif not isinstance(prereq_unprot, bool):
            raise TypeError
        elif not isinstance(prereq_prot, bool):
            raise TypeError

        if not new and not prereq_unprot and not prereq_prot:
            return {}

        def passes_filter(key, context, /):
            if not isinstance(context, cls):
                return False
            try:
                protected = self._prereq[key]
                return (prereq_prot and protected) or (prereq_unprot and not protected)
            except KeyError:
                return new

        return {key: context for key, context in self._contexts.items()
                if passes_filter(key, context)}

    def require(self, context, /, protect=True):
        """Require a context with the specified key to exist in the registry.
        """
        if context is None:
            return

        if not isinstance(context, Context):
            raise TypeError
        elif not isinstance(protect, bool):
            raise TypeError

        key = Context.key_of(context)

        if key not in self:
            self._contexts[key] = context
            self._prereq[key] = protect
        else:
            if context != self._contexts[key]:
                raise TypeError(f"Required context mismatch: want {context}, have {self._contexts[key]}")
            elif protect != self.is_protected(key):
                raise ValueError(f"Required context protection mode mismatch: want {protect}, have {self.is_protected(key)}")

    def delete(self, context, /):
        """Delete a context from the registry.
        """
        if context is None:
            return

        key = Context.key_of(context)

        if context != self._contexts[key]:
            raise TypeError(f"Context mismatch: {context} is not {self._contexts[key]}")

        del self[key]

    def cleanup(self, /, prereq=False):
        """Delete all new (and, optionally, unprotected prerequisite) contexts
        from the registry.
        """
        for key in tuple(reversed(self.contexts(prereq_unprot=prereq,
                                                prereq_prot=False).keys())):
            del self[key]

    def rekey_context(self, key, original_key, /):
        """Change key of a context.
        """
        self._check_key_used(original_key)
        self._check_key_not_protected(original_key)
        self._check_key_available(key)

        self[key] = self[original_key]
        del self[original_key]

        return self[key]

    def new_context(self, key, entity, /):
        """Create a new context and add it to the registry.
        """
        self[key] = entity
        return self[key]

    @contextmanager
    def deleted_context(self, key, /):
        """Obtain a context and delete it afterwards.
        """
        context = self[key]
        try:
            yield context
        finally:
            del self[key]

    @contextmanager
    def temp_context(self, key, entity, /):
        """Create a temporary context.
        """
        self[key] = entity
        with self.deleted_context(key) as context:
            yield context

    def interface_of(self, key, /):
        """Create a representation of a context interface.
        """
        context = self[key]
        return ContextInterface(context, context.__class__)

    def _check_key_available(self, key, /):
        """Check if a key is available to be used by a new context.
        """
        if not isinstance(key, str):
            raise TypeError("Context key must be a string")
        elif key in self:
            raise KeyError(f"Context {repr(key)} is in the registry already")

    def _check_key_used(self, key, /):
        """Check if a context with the specified key exists in the registry.
        """
        if key not in self:
            raise KeyError(f"Context {repr(key)} is not in the registry")

    def _check_key_not_protected(self, key, /):
        """Check if a context is protected from deletion.
        """
        if self.is_protected(key):
            raise KeyError(f"Prerequisite context {repr(key)} is protected")

    def _create_context(self, key, entity, /):
        """Create a context from an entity.

        This method may be overridden in derived classes.
        """
        if isinstance(entity, Context):
            return self._impl.alias_context(key, entity)

        elif isinstance(entity, _ContextSpec):
            return self._impl.create_context(key, entity)

        elif isinstance(entity, Parameters):
            return self._impl.create_plist_context(key, entity)

        elif isinstance(entity, (NoneType, Object, Context.Slot)):
            return self._impl.create_ptr_context(key, entity)

        elif isinstance(entity, (tuple, list)):
            return self._impl.create_dptr_array_context(key, entity)

        else:
            return NotImplemented

    def _operate(self, operation, /):
        """Perform a registry operation.

        This method may be overridden in derived classes.
        """
        if isinstance(operation, Context.Slot):
            self._impl.call_slot(operation)

        elif isinstance(operation, _ContextSlotAssignment):
            if operation.chain[-1] is Context.Slot.UNSET:
                for i in range(len(operation.chain), 1, -1):
                    self._impl.unset_slot(operation.chain[i-2])

            else:
                for i in range(len(operation.chain), 1, -1):
                    self._impl.set_slot(operation.chain[i-2], operation.chain[i-1])

        else:
            return NotImplemented

