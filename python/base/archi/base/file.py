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
# @brief Representation of Archipelago input file.

import ctypes as c

from .memory import CValue, MemoryBlock, MemoryBlockSet, Memory


class File:
    """Representation of an Archipelago input file.
    """
    # keys recognized by the Archipelago executable
    KEY_REGISTRY = 'archi.instructions'
    KEY_SIGNALS  = 'archi.signals'

    def __init__(self):
        """Initialize a file.
        """
        self.reset()

    def __getitem__(self, key: 'str'):
        """Get element of the file contents dictionary.
        """
        if not isinstance(key, str):
            raise TypeError

        return self._contents.get(key)

    def __setitem__(self, key: 'str', value):
        """Set element of the file contents dictionary.
        """
        if not isinstance(key, str):
            raise TypeError

        self._contents[key] = value

    def __delitem__(self, key: 'str'):
        """Unset element of the file contents dictionary.
        """
        if not isinstance(key, str):
            raise TypeError

        del self._contents[key]

    def contents(self) -> 'dict[str]':
        """Get the dictionary of file contents.
        """
        from types import MappingProxyType

        return MappingProxyType(self._contents)

    def blocks(self) -> 'MemoryBlockSet':
        """Get the additional memory block set of the file.
        """
        return self._blocks

    def set_blocks(self, blocks: 'MemoryBlockSet', /):
        """Set the additional memory block set of the file.
        """
        if not isinstance(blocks, MemoryBlockSet):
            raise TypeError

        self._blocks = blocks

    def reset(self, contents: 'bool' = True, blocks: 'bool' = True):
        """Reset the internal state.
        """
        if contents:
            self._contents = {}
        if blocks:
            self._blocks = MemoryBlockSet()

    def memory(self) -> 'Memory':
        """Create a memory representation object of the file.
        """
        import copy
        from .ctypes.file import archi_exe_input_file_header_t

        blocks = copy.copy(self._blocks)
        block_contents = _FileMarshaller(blocks).marshal(self)

        block_header = None
        memory = None

        def callback_header(header):
            header.header.addr = block_header.memory_address()
            header.header.end  = block_header.memory_address() + memory.size()

            if block_contents:
                header.contents = c.cast(block_contents.memory_address(), type(header.contents))

        block_header = MemoryBlock(CValue(archi_exe_input_file_header_t(), callback=callback_header))
        memory = Memory(block_header, blocks.blocks())

        return memory

###############################################################################

class _Marshaller:
    """Base class for marshallers of objects of various types.
    """
    def __init__(self, blocks: 'MemoryBlockSet', value_dict: 'dict[Any, CValue]' = {}):
        """Initialize the marshaller object.
        """
        if not isinstance(blocks, MemoryBlockSet):
            raise TypeError
        elif not isinstance(value_dict, dict) \
                or not all(isinstance(v, CValue) for v in value_dict.values()):
            raise TypeError

        self._blocks = blocks
        self._value_dict = value_dict

    def marshal(self, obj) -> 'MemoryBlock':
        """Marshal a object of the corresponding type.
        """
        raise NotImplementedError

    def _marshal_value(self, value: 'CValue') -> 'MemoryBlock':
        if value is None:
            return None
        elif not isinstance(value, CValue):
            raise TypeError

        return self._blocks.add(value)

    def _marshal_string(self, string: 'str') -> 'MemoryBlock':
        if string is None:
            return None
        elif not isinstance(string, str):
            raise TypeError

        if string in self._value_dict:
            value = self._value_dict[string]
        else:
            value = CValue(string)
            self._value_dict[string] = value

        return self._marshal_value(value)

    def _marshal_parameter_list(self, params: 'dict[str, CValue]') -> 'MemoryBlock':
        from .ctypes.base import archi_parameter_list_t

        if params is None:
            return None
        elif not isinstance(params, dict) \
                or not all(isinstance(key, str) for key in params.keys()):
            raise TypeError

        block_nodes = [None] * len(params)

        for idx, (key, value) in enumerate(params.items()):
            node = archi_parameter_list_t()

            block_key = self._marshal_string(key)

            if value is not None:
                if isinstance(value, MemoryBlock):
                    block_value = value
                    value = block_value.cvalue()
                else:
                    block_value = self._marshal_value(value)

                node.value = _Marshaller._init_pointer(value)
            else:
                block_value = None

            def callback_node(node: 'archi_parameter_list_t',
                              idx=idx, block_key=block_key, block_value=block_value):
                if idx < len(params) - 1:
                    node.next = c.cast(block_nodes[idx + 1].memory_address(), type(node.next))

                node.name = block_key.memory_address()
                if block_value is not None:
                    node.value.ptr = block_value.memory_address()

            block_node = MemoryBlock(CValue(node, callback=callback_node))
            block_nodes[idx] = block_node

            self._blocks.add(block_node)

        return block_nodes[0] if block_nodes else None

    @staticmethod
    def _init_pointer(value: 'CValue') -> 'archi_pointer_t':
        from .ctypes.base import archi_pointer_t

        pointer = archi_pointer_t()

        flags = value.attributes().get('flags', 0)
        if not isinstance(flags, int):
            raise TypeError
        elif (flags < 0) or (flags >= 1 << archi_pointer_t.NUM_FLAG_BITS):
            raise ValueError(f"Pointer flags must fit into {archi_pointer_t.NUM_FLAG_BITS} lowest bits")

        pointer.flags = flags
        pointer.element.num_of = value.num_elements()
        pointer.element.size = value.element_size()
        pointer.element.alignment = value.element_alignment()

        return pointer


