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
import random
import string

from .object import Object
from .context import (
        Parameters,
        _ContextInterface,
        _ContextSpec,
        Context,
        PointerContext,
        DataPointerArrayContext,
        )
import archi.context as ctx


class RegistryOperationList:
    """Implementation of the list of of registry operations.
    """
    def __init__(self, /):
        """Initialize the list of registry operations.
        """
        self._ops = []

    @property
    def list(self, /):
        """Get direct access to the list of registry operations.
        """
        return self._ops

    def clear(self, /):
        """Clear the list of registry operations.
        """
        self.list.clear()

    def pop(self, /):
        """Obtain the copy of the list of registry operations, clearing the original.
        """
        ops = self.list.copy()
        self.list.clear()
        return ops

    def append_op(self, op, data, /):
        """Append an operation to the list.
        """
        if not isinstance(op, str):
            raise TypeError
        elif not isinstance(data, Object):
            raise TypeError

        self.list.append((op, data))

    def temp_key(self, /, prefix, rnd_len=4):
        """Generate a temporary context key with the specified prefix and randomized postfix.
        """
        if not isinstance(prefix, str):
            raise TypeError
        elif not prefix:
            raise ValueError
        elif rnd_len < 0:
            raise ValueError

        postfix = ''.join(random.choice(string.ascii_letters + string.digits) \
                for _ in range(rnd_len))

        return f'.{prefix}_{len(self.list)}:{postfix}'

    def create(self, /, key, entity):
        """Create a context from an entity.
        """
        if not isinstance(key, str):
            raise TypeError

        if isinstance(entity, Context):
            return self.alias_context(key, entity)
        elif isinstance(entity, _ContextSpec):
            return self.create_context(key, entity)
        elif isinstance(entity, Parameters):
            return self.create_plist_context(key, entity)
        elif isinstance(entity, (type(None), Object, Context.Slot)):
            return self.create_ptr_context(key, entity)
        elif isinstance(entity, (tuple, list)):
            return self.create_dptr_array_context(key, entity)
        else:
            raise TypeError(f"Cannot create context from {entity}")

    def invoke(self, /, entity):
        """Invoke an entity (causing side effects only, no context created).
        """
        if isinstance(entity, Context.Slot):
            self.call_slot(entity)
        else:
            raise TypeError(f"Cannot invoke {entity}")

    def delete_context(self, /, key):
        """Append context deletion operation to the list.
        """
        from .object import RegistryOpData_delete

        self.append_op('delete', RegistryOpData_delete.construct(
            key=key))

    def alias_context(self, /, key, context):
        """Append context aliasing operation to the list.
        """
        from .object import RegistryOpData_alias

        self.append_op('alias', RegistryOpData_alias.construct(
            key=key,
            original_key=Context.key_of(context)))

        return context.__class__()

    def create_context(self, /, key, spec):
        """Append context creation operation(s) to the list.
        """
        from .object import RegistryOpData_create_as, RegistryOpData_create_from

        if not isinstance(spec, _ContextSpec):
            raise TypeError

        if isinstance(spec.interface_origin, Context):
            with self.parameters(spec.params) as (params_context_key, params_list):
                self.append_op('create_as', RegistryOpData_create_as.construct(
                    key=key,
                    sample_key=Context.key_of(spec.interface_origin),
                    init_params_context_key=params_context_key,
                    init_params_list=params_list))

        elif isinstance(spec.interface_origin, Context.Slot):
            slot = spec.interface_origin

            with self.parameters(spec.params) as (params_context_key, params_list):
                if not Context.Slot.is_call(slot):
                    temp_context_key = None

                    source_key = Context.key_of(Context.Slot.context_of(slot))
                    source_slot_name = Context.Slot.name_of(slot)
                    source_slot_indices = Context.Slot.indices_of(slot)
                else:
                    temp_context_key = self.temp_key('iface')

                    self.create_ptr_context(temp_context_key, slot)

                    source_key = temp_context_key
                    source_slot_name = ''
                    source_slot_indices = ()

                self.append_op('create_from', RegistryOpData_create_from.construct(
                    key=key,
                    source_key=source_key,
                    source_slot_name=source_slot_name,
                    source_slot_indices=source_slot_indices,
                    init_params_context_key=params_context_key,
                    init_params_list=params_list))

                if temp_context_key is not None:
                    self.delete_context(temp_context_key)

        else:
            raise TypeError

        return spec.context_cls()

    def create_plist_context(self, /, key, params):
        """Append parameter list creation operation(s) to the list.
        """
        with self.parameters(params) as (params_context_key, params_list):
            self.create_params(key, params_context_key, params_list)

        return params.__class__.Context()

    def create_ptr_context(self, /, key, entity):
        """Append pointer creation operation(s) to the list.
        """
        from .object import RegistryOpData_create_ptr

        if isinstance(entity, (type(None), Object)):
            self.append_op('create_ptr', RegistryOpData_create_ptr.construct(
                key=key,
                pointee=entity))

        elif isinstance(entity, Context.Slot):
            self.append_op('create_ptr', RegistryOpData_create_ptr.construct(
                key=key,
                pointee=None))

            self.set_slot(key, 'pointee', (), entity)

        else:
            raise TypeError

        return PointerContext()

    def create_dptr_array_context(self, /, key, seq):
        """Append data pointer array create operation(s) to the list.
        """
        from .object import RegistryOpData_create_dptr_array

        self.append_op('create_dptr_array', RegistryOpData_create_dptr_array.construct(
            key=key,
            length=len(seq)))

        for index, entity in enumerate(seq):
            if entity is not None:
                self.set_slot(key, '', (index,), entity)

        return DataPointerArrayContext()

    def unset_slot(self, /, context_key, slot_name, slot_indices):
        """Append slot unassignment operation to the list.
        """
        from .object import RegistryOpData_unassign

        self.append_op('unassign', RegistryOpData_unassign.construct(
            key=context_key,
            slot_name=slot_name,
            slot_indices=slot_indices))

    def set_slot(self, /, context_key, slot_name, slot_indices, value):
        """Append slot assignment operation to the list.
        """
        from .object import RegistryOpData_assign, RegistryOpData_assign_slot, RegistryOpData_assign_call

        if isinstance(value, (type(None), Object)):
            self.append_op('assign', RegistryOpData_assign.construct(
                key=context_key,
                slot_name=slot_name,
                slot_indices=slot_indices,
                value=value))

        elif isinstance(value, Context):
            self.append_op('assign_slot', RegistryOpData_assign_slot.construct(
                key=context_key,
                slot_name=slot_name,
                slot_indices=slot_indices,
                source_key=Context.key_of(value),
                source_slot_name='',
                source_slot_indices=()))

        elif isinstance(value, Context.Slot):
            if not Context.Slot.is_call(value):
                self.append_op('assign_slot' if not Context.Slot.is_weak_ref(value) else 'assign_slot_weak',
                               RegistryOpData_assign_slot.construct(
                                   key=context_key,
                                   slot_name=slot_name,
                                   slot_indices=slot_indices,
                                   source_key=Context.key_of(Context.Slot.context_of(value)),
                                   source_slot_name=Context.Slot.name_of(value),
                                   source_slot_indices=Context.Slot.indices_of(value)))

            else:
                with self.parameters(Context.Slot.call_params_of(value)) as (params_context_key, params_list):
                    self.append_op('assign_call' if not Context.Slot.is_weak_ref(value) else 'assign_call_weak',
                                   RegistryOpData_assign_call.construct(
                                       key=context_key,
                                       slot_name=slot_name,
                                       slot_indices=slot_indices,
                                       source_key=Context.key_of(Context.Slot.context_of(value)),
                                       source_slot_name=Context.Slot.name_of(value),
                                       source_slot_indices=Context.Slot.indices_of(value),
                                       source_call_params_context_key=params_context_key,
                                       source_call_params_list=params_list))

        else:
            raise TypeError

    def call_slot(self, /, slot):
        """Append context call invokation operation to the list.
        """
        from .object import RegistryOpData_invoke

        if not Context.Slot.is_call(slot):
            raise AttributeError(f"Slot {_slot_str(Context.Slot.name_of(slot), Context.Slot.indices_of(slot))} of context '{Context.key_of(Context.Slot.context_of(slot))}' is not a call slot")

        with self.parameters(Context.Slot.call_params_of(slot)) as (params_context_key, params_list):
            self.append_op('invoke', RegistryOpData_invoke.construct(
                key=Context.key_of(Context.Slot.context_of(slot)),
                slot_name=Context.Slot.name_of(slot),
                slot_indices=Context.Slot.indices_of(slot),
                call_params_context_key=params_context_key,
                call_params_list=params_list))

    def create_params(self, /, key, params_context_key, params_list):
        """Append parameter list context creation operation to the list.
        """
        from .object import RegistryOpData_create_params

        self.append_op('create_params', RegistryOpData_create_params.construct(
            key=key,
            params_context_key=params_context_key,
            params_list=params_list))

    @contextmanager
    def parameters(self, params, /):
        """Prepare the temporary parameter list if needed and append necessary
        list forming operations.
        """
        if not params.dynamic_params:
            yield (params.base_context_key, params.static_params)
        else:
            temp_context_key = self.temp_key('params')

            self.create_params(temp_context_key, params.base_context_key, params.static_params)

            try:
                for name, value in params.dynamic_params.items():
                    self.set_slot(temp_context_key, name, (), value)

                yield (temp_context_key, {})
            finally:
                self.delete_context(temp_context_key)


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

    # Dictionary of built-in contexts
    BUILTIN = {KEY_REGISTRY: ctx.HashmapContext,
               KEY_EXECUTABLE: ctx.LibraryContext,
               KEY_INPUT_FILE: ctx.FileMappingContext,
               KEY_SIGNAL_HANDLER: ctx.SignalHandlerDataHashmapContext}

    def __init__(self, /, operations=None, require_builtins=True, protect_builtins=True):
        """Initialize a context registry instance.
        """
        if not isinstance(operations, (type(None), RegistryOperationList)):
            raise TypeError
        elif not isinstance(require_builtins, bool):
            raise TypeError
        elif not isinstance(protect_builtins, bool):
            raise TypeError

        self._operations = operations if operations is not None else RegistryOperationList()

        self._contexts = {}
        self._prerequisites = {}

        if require_builtins:
            for key, context_cls in self.__class__.BUILTIN.items():
                self.require_context(key, cls=context_cls, protect=protect_builtins)

    def __getitem__(self, key, /):
        """Obtain a context from the context registry.
        """
        if key is None:
            return None
        elif not isinstance(key, str):
            raise TypeError

        return self._contexts[key]

    def __setitem__(self, key, entity, /):
        """Create a context and insert it to the registry.
        """
        if key is None:
            raise KeyError("Cannot create a context without a key")
        elif not isinstance(key, str):
            raise TypeError
        elif not key:
            raise KeyError(f"Cannot create a context with an empty key")
        elif key.startswith('.'):
            raise KeyError(f"Cannot create a context with a temporary key explicitly")
        elif key in self._contexts:
            raise KeyError(f"Context '{key}' is already in the registry")

        if not self.owns(entity):
            raise ValueError("The entity {entity} does not belong to the registry")

        context = self.operations.create(key, entity)

        if not isinstance(context, Context):
            raise TypeError
        elif context._.registry is not None:
            raise ValueError

        context._.registry = self
        context._.key = key

        self._contexts[key] = context

    def __delitem__(self, key, /):
        """Remove a context from the registry.
        """
        if key is None:
            return
        elif not isinstance(key, str):
            raise TypeError
        elif not key:
            raise KeyError(f"Cannot delete a context with an empty key")
        elif key.startswith('.'):
            raise KeyError(f"Cannot delete a context with a temporary key explicitly")
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

        self.operations.delete_context(key)

    def __call__(self, entity, /):
        """Invoke an entity (causing side effects only, context not created).
        """
        if not self.owns(entity):
            raise ValueError("The entity {entity} does not belong to the registry")

        self.operations.invoke(entity)

    def __contains__(self, key, /):
        """Check if a context with the specified key is in the registry.
        """
        if key is None:
            return False
        elif not isinstance(key, str):
            raise TypeError

        return key in self._contexts

    def __iter__(self, /):
        """Return an iterator.
        """
        return iter(self._contexts)

    def contexts(self, /, cls=Context, is_prereq=True, is_new=True):
        """Obtain the dictionary of known contexts of the specified type.
        """
        if not issubclass(cls, Context):
            raise TypeError
        elif not is_prereq and not is_new:
            raise ValueError

        return {key: context for key, context in self._contexts.items() if isinstance(context, cls) \
                and ((is_prereq and key in self._prerequisites) \
                or (is_new and key not in self._prerequisites))}

    @property
    def operations(self, /):
        """Get the current list of operations.
        """
        return self._operations

    def owns(self, entity, /):
        """Check if an entity belongs to the registry.
        """
        if isinstance(entity, Context):
            return Context.registry_of(entity) is self
        elif isinstance(entity, Context.Slot):
            return self.owns(Context.Slot.context_of(entity))
        elif isinstance(entity, _ContextSpec):
            return self.owns(entity.interface_origin)
        elif isinstance(entity, _ContextInterface):
            return self.owns(entity.origin)
        else:
            return True

    def is_prerequisite(self, key, /):
        """Check if a context key was added to the registry by require_context().
        """
        if key is None:
            return False
        elif not isinstance(key, str):
            raise TypeError

        return key in self._prerequisites

    def require_context(self, key, /, cls=Context, protect=True):
        """Require a context with the specified key to exist in the registry.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not issubclass(cls, Context):
            raise TypeError
        elif not isinstance(protect, bool):
            raise TypeError

        if key not in self:
            context = cls()
            context._.registry = self
            context._.key = key

            self._contexts[key] = context
            self._prerequisites[key] = protect

        else:
            context = self._contexts[key]

            if not isinstance(context, cls):
                raise TypeError(f"Required context '{key}' has type {context.__class__} (want {cls})")

        return context

    def require_builtin(self, key, /, protect=True):
        """Require a built-in context with the specified key to exist in the registry.
        """
        return self.require_context(key, cls=self.__class__.BUILTIN[key], protect=protect)

    def rekey_context(self, old_key, /, key):
        """Change key of a context.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not isinstance(old_key, str):
            raise TypeError

        if self._prerequisites.get(old_key, False):
            raise KeyError(f"Prerequisite context '{old_key}' is protected from key changing")

        context = self.new_context(self[old_key], key)
        del self[old_key]

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
        elif not self.owns(context):
            raise ValueError(f"Context '{Context.key_of(context)}' does not belong to the registry")

        del self[Context.key_of(context)]

    def cleanup(self, /):
        """Delete all created contexts from the registry.
        """
        for key in list(reversed(self.contexts(is_prereq=False).keys())):
            del self[key]

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
    def temp_context(self, entity, /, key):
        """Create a temporary context.
        """
        self[key] = entity
        with self.deleted_context(key) as context:
            yield context

    def interface_of(self, key, /):
        """Create a representation of a context interface.
        """
        if not isinstance(key, str):
            raise TypeError

        context = self[key]
        return _ContextInterface(context, context.__class__)

