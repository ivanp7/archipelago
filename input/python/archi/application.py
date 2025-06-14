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
# @brief Representation of Archipelago application initialization files.

import ctypes as c

from .memory import CValue, MemoryBlock, Memory


class Parameters:
    """Representation of a parameter list.
    """
    def __init__(self, _: "Context" = None, /, **params):
        """Create a context parameter list representation instance.
        """
        if _ is not None and not isinstance(_, Context):
            raise TypeError("Dynamic parameter list object must be of type Context or None")

        self._dparams = _
        self._sparams = dict(reversed(params.items())) # archi reverses order when copies the list,
                                                       # we must counter this here

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
            raise TypeError("Context interface object must be of type ContextInterface")
        elif not isinstance(params, Parameters):
            raise TypeError("Parameter list object must be of type Parameters")

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
            raise TypeError("Context interface can be created from Context and Context._Slot objects only")

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
                raise TypeError("Context object must be of type Context")
            elif not isinstance(name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(indices, list) \
                    or not all(isinstance(index, int) for index in indices):
                raise TypeError("Slot indices must be a list of integers")

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
                raise TypeError("Slot index must be an integer")

            return Context._Slot(self._context, self._name, self._indices + [index])

        def __setattr__(self, name: "str", value):
            """Perform a slot setting operation.
            """
            self._context._set(f'{self._name}.{name}', self._indices, value)

        def __setitem__(self, index: "int", value):
            """Perform a slot setting operation.
            """
            if not isinstance(index, int):
                raise TypeError("Context slot index must be an integer")

            self._context._set(self._name, self._indices + [index], value)

        def __call__(self, _: "Context" = None, /, **params) -> "Context._Action":
            """Perform an action.
            """
            if _ is not None and not isinstance(_, Context):
                raise TypeError("Dynamic parameter list object must be of type Context")

            self._context._act(self._name, self._indices, dparams=_,
                               sparams=dict(reversed(params.items()))) # archi reverses order when copies the list,
                                                                       # we must counter this here
            return Context._Action(self._context, self._name, self._indices)

    class _Action:
        """Representation of a context action.
        """
        def __init__(self, context: "Context", name: "str", indices: "list[int]" = []):
            """Create a context action representation instance.
            """
            if not isinstance(context, Context):
                raise TypeError("Context object must be of type Context")
            elif not isinstance(name, str):
                raise TypeError("Action name must be a string")
            elif not isinstance(indices, list) \
                    or not all(isinstance(index, int) for index in indices):
                raise TypeError("Action indices must be a list of integers")

            self._context = context
            self._name = name
            self._indices = indices

    def __init__(self, app: "Application", key: "str"):
        """Create a context representation instance.
        """
        if not isinstance(app, Application):
            raise TypeError("Application object must be of type Application")
        elif not isinstance(key, str):
            raise TypeError("Key must be a string")

        object.__setattr__(self, '_app', app)
        object.__setattr__(self, '_key', key)

    def __getattr__(self, name: "str") -> "Context._Slot":
        """Obtain a context slot object.
        """
        return Context._Slot(self, name)

    def __getitem__(self, index: "int") -> "Context._Slot":
        """Obtain a context slot object.
        """
        if not isinstance(index, int):
            raise TypeError("Slot index must be an integer")

        return Context._Slot(self, indices=[index])

    def __setattr__(self, name: "str", value):
        """Perform a slot setting operation.
        """
        self._set(name, [], value)

    def __setitem__(self, index: "int", value):
        """Perform a slot setting operation.
        """
        if not isinstance(index, int):
            raise TypeError("Context slot index must be an integer")

        self._set('', [index], value)

    def __call__(self, _: "Context" = None, /, **params) -> "Context._Action":
        """Perform an action.
        """
        if _ is not None and not isinstance(_, Context):
            raise TypeError("Dynamic parameter list object must be of type Context")

        self._act('', [], dparams=_,
                  sparams=dict(reversed(params.items()))) # archi reverses order when copies the list,
                                                          # we must counter this here
        return Context._Action(self, '', [])

    def _set(self, slot_name: "str", slot_indices: "list[int]", value):
        """Append a set() instruction to the list.
        """
        if not isinstance(slot_name, str):
            raise TypeError("Slot name must be a string")
        elif not isinstance(slot_indices, list) \
                or not all(isinstance(index, int) for index in slot_indices):
            raise TypeError("Slot indices must be a list of integers")

        if value is None or isinstance(value, CValue):
            self._app._instructions.append(Application._InstructionSetToValue(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                value=value))

        elif isinstance(value, Context):
            self._app._instructions.append(Application._InstructionSetToContextData(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                source_key=value._key))

        elif isinstance(value, Context._Slot) or isinstance(value, Context._Action):
            self._app._instructions.append(Application._InstructionSetToContextSlot(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                source_key=value._context._key,
                source_slot_name=value._name, source_slot_indices=value._indices))

        else:
            self._app._instructions.append(Application._InstructionSetToValue(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                value=CValue(value)))

    def _act(self, action_name: "str", action_indices: "list[int]",
             dparams: "Context", sparams: "dict"):
        """Append an act() instruction to the list.
        """
        if not isinstance(action_name, str):
            raise TypeError("Action name must be a string")
        elif not isinstance(action_indices, list) \
                or not all(isinstance(index, int) for index in action_indices):
            raise TypeError("Action indices must be a list of integers")
        elif dparams is not None and not isinstance(dparams, Context):
            raise TypeError("Dynamic parameter list object must be of type Context")
        elif not isinstance(sparams, dict) \
                or not all(isinstance(key, str) for key in sparams.keys()):
            raise TypeError("Static parameter list a dictionary with string keys")

        self._app._instructions.append(Application._InstructionAct(
            key=self._key,
            action_name=action_name, action_indices=action_indices,
            dparams_key=dparams._key, sparams=sparams))

    @staticmethod
    def key_of(context: "Context") -> "str":
        """Obtain key of a context.
        """
        if not isinstance(context, Context):
            raise TypeError("Context object must be of type Context")

        return context._key

    @staticmethod
    def application_of(context: "Context") -> "str":
        """Obtain owning application of a context.
        """
        if not isinstance(context, Context):
            raise TypeError("Context object must be of type Context")

        return context._app

###############################################################################

class Application:
    """Representation of a whole application initialization file.
    """
    class _Instruction:
        """Representation of an abstract application initialization instruction.
        """
        NOOP, INIT_FROM_CONTEXT, INIT_FROM_SLOT, \
                INIT_POINTER, INIT_DATA_ARRAY, INIT_FUNC_ARRAY, COPY, FINAL, \
                SET_TO_VALUE, SET_TO_CONTEXT_DATA, SET_TO_CONTEXT_SLOT, ACT = range(12)

    class _InstructionNoop(_Instruction):
        """Representation of an application initialization instruction: no-op.
        """
        def __init__(self):
            """Initialize an instruction.
            """
            self._type = Application._Instruction.NOOP

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_base_t

            instr = archi_exe_registry_instr_base_t()
            instr.type = self._type

            return MemoryBlock(CValue(instr))

    class _InstructionInitFromContext(_Instruction):
        """Representation of an application initialization instruction:
        initialize a new context using interface of a source context.
        """
        def __init__(self, key: "str", interface_source_key: "str",
                     dparams_key: "str", sparams: "dict"):
            """Initialize an instruction.
            """
            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif not isinstance(interface_source_key, str):
                raise TypeError("Interface source context key must be a string")
            elif dparams_key is not None and not isinstance(dparams_key, str):
                raise TypeError("Dynamic parameter list key must be a string")
            elif not isinstance(sparams, dict):
                raise TypeError("Static parameter list must be of type dict")

            self._type = Application._Instruction.INIT_FROM_CONTEXT
            self._key = key
            self._interface_source_key = interface_source_key
            self._dparams_key = dparams_key
            self._sparams = sparams

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_init_from_context_t

            instr = archi_exe_registry_instr_init_from_context_t()
            instr.base.type = self._type

            ptr_key = app._alloc_string(self._key)

            ptr_interface_source_key = app._alloc_string(self._interface_source_key) \
                    if self._interface_source_key is not None else None

            ptr_dparams_key = app._alloc_string(self._dparams_key) \
                    if self._dparams_key is not None else None
            ptr_sparams = app._alloc_params(self._sparams)

            def init_instr(instr: "archi_exe_registry_instr_init_from_context_t"):
                instr.key = ptr_key.address()

                if ptr_interface_source_key is not None:
                    instr.interface_source_key = ptr_interface_source_key.address()

                if ptr_dparams_key is not None:
                    instr.dparams_key = ptr_dparams_key.address()
                if ptr_sparams is not None:
                    instr.sparams = c.cast(ptr_sparams.address(), type(instr.sparams))

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionInitFromSlot(_Instruction):
        """Representation of an application initialization instruction:
        initialize a new context using interface from a context slot.
        """
        def __init__(self, key: "str", interface_source_key: "str",
                     interface_source_slot_name: "str", interface_source_slot_indices: "list[int]",
                     dparams_key: "str", sparams: "dict"):
            """Initialize an instruction.
            """
            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif not isinstance(interface_source_key, str):
                raise TypeError("Interface source context key must be a string")
            elif not isinstance(interface_source_slot_name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(interface_source_slot_indices, list) \
                    or not all(isinstance(index, int) for index in interface_source_slot_indices):
                raise TypeError("Slot indices must be a list of integers")
            elif dparams_key is not None and not isinstance(dparams_key, str):
                raise TypeError("Dynamic parameter list key must be a string")
            elif not isinstance(sparams, dict):
                raise TypeError("Static parameter list must be of type dict")

            self._type = Application._Instruction.INIT_FROM_SLOT
            self._key = key
            self._interface_source_key = interface_source_key
            self._interface_source_slot_name = interface_source_slot_name
            self._interface_source_slot_indices = interface_source_slot_indices
            self._dparams_key = dparams_key
            self._sparams = sparams

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_init_from_slot_t

            instr = archi_exe_registry_instr_init_from_slot_t()
            instr.base.type = self._type

            ptr_key = app._alloc_string(self._key)

            ptr_interface_source_key = app._alloc_string(self._interface_source_key)
            ptr_interface_source_slot_name = app._alloc_string(self._interface_source_slot_name)
            ptr_interface_source_slot_indices = app._alloc_index_array(self._interface_source_slot_indices)

            ptr_dparams_key = app._alloc_string(self._dparams_key) \
                    if self._dparams_key is not None else None
            ptr_sparams = app._alloc_params(self._sparams)

            def init_instr(instr: "archi_exe_registry_instr_init_from_slot_t"):
                instr.key = ptr_key.address()

                instr.interface_source_key = ptr_interface_source_key.address()
                instr.interface_source_slot.name = ptr_interface_source_slot_name.address()
                if ptr_interface_source_slot_indices is not None:
                    instr.interface_source_slot.index = c.cast(ptr_interface_source_slot_indices.address(),
                                                               type(instr.interface_source_slot.index))

                if ptr_dparams_key is not None:
                    instr.dparams_key = ptr_dparams_key.address()
                if ptr_sparams is not None:
                    instr.sparams = c.cast(ptr_sparams.address(), type(instr.sparams))

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionInitPointer(_Instruction):
        """Representation of an application initialization instruction:
        initialize a new pointer context.
        """
        def __init__(self, key: "str", value: "CValue"):
            """Initialize an instruction.
            """
            from .ctypes.common import archi_pointer_t

            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif value is not None and not isinstance(value, CValue):
                raise TypeError("Object assigned to a slot must be of type CValue")

            self._type = Application._Instruction.INIT_POINTER
            self._key = key
            self._value = value

            if value is not None:
                self._flags = value.attributes().get('flags', 0)
                if self._flags >= 1 << archi_pointer_t.NUM_FLAG_BITS:
                    raise ValueError(f"Flags must fit into {archi_pointer_t.NUM_FLAG_BITS} lowest bits")
            else:
                self._flags = 0

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_init_pointer_t

            instr = archi_exe_registry_instr_init_pointer_t()
            instr.base.type = self._type
            if self._value is not None:
                instr.value.flags = self._flags
                instr.value.element.num_of = self._value.num_elements()
                instr.value.element.size = self._value.element_size()
                instr.value.element.alignment = self._value.element_alignment()

            ptr_key = app._alloc_string(self._key)

            ptr_value = app._alloc_value(self._value)

            def init_instr(instr: "archi_exe_registry_instr_init_pointer_t"):
                instr.key = ptr_key.address()

                if ptr_value is not None:
                    instr.value.ptr = ptr_value.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionInitDataArray(_Instruction):
        """Representation of an application initialization instruction:
        initialize a new array context.
        """
        def __init__(self, key: "str", num_elements: "int", flags: "int"):
            """Initialize an instruction.
            """
            from .ctypes.common import archi_pointer_t

            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif not isinstance(num_elements, int):
                raise TypeError("Number of array elements must be a non-negative integer")
            elif num_elements < 0:
                raise ValueError("Number of array elements must be a non-negative integer")
            elif flags is not None and not isinstance(flags, int):
                raise TypeError("Flags must be a non-negative integer")
            elif flags is not None and flags < 0:
                raise ValueError("Flags must be a non-negative integer")
            elif flags is not None and flags >= (1 << archi_pointer_t.NUM_FLAG_BITS):
                raise ValueError(f"Flags must fit into {archi_pointer_t.NUM_FLAG_BITS} lowest bits")

            self._type = Application._Instruction.INIT_DATA_ARRAY
            self._key = key
            self._num_elements = num_elements
            self._flags = flags

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_init_array_t

            instr = archi_exe_registry_instr_init_array_t()
            instr.base.type = self._type

            instr.num_elements = self._num_elements
            if self._flags is not None:
                instr.flags = self._flags

            ptr_key = app._alloc_string(self._key)

            def init_instr(instr: "archi_exe_registry_instr_init_array_t"):
                instr.key = ptr_key.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    # class _InstructionInitFuncArray is not implemented yet

    class _InstructionCopy(_Instruction):
        """Representation of an application initialization instruction:
        create a context alias.
        """
        def __init__(self, key: "str", original_key: "str"):
            """Initialize an instruction.
            """
            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif not isinstance(original_key, str):
                raise TypeError("Original context key must be a string")

            self._type = Application._Instruction.COPY
            self._key = key
            self._original_key = original_key

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_copy_t

            instr = archi_exe_registry_instr_copy_t()
            instr.base.type = self._type

            ptr_key = app._alloc_string(self._key)

            ptr_original_key = app._alloc_string(self._original_key)

            def init_instr(instr: "archi_exe_registry_instr_copy_t"):
                instr.key = ptr_key.address()

                instr.original_key = ptr_original_key.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionFinal(_Instruction):
        """Representation of an application initialization instruction: finalize a context.
        """
        def __init__(self, key: "str"):
            """Initialize an instruction.
            """
            if not isinstance(key, str):
                raise TypeError("Context key must be a string")

            self._type = Application._Instruction.FINAL
            self._key = key

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_final_t

            instr = archi_exe_registry_instr_final_t()
            instr.base.type = self._type

            ptr_key = app._alloc_string(self._key)

            def init_instr(instr: "archi_exe_registry_instr_final_t"):
                instr.key = ptr_key.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionSetToValue(_Instruction):
        """Representation of an application initialization instruction:
        set a value to a context slot.
        """
        def __init__(self, key: "str", slot_name: "str", slot_indices: "list[int]",
                     value: "CValue"):
            """Initialize an instruction.
            """
            from .ctypes.common import archi_pointer_t

            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif not isinstance(slot_name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(slot_indices, list) \
                    or not all(isinstance(index, int) for index in slot_indices):
                raise TypeError("Slot indices must be a list of integers")
            elif value is not None and not isinstance(value, CValue):
                raise TypeError("Object assigned to a slot must be of type CValue")

            self._type = Application._Instruction.SET_TO_VALUE
            self._key = key
            self._slot_name = slot_name
            self._slot_indices = slot_indices
            self._value = value

            if value is not None:
                self._flags = value.attributes().get('flags', 0)
                if self._flags >= 1 << archi_pointer_t.NUM_FLAG_BITS:
                    raise ValueError(f"Flags must fit into {archi_pointer_t.NUM_FLAG_BITS} lowest bits")

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_set_to_value_t

            instr = archi_exe_registry_instr_set_to_value_t()
            instr.base.type = self._type
            instr.slot.num_indices = len(self._slot_indices)
            if self._value is not None:
                instr.value.flags = self._flags
                instr.value.element.num_of = self._value.num_elements()
                instr.value.element.size = self._value.element_size()
                instr.value.element.alignment = self._value.element_alignment()

            ptr_key = app._alloc_string(self._key)

            ptr_slot_name = app._alloc_string(self._slot_name)
            ptr_slot_indices = app._alloc_index_array(self._slot_indices)
            ptr_value = app._alloc_value(self._value)

            def init_instr(instr: "archi_exe_registry_instr_set_to_value_t"):
                instr.key = ptr_key.address()

                instr.slot.name = ptr_slot_name.address()
                if ptr_slot_indices is not None:
                    instr.slot.index = c.cast(ptr_slot_indices.address(), type(instr.slot.index))

                if ptr_value is not None:
                    instr.value.ptr = ptr_value.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionSetToContextData(_Instruction):
        """Representation of an application initialization instruction:
        set a source context pointer to a context slot.
        """
        def __init__(self, key: "str", slot_name: "str", slot_indices: "list[int]",
                     source_key: "str"):
            """Initialize an instruction.
            """
            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif not isinstance(slot_name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(slot_indices, list) \
                    or not all(isinstance(index, int) for index in slot_indices):
                raise TypeError("Slot indices must be a list of integers")
            elif not isinstance(source_key, str):
                raise TypeError("Source context key must be a string")

            self._type = Application._Instruction.SET_TO_CONTEXT_DATA
            self._key = key
            self._slot_name = slot_name
            self._slot_indices = slot_indices
            self._source_key = source_key

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_set_to_context_data_t

            instr = archi_exe_registry_instr_set_to_context_data_t()
            instr.base.type = self._type
            instr.slot.num_indices = len(self._slot_indices)

            ptr_key = app._alloc_string(self._key)

            ptr_slot_name = app._alloc_string(self._slot_name)
            ptr_slot_indices = app._alloc_index_array(self._slot_indices)
            ptr_source_key = app._alloc_string(self._source_key)

            def init_instr(instr: "archi_exe_registry_instr_set_to_context_data_t"):
                instr.key = ptr_key.address()

                instr.slot.name = ptr_slot_name.address()
                if ptr_slot_indices is not None:
                    instr.slot.index = c.cast(ptr_slot_indices.address(), type(instr.slot.index))
                instr.source_key = ptr_source_key.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionSetToContextSlot(_Instruction):
        """Representation of an application initialization instruction:
        set a source context slot to a context slot.
        """
        def __init__(self, key: "str", slot_name: "str", slot_indices: "list[int]",
                     source_key: "str", source_slot_name: "str", source_slot_indices: "list[int]"):
            """Initialize an instruction.
            """
            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif not isinstance(slot_name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(slot_indices, list) \
                    or not all(isinstance(index, int) for index in slot_indices):
                raise TypeError("Slot indices must be a list of integers")
            elif not isinstance(source_key, str):
                raise TypeError("Source context key must be a string")
            elif not isinstance(source_slot_name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(source_slot_indices, list) \
                    or not all(isinstance(index, int) for index in source_slot_indices):
                raise TypeError("Slot indices must be a list of integers")

            self._type = Application._Instruction.SET_TO_CONTEXT_SLOT
            self._key = key
            self._slot_name = slot_name
            self._slot_indices = slot_indices
            self._source_key = source_key
            self._source_slot_name = source_slot_name
            self._source_slot_indices = source_slot_indices

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_set_to_context_slot_t

            instr = archi_exe_registry_instr_set_to_context_slot_t()
            instr.base.type = self._type
            instr.slot.num_indices = len(self._slot_indices)
            instr.source_slot.num_indices = len(self._source_slot_indices)

            ptr_key = app._alloc_string(self._key)

            ptr_slot_name = app._alloc_string(self._slot_name)
            ptr_slot_indices = app._alloc_index_array(self._slot_indices)
            ptr_source_key = app._alloc_string(self._source_key)
            ptr_source_slot_name = app._alloc_string(self._source_slot_name)
            ptr_source_slot_indices = app._alloc_index_array(self._source_slot_indices)

            def init_instr(instr: "archi_exe_registry_instr_set_to_context_slot_t"):
                instr.key = ptr_key.address()

                instr.slot.name = ptr_slot_name.address()
                if ptr_slot_indices is not None:
                    instr.slot.index = c.cast(ptr_slot_indices.address(), type(instr.slot.index))
                instr.source_key = ptr_source_key.address()
                instr.source_slot.name = ptr_source_slot_name.address()
                if ptr_source_slot_indices is not None:
                    instr.source_slot.index = c.cast(ptr_source_slot_indices.address(), type(instr.source_slot.index))

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionAct(_Instruction):
        """Representation of an application initialization instruction:
        invoke a context action.
        """
        def __init__(self, key: "str",
                     action_name: "str", action_indices: "list[int]", dparams_key: "str", sparams: "dict"):
            """Initialize an instruction.
            """
            if not isinstance(key, str):
                raise TypeError("Context key must be a string")
            elif not isinstance(action_name, str):
                raise TypeError("Action name must be a string")
            elif not isinstance(action_indices, list) \
                    or not all(isinstance(index, int) for index in action_indices):
                raise TypeError("Action indices must be a list of integers")
            elif dparams_key is not None and not isinstance(dparams_key, str):
                raise TypeError("Dynamic parameter list key must be a string")
            elif not isinstance(sparams, dict):
                raise TypeError("Static parameter list must be of type dict")

            self._type = Application._Instruction.ACT
            self._key = key
            self._action_name = action_name
            self._action_indices = action_indices
            self._dparams_key = dparams_key
            self._sparams = sparams

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            from .ctypes.instructions import archi_exe_registry_instr_act_t

            instr = archi_exe_registry_instr_act_t()
            instr.base.type = self._type
            instr.action.num_indices = len(self._action_indices)

            ptr_key = app._alloc_string(self._key)

            ptr_action_name = app._alloc_string(self._action_name)
            ptr_action_indices = app._alloc_index_array(self._action_indices)

            ptr_dparams_key = app._alloc_string(self._dparams_key) \
                    if self._dparams_key is not None else None
            ptr_sparams = app._alloc_params(self._sparams)

            def init_instr(instr: "archi_exe_registry_instr_act_t"):
                instr.key = ptr_key.address()

                instr.action.name = ptr_action_name.address()
                if ptr_action_indices is not None:
                    instr.action.index = c.cast(ptr_action_indices.address(), type(instr.action.index))

                if ptr_dparams_key is not None:
                    instr.dparams_key = ptr_dparams_key.address()
                if ptr_sparams is not None:
                    instr.sparams = ptr_sparams.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    KEY_REGISTRY    = 'archi.registry'
    KEY_EXECUTABLE  = 'archi.executable'
    KEY_INPUT_FILE  = 'archi.input_file'
    KEY_SIGNAL      = 'archi.signal'

    CONTENT_INSTRUCTIONS = 'archi.instructions'
    CONTENT_SIGNALS      = 'archi.signals'

    def __init__(self):
        """Initialize a file.
        """
        self._contents = {}
        self._instructions = []
        self._blocks = {} # CValue -> MemoryBlock

        self._ptr_blocks = []
        self._ptr_values = {}       # CValue -> MemoryBlock
        self._ptr_strings = {}      # str -> MemoryBlock
        self._ptr_index_arrays = {} # list[int] -> MemoryBlock

    def __getitem__(self, key: "str") -> "Context":
        """Obtain a context from the registry.
        """
        if not isinstance(key, str):
            raise TypeError("Key must be a string")

        return Context(self, key)

    def __setitem__(self, key: "str", entity):
        """Initialize a context and insert it to the registry.
        """
        if not isinstance(key, str):
            raise TypeError("Key must be a string")

        if isinstance(entity, Parameters):
            dparams_key = entity.dynamic_list()._key \
                    if entity.dynamic_list() is not None else None

            self._instructions.append(Application._InstructionInitFromContext(
                key=key,
                interface_source_key=None,
                dparams_key=dparams_key, sparams=entity.static_list()))

        elif isinstance(entity, ContextSpec):
            source = entity.interface().source()

            if isinstance(source, Context):
                dparams_key = entity.parameters().dynamic_list()._key \
                        if entity.parameters().dynamic_list() is not None else None

                self._instructions.append(Application._InstructionInitFromContext(
                    key=key,
                    interface_source_key=source._key,
                    dparams_key=dparams_key, sparams=entity.parameters().static_list()))

            elif isinstance(source, Context._Slot):
                dparams_key = entity.parameters().dynamic_list()._key \
                        if entity.parameters().dynamic_list() is not None else None

                self._instructions.append(Application._InstructionInitFromSlot(
                    key=key,
                    interface_source_key=source._context._key,
                    interface_source_slot_name=source._name, interface_source_slot_indices=source._indices,
                    dparams_key=dparams_key, sparams=entity.parameters().static_list()))

        elif isinstance(entity, Context):
            self._instructions.append(Application._InstructionCopy(
                key=key,
                original_key=entity._key))

        elif isinstance(entity, list):
            self._instructions.append(Application._InstructionInitDataArray(
                key=key,
                num_elements=len(entity),
                flags=None))

            for index, element in enumerate(entity):
                if isinstance(element, Context):
                    self._instructions.append(Application._InstructionSetToContextData(
                        key=key,
                        slot_name='', slot_indices=[index],
                        source_key=element._key))

                elif isinstance(element, Context._Slot) or isinstance(element, Context._Action):
                    self._instructions.append(Application._InstructionSetToContextSlot(
                        key=key,
                        slot_name='', slot_indices=[index],
                        source_key=element._context._key,
                        source_slot_name=element._name, source_slot_indices=element._indices))

                elif element is None or isinstance(element, CValue):
                    self._instructions.append(Application._InstructionSetToValue(
                        key=key,
                        slot_name='', slot_indices=[index],
                        value=element))

                else:
                    self._instructions.append(Application._InstructionSetToValue(
                        key=key,
                        slot_name='', slot_indices=[index],
                        value=CValue(element)))

        elif isinstance(entity, Context._Slot) or isinstance(entity, Context._Action):
            self._instructions.append(Application._InstructionInitPointer(
                key=key,
                value=None))

            self._instructions.append(Application._InstructionSetToContextSlot(
                key=key,
                slot_name='value', slot_indices=[],
                source_key=entity._context._key,
                source_slot_name=entity._name, source_slot_indices=entity._indices))

        elif entity is None or isinstance(entity, CValue):
            self._instructions.append(Application._InstructionInitPointer(
                key=key,
                value=entity))

        else:
            self._instructions.append(Application._InstructionInitPointer(
                key=key,
                value=CValue(entity)))

    def __delitem__(self, key: "str"):
        """Finalize a context and remove it from the registry.
        """
        if not isinstance(key, str):
            raise TypeError("Key must be a string")

        self._instructions.append(Application._InstructionFinal(
            key=key))

    def add_contents(self, contents: "dict"):
        """Add key-values to the dictionary of contents.
        """
        if not isinstance(contents, dict) \
                or not all(isinstance(key, str) for key in contents.keys()):
            raise TypeError("Contents must be a dictionary with string keys")
        elif Application.CONTENT_INSTRUCTIONS in contents:
            raise KeyError(f"Application contents list cannot contain key '{Application.CONTENT_INSTRUCTIONS}'")

        self._contents.update(contents)

    def add_block(self, value: "CValue") -> "MemoryBlock":
        """Add a memory block to the list of blocks.
        """
        block = MemoryBlock(value)
        self._blocks[value] = block
        return block

    def reset(self):
        """Reset the internal state.
        """
        self._contents = {}
        self._instructions = []
        self._blocks = {}

    def address_of(self, entity) -> "int":
        """Obtain actual address of an entity in the memory.
        This method is to be used in CValue callbacks to obtain addresses of known objects.
        """
        if isinstance(entity, CValue):
            block = self._ptr_values.get(entity) or self._blocks.get(entity)
        elif isinstance(entity, str):
            block = self._ptr_strings.get(entity)
        elif isinstance(entity, list):
            block = self._ptr_index_arrays.get(tuple(entity))
        else:
            raise TypeError("Application only stores entities of type: CValue, str, list[int]")

        return block.address() if block is not None else None

    def memory(self) -> "Memory":
        """Create a Memory object from the current application internal state.
        """
        from .ctypes.common import archi_parameter_list_t, archi_exe_input_file_header_t
        from .ctypes.instructions import archi_exe_registry_instr_list_t

        # Initialize the list of blocks
        self._ptr_blocks = list(self._blocks.values())

        # Allocate all blocks required for the instructions
        ptr_instructions = [None] * len(self._instructions)
        for idx, instruction in enumerate(self._instructions):
            ptr_instr = instruction.alloc(self, ptr_instructions, idx)
            self._ptr_blocks.append(ptr_instr)

            node = archi_exe_registry_instr_list_t()

            def init_instructions_node(node: "archi_exe_registry_instr_list_t",
                                       idx=idx, ptr_instr=ptr_instr):
                if idx < len(ptr_instructions) - 1:
                    node.next = c.cast(ptr_instructions[idx + 1].address(), type(node.next))

                node.instruction = c.cast(ptr_instr.address(), type(node.instruction))

            ptr_instructions[idx] = MemoryBlock(CValue(node, callback=init_instructions_node))

        self._ptr_blocks += ptr_instructions

        # Allocate custom file contents
        ptr_contents = self._alloc_params(self._contents)

        # Insert the instructions to the file contents:
        if ptr_instructions:
            node = archi_parameter_list_t()

            node.value.element.num_of = 1
            node.value.element.size = c.sizeof(archi_exe_registry_instr_list_t)
            node.value.element.alignment = c.alignment(archi_exe_registry_instr_list_t)

            ptr_key = self._alloc_string(Application.CONTENT_INSTRUCTIONS)

            def init_contents_node(node: "archi_parameter_list_t",
                                   ptr_contents=ptr_contents, ptr_key=ptr_key,
                                   ptr_instructions=ptr_instructions[0]):
                if ptr_contents is not None:
                    node.next = c.cast(ptr_contents.address(), type(node.next))

                node.name = ptr_key.address()
                node.value.ptr = ptr_instructions.address()

            ptr_contents = MemoryBlock(CValue(node, callback=init_contents_node))
            self._ptr_blocks.append(ptr_contents)

        # Initialize the file header and allocate the block
        file_header = archi_exe_input_file_header_t()
        file_header.magic = archi_exe_input_file_header_t.MAGIC.encode() + b'\x00'

        memory = None
        ptr_header = None

        def init_header(file_header):
            file_header.header.addr = ptr_header.address()
            file_header.header.end  = ptr_header.address() + memory.size()

            file_header.contents = c.cast(ptr_contents.address(), type(file_header.contents))

        ptr_header = MemoryBlock(CValue(file_header, callback=init_header))

        # Initialize the memory object
        memory = Memory(ptr_header, self._ptr_blocks)

        # Reset auxiliary storage
        self._ptr_blocks = []
        self._ptr_values = {}
        self._ptr_strings = {}
        self._ptr_index_arrays = {}

        return memory

    def _alloc_value(self, value) -> "MemoryBlock":
        if value is None:
            return None
        elif not isinstance(value, CValue):
            raise TypeError("A value must be of type CValue")

        if value not in self._ptr_values:
            block = MemoryBlock(value)

            self._ptr_values[value] = block
            self._ptr_blocks.append(block)
        else:
            block = self._ptr_values[value]

        return block

    def _alloc_string(self, string: "str") -> "MemoryBlock":
        if not isinstance(string, str):
            raise TypeError("A string must be of type str")

        if string not in self._ptr_strings:
            block = MemoryBlock(CValue(string))

            self._ptr_strings[string] = block
            self._ptr_blocks.append(block)
        else:
            block = self._ptr_strings[string]

        return block

    def _alloc_index_array(self, index_array: "list[int]") -> "MemoryBlock":
        if not isinstance(index_array, list):
            raise TypeError("An index array must be of type list")

        if not index_array:
            return None

        index_array = tuple(index_array)

        if index_array not in self._ptr_index_arrays:
            block = MemoryBlock(CValue((c.c_size_t * len(index_array))(*index_array)))

            self._ptr_index_arrays[index_array] = block
            self._ptr_blocks.append(block)
        else:
            block = self._ptr_index_arrays[index_array]

        return block

    def _alloc_params(self, params: "dict") -> "MemoryBlock":
        from .ctypes.common import archi_pointer_t, archi_parameter_list_t

        if not isinstance(params, dict):
            raise TypeError("Parameter list must be of type dict")

        ptr_nodes = [None] * len(params)

        for idx, (key, value) in enumerate(params.items()):
            if not isinstance(value, CValue):
                value = CValue(value)

            node = archi_parameter_list_t()

            if value is not None:
                flags = value.attributes().get('flags', 0)
                if flags >= 1 << archi_pointer_t.NUM_FLAG_BITS:
                    raise ValueError(f"Flags must fit into {archi_pointer_t.NUM_FLAG_BITS} lowest bits")

                node.value.flags = flags
                node.value.element.num_of = value.num_elements()
                node.value.element.size = value.element_size()
                node.value.element.alignment = value.element_alignment()

            ptr_key = self._alloc_string(key)
            ptr_value = self._alloc_value(value)

            def init_list_node(node: "archi_parameter_list_t",
                               idx=idx, ptr_key=ptr_key, ptr_value=ptr_value):
                if idx < len(params) - 1:
                    node.next = c.cast(ptr_nodes[idx + 1].address(), type(node.next))

                node.name = ptr_key.address()
                if ptr_value is not None:
                    node.value.ptr = ptr_value.address()

            ptr_nodes[idx] = MemoryBlock(CValue(node, callback=init_list_node))

        self._ptr_blocks += ptr_nodes
        return ptr_nodes[0] if ptr_nodes else None