class _FileMarshaller(_Marshaller):
    """File marshaller implementation.
    """
    def marshal(self, obj: 'File') -> 'MemoryBlock':
        """Marshal an object of type File.
        """
        from .app import Registry

        if not isinstance(obj, File):
            raise TypeError

        contents = obj._contents.copy()

        for key, value in contents.items():
            if isinstance(value, Registry):
                contents[key] = _RegistryMarshaller(self._blocks, self._value_dict).marshal(value)

        return self._marshal_parameter_list(contents)


class _RegistryMarshaller(_Marshaller):
    """Registry marshaller implementation.
    """
    def marshal(self, obj: 'Registry') -> 'MemoryBlock':
        """Marshal an object of type Registry.
        """
        from .app import Registry
        from .ctypes.instruction import archi_exe_registry_instr_list_t

        if not isinstance(obj, Registry):
            raise TypeError

        instructions = obj._instructions.copy()

        block_instructions = [None] * len(instructions)

        for idx, instruction in enumerate(instructions):
            block_instruction = self._marshal_instruction(instruction)

            def callback_node(node: 'archi_exe_registry_instr_list_t',
                              idx=idx, block_instruction=block_instruction):
                if idx < len(instructions) - 1:
                    node.next = c.cast(block_instructions[idx + 1].memory_address(), type(node.next))

                node.instruction = c.cast(block_instruction.memory_address(), type(node.instruction))

            block_node = MemoryBlock(CValue(archi_exe_registry_instr_list_t(), callback=callback_node))
            block_instructions[idx] = block_node

            self._blocks.add(block_node)

        return block_instructions[0] if block_instructions else None

    def _marshal_instruction(self, instruction: 'Registry._Instruction') -> 'MemoryBlock':
        from .app import Registry
        from .ctypes.instruction import (
                archi_exe_registry_instr_base_t,
                archi_exe_registry_instr_init_from_context_t,
                archi_exe_registry_instr_init_from_slot_t,
                archi_exe_registry_instr_init_pointer_t,
                archi_exe_registry_instr_init_array_t,
                archi_exe_registry_instr_copy_t,
                archi_exe_registry_instr_delete_t,
                archi_exe_registry_instr_set_to_value_t,
                archi_exe_registry_instr_set_to_context_data_t,
                archi_exe_registry_instr_set_to_context_slot_t,
                archi_exe_registry_instr_act_t,
                )

        InstructionType = Registry._Instruction.Type

        if instruction.type() == InstructionType.NOOP.value:
            instr = archi_exe_registry_instr_base_t()
            instr.type = instruction.type()

            callback_instr = None

        elif instruction.type() == InstructionType.INIT_FROM_CONTEXT.value:
            instr = archi_exe_registry_instr_init_from_context_t()
            instr.base.type = instruction.type()

            block_key = self._marshal_string(instruction['key'])
            block_interface_origin_key = self._marshal_string(instruction['interface_origin_key'])
            block_dparams_key = self._marshal_string(instruction['dparams_key'])
            block_sparams = self._marshal_parameter_list(instruction['sparams'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()
                if block_interface_origin_key is not None:
                    instr.interface_origin_key = block_interface_origin_key.memory_address()
                if block_dparams_key is not None:
                    instr.dparams_key = block_dparams_key.memory_address()
                if block_sparams is not None:
                    instr.sparams = c.cast(block_sparams.memory_address(), type(instr.sparams))

        elif instruction.type() == InstructionType.INIT_FROM_SLOT.value:
            instr = archi_exe_registry_instr_init_from_slot_t()
            instr.base.type = instruction.type()
            instr.interface_origin_slot.num_indices = len(instruction['interface_origin_slot_indices'])

            block_key = self._marshal_string(instruction['key'])
            block_interface_origin_key = self._marshal_string(instruction['interface_origin_key'])
            block_interface_origin_slot_name = self._marshal_string(instruction['interface_origin_slot_name'])
            block_interface_origin_slot_indices = self._marshal_index_array(instruction['interface_origin_slot_indices'])
            block_dparams_key = self._marshal_string(instruction['dparams_key'])
            block_sparams = self._marshal_parameter_list(instruction['sparams'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()
                instr.interface_origin_key = block_interface_origin_key.memory_address()
                instr.interface_origin_slot.name = block_interface_origin_slot_name.memory_address()
                if block_interface_origin_slot_indices is not None:
                    instr.interface_origin_slot.index = c.cast(block_interface_origin_slot_indices.memory_address(),
                                                               type(instr.interface_origin_slot.index))
                if block_dparams_key is not None:
                    instr.dparams_key = block_dparams_key.memory_address()
                if block_sparams is not None:
                    instr.sparams = c.cast(block_sparams.memory_address(), type(instr.sparams))

        elif instruction.type() == InstructionType.INIT_POINTER.value:
            instr = archi_exe_registry_instr_init_pointer_t()
            instr.base.type = instruction.type()
            if instruction['value'] is not None:
                instr.value = _Marshaller._init_pointer(instruction['value'])

            block_key = self._marshal_string(instruction['key'])
            block_value = self._marshal_value(instruction['value'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()
                if block_value is not None:
                    instr.value.ptr = block_value.memory_address()

        elif instruction.type() == InstructionType.INIT_ARRAY.value:
            instr = archi_exe_registry_instr_init_array_t()
            instr.base.type = instruction.type()
            instr.num_elements = instruction['num_elements']
            if instruction['flags'] is not None:
                instr.flags = instruction['flags']

            block_key = self._marshal_string(instruction['key'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()

        elif instruction.type() == InstructionType.COPY.value:
            instr = archi_exe_registry_instr_copy_t()
            instr.base.type = instruction.type()

            block_key = self._marshal_string(instruction['key'])
            block_original_key = self._marshal_string(instruction['original_key'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()
                instr.original_key = block_original_key.memory_address()

        elif instruction.type() == InstructionType.DELETE.value:
            instr = archi_exe_registry_instr_delete_t()
            instr.base.type = instruction.type()

            block_key = self._marshal_string(instruction['key'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()

        elif instruction.type() == InstructionType.SET_TO_VALUE.value:
            instr = archi_exe_registry_instr_set_to_value_t()
            instr.base.type = instruction.type()
            instr.slot.num_indices = len(instruction['slot_indices'])
            if instruction['value'] is not None:
                instr.value = _Marshaller._init_pointer(instruction['value'])

            block_key = self._marshal_string(instruction['key'])
            block_slot_name = self._marshal_string(instruction['slot_name'])
            block_slot_indices = self._marshal_index_array(instruction['slot_indices'])
            block_value = self._marshal_value(instruction['value'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()
                instr.slot.name = block_slot_name.memory_address()
                if block_slot_indices is not None:
                    instr.slot.index = c.cast(block_slot_indices.memory_address(), type(instr.slot.index))
                if block_value is not None:
                    instr.value.ptr = block_value.memory_address()

        elif instruction.type() == InstructionType.SET_TO_CONTEXT_DATA.value:
            instr = archi_exe_registry_instr_set_to_context_data_t()
            instr.base.type = instruction.type()
            instr.slot.num_indices = len(instruction['slot_indices'])

            block_key = self._marshal_string(instruction['key'])
            block_slot_name = self._marshal_string(instruction['slot_name'])
            block_slot_indices = self._marshal_index_array(instruction['slot_indices'])
            block_source_key = self._marshal_string(instruction['source_key'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()
                instr.slot.name = block_slot_name.memory_address()
                if block_slot_indices is not None:
                    instr.slot.index = c.cast(block_slot_indices.memory_address(), type(instr.slot.index))
                instr.source_key = block_source_key.memory_address()

        elif instruction.type() == InstructionType.SET_TO_CONTEXT_SLOT.value:
            instr = archi_exe_registry_instr_set_to_context_slot_t()
            instr.base.type = instruction.type()
            instr.slot.num_indices = len(instruction['slot_indices'])
            instr.source_slot.num_indices = len(instruction['source_slot_indices'])

            block_key = self._marshal_string(instruction['key'])
            block_slot_name = self._marshal_string(instruction['slot_name'])
            block_slot_indices = self._marshal_index_array(instruction['slot_indices'])
            block_source_key = self._marshal_string(instruction['source_key'])
            block_source_slot_name = self._marshal_string(instruction['source_slot_name'])
            block_source_slot_indices = self._marshal_index_array(instruction['source_slot_indices'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()
                instr.slot.name = block_slot_name.memory_address()
                if block_slot_indices is not None:
                    instr.slot.index = c.cast(block_slot_indices.memory_address(), type(instr.slot.index))
                instr.source_key = block_source_key.memory_address()
                instr.source_slot.name = block_source_slot_name.memory_address()
                if block_source_slot_indices is not None:
                    instr.source_slot.index = c.cast(block_source_slot_indices.memory_address(), type(instr.source_slot.index))

        elif instruction.type() == InstructionType.ACT.value:
            instr = archi_exe_registry_instr_act_t()
            instr.base.type = instruction.type()
            instr.action.num_indices = len(instruction['action_indices'])

            block_key = self._marshal_string(instruction['key'])
            block_action_name = self._marshal_string(instruction['action_name'])
            block_action_indices = self._marshal_index_array(instruction['action_indices'])
            block_dparams_key = self._marshal_string(instruction['dparams_key'])
            block_sparams = self._marshal_parameter_list(instruction['sparams'])

            def callback_instr(instr):
                instr.key = block_key.memory_address()
                instr.action.name = block_action_name.memory_address()
                if block_action_indices is not None:
                    instr.action.index = c.cast(block_action_indices.memory_address(), type(instr.action.index))
                if block_dparams_key is not None:
                    instr.dparams_key = block_dparams_key.memory_address()
                if block_sparams is not None:
                    instr.sparams = c.cast(block_sparams.memory_address(), type(instr.sparams))

        else:
            raise ValueError(f"Unknown instruction type {instruction.type()}")

        block_instr = MemoryBlock(CValue(instr, callback=callback_instr))
        self._blocks.add(block_instr)

        return block_instr

    def _marshal_index_array(self, index_array: 'list[int]') -> 'MemoryBlock':
        if not index_array:
            return None
        elif not isinstance(index_array, list) \
                or not all(isinstance(idx, int) for idx in index_array):
            raise TypeError

        index_tuple = tuple(index_array)

        if index_tuple in self._value_dict:
            value = self._value_dict[index_tuple]
        else:
            value = CValue((c.c_size_t * len(index_tuple))(*index_tuple))
            self._value_dict[index_tuple] = value

        return self._marshal_value(value)

