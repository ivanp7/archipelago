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
# @brief Python module: Application initialization file generator.

import ctypes as c

###############################################################################
# Representation of memory consisting of multiple blocks with pointer support
###############################################################################

class CValue:
    """Wrapper around a ctypes value with a callback function and optional attributes.
    """
    def __init__(self, value: "c._CData", /, callback: "Callable[[c._CData]]" = None, **attributes):
        """Initialize a ctypes value wrapper.
        """
        import types

        if not isinstance(value, c._CData):
            raise TypeError("Wrapped value must be a ctypes object")
        elif callback is not None and not isinstance(callback, types.FunctionType):
            raise TypeError("Callback must be a function or None")

        self._object = value
        self._callback = callback
        self._attributes = attributes

        if isinstance(value, c.Array):
            if len(value) == 0:
                raise ValueError("Array of values must not be empty")

            self._num_of = len(value)
            self._size = c.sizeof(value[0])
            self._alignment = c.alignment(value[0])
        else:
            self._num_of = 1
            self._size = c.sizeof(value)
            self._alignment = c.alignment(value)

    def object(self) -> "c._CData":
        """Obtain the original value object.
        """
        return self._object

    def callback(self) -> "Callable[[c._CData]]":
        """Obtain the callback.

        This callback function is called on the copy when the value is copied to the memory,
        after all memory blocks have been assigned addresses.
        It is needed to write data structures containing pointers to other structures.
        """
        return self._callback

    def attributes(self) -> "dict":
        """Obtain dictionary of additional attributes.
        """
        return self._attributes

    def num_elements(self) -> "int":
        """Get the number of value elements.
        """
        return self._num_of

    def element_size(self) -> "int":
        """Get the size of a value element in bytes.
        """
        return self._size

    def element_alignment(self) -> "int":
        """Get the alignment requirement of a value element in bytes.
        """
        return self._alignment


class MemoryBlock:
    """Representation of a continuous memory block.
    """
    def __init__(self, value: "CValue", /):
        """Initialize a memory block.
        """
        if not isinstance(value, CValue):
            raise TypeError("Value object must be of type CValue")

        self._value = value

        self._owner = None
        self._address = None
        self._object = None

    def value(self) -> "CValue":
        """Obtain the original value.
        """
        return self._value

    def size(self) -> "int":
        """Obtain block size in bytes.
        """
        return c.sizeof(self._value.object())

    def alignment(self) -> "int":
        """Obtain block alignment requirement in bytes.
        """
        return c.alignment(self._value.object())

    def owner(self) -> "Memory":
        """Obtain memory object owning this block.
        """
        return self._owner

    def address(self) -> "int":
        """Obtain actual block address in the memory.
        """
        return self._address

    def object(self) -> "c._CData":
        """Obtain actual block object in the memory.
        """
        return self._object


class Memory:
    """Representation of memory consisting of relocatable memory blocks.
    """
    def __init__(self, header: "MemoryBlock" = None, blocks: "list[MemoryBlock]" = []):
        """Initialize a memory object.
        """
        self.set_header_block(header)
        self.set_blocks(blocks)

    def header_block(self) -> "MemoryBlock":
        """Obtain the header block of the memory.
        """
        return self._header

    def set_header_block(self, header: "MemoryBlock"):
        """Set the header block of the memory.
        """
        if header is self._header:
            return

        if header is not None and not isinstance(header, MemoryBlock):
            raise TypeError("Memory header object must be of type MemoryBlock")
        elif header is not None and header._owner is not None:
            raise RuntimeError("Memory block is already owned by another Memory object")
        elif header in self._blocks:
            raise RuntimeError("Header block cannot be in the list of regular blocks")

        if self._header is not None:
            self._header._owner = None

        self._header = header
        self._header._owner = self

        self._reset()

    def blocks(self) -> "list[MemoryBlock]":
        """Obtain the list of blocks of the memory.
        """
        return self._blocks.copy()

    def set_blocks(self, blocks: "list[MemoryBlock]"):
        """Set the list of blocks of the memory.
        """
        if not isinstance(blocks, list) \
                or not all(isinstance(block, MemoryBlock) for block in blocks):
            raise TypeError("List of blocks must be of type list[MemoryBlock]")
        elif not all(block._owner is None or block._owner is self for block in blocks):
            raise RuntimeError("Some of blocks in the list are owned by another Memory object")
        elif self._header in blocks:
            raise RuntimeError("Header block cannot be in the list of regular blocks")
        elif len(blocks) != len(set(blocks)):
            raise TypeError("List of blocks cannot contain duplicate elements")

        for block in self._blocks:
            block._owner = None

        self._blocks = blocks.copy()

        for block in self._blocks:
            block._owner = self

        self._reset()

    def size(self) -> "int":
        """Obtain the total size of the memory in bytes.
        """
        if self._size is None:
            self._calculate()

        return self._size

    def padding(self) -> "int":
        """Obtain the total number of padding bytes in the memory.
        """
        if self._padding is None:
            self._calculate()

        return self._padding

    def alignment(self) -> "int":
        """Obtain the alignment requirement the memory.
        """
        if self._alignment is None:
            self._calculate()

        return self._alignment

    def pack(self):
        """Reorder the blocks to decrease the memory size.
        """
        from functools import cmp_to_key

        # Sort blocks by descending alignment (for equal alignments, by descending size)
        def compare(block1, block2):
            return block2.alignment() - block1.alignment() \
                    if block2.alignment() != block1.alignment() \
                    else block2.size() - block1.size()

        self._blocks.sort(key=cmp_to_key(compare))

        # Pack sorted list of blocks
        blocks = {} # offset -> MemoryBlock

        gaps = []
        end = self._header.size() if self._header is not None else 0

        for block in self._blocks:
            alignment = block.alignment()
            size = block.size()

            # Try to fit the block into one of gaps
            gap_used = None
            new_gaps = []
            for idx, gap in enumerate(gaps):
                gap_addr = gap[0]
                gap_size = gap[1]

                gap_addr_align = (gap_addr + (alignment - 1)) & ~(alignment - 1)
                gap_size_align = gap_size - (gap_addr_align - gap_addr)

                if size <= gap_size_align: # succeed
                    if gap_addr_align > gap_addr:
                        new_gaps.append((gap_addr, gap_addr_align - gap_addr))

                    if size < gap_size_align:
                        new_gaps.append((gap_addr_align + size, gap_size_align - size))

                    gap_used = idx
                    break

            # Insert the block into the dictionary and update gaps
            if gap_used is None:
                address = (end + (alignment - 1)) & ~(alignment - 1)
                if address > end:
                    gaps.append((end, address - end))

                end = address + size
            else:
                address = gap_addr_align
                gaps[gap_used:gap_used+1] = new_gaps

            blocks[address] = block

        # Create the new list of blocks
        self._blocks = [block for _, block in sorted(blocks.items())]

        self._reset()

    def buffer(self, address: "int" = None) -> "bytearray":
        """Serialize the memory to a new byte array.
        """
        if address is not None and not isinstance(address, int):
            raise TypeError("Memory address must be an integer")

        if self.size() == 0:
            return bytearray(0)

        # Create a byte array
        buffer = bytearray(self.size())
        buffer_address = c.addressof(c.c_char.from_buffer(buffer))

        if address is None:
            address = buffer_address

        blocks = [self._header] if self._header is not None else []
        blocks += self._blocks

        # Assign addresses to the blocks and copy their contents to the buffer
        offset = 0

        for block in blocks:
            alignment = block.alignment()
            offset = (offset + (alignment - 1)) & ~(alignment - 1)

            src_object = block.value().object()
            dst_object = src_object.from_address(buffer_address + offset)

            c.memmove(dst_object, src_object, block.size())

            block._address = address + offset
            block._object = dst_object

            offset += block.size()

        # Call the block callbacks
        for block in blocks:
            callback = block.value().callback()

            if callback is not None:
                callback(block._object)

        # Reset block addresses
        for block in blocks:
            block._address = None
            block._object = None

        return buffer

    def _reset(self):
        self._size = None
        self._padding = None

    def _calculate(self):
        size = self._header.size() if self._header is not None else 0
        padding = 0
        max_alignment = self._header.alignment() if self._header is not None else 1

        for block in self._blocks:
            alignment = block.alignment()
            max_alignment = max(alignment, max_alignment)

            padded_size = (size + (alignment - 1)) & ~(alignment - 1)

            padding += padded_size - size
            size = padded_size + block.size()

        self._size = size
        self._padding = padding
        self._alignment = max_alignment

###############################################################################
# Archipelago application initialization file
###############################################################################

class Parameters:
    """Representation of a parameter list.
    """
    def __init__(self, _: "Context" = None, /, **params):
        """Create a context parameter list representation instance.
        """
        if _ is not None and not isinstance(_, Context):
            raise TypeError("Dynamic parameter list object must be of type Context or None")
        elif not all(isinstance(v, c._CData) or isinstance(v, CValue) for v in params.values()):
            raise TypeError("Values in a static parameter list must be of type ctypes._CData or CValue")

        self._dparams = _
        self._sparams = params

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
    def __init__(self, interface: "Context", params: "Parameters"):
        """Create a context specification instance.
        """
        if not isinstance(interface, Context):
            raise TypeError("Context interface object must be of type Context")
        elif not isinstance(params, Parameters):
            raise TypeError("Parameter list object must be of type Parameters")

        self._interface = interface
        self._params = params

    def interface(self) -> "Context":
        """Obtain the context interface from the specification.
        """
        return self._interface

    def parameters(self) -> "Parameters":
        """Obtain the context parameter list from the specification.
        """
        return self._params


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

        def __getattr__(self, name: "str") -> "_Slot":
            """Obtain a context slot object.
            """
            if self._indices:
                raise RuntimeError("Indexed slot name is final and cannot be extended using dot (.)")

            return _Slot(self._context, f'{self._name}.{name}')

        def __getitem__(self, index: "int") -> "_Slot":
            """Obtain a context slot object.
            """
            if not isinstance(index, int):
                raise TypeError("Slot index must be an integer")

            return _Slot(self._context, self._name, self._indices + [index])

        def __setattr__(self, name: "str", value):
            """Perform a slot setting operation.
            """
            self._context._set(f'{self._name}.{name}', self._indices)

        def __setitem__(self, index: "int", value):
            """Perform a slot setting operation.
            """
            if not isinstance(index, int):
                raise TypeError("Context slot index must be an integer")

            self._context._set(self._name, self._indices + [index])

        def __call__(self, _: "Context" = None, /, **params) -> "_Action":
            """Perform an action.
            """
            if _ is not None and not isinstance(_, Context):
                raise TypeError("Dynamic parameter list object must be of type Context")
            elif not all(isinstance(v, CValue) or isinstance(v, c._CData) for v in params.values()):
                raise TypeError("Values in a static parameter list must be of type CValue or ctypes._CData")

            self._context._act(self._name, self._indices, dparams=_, sparams=params)
            return _Action(self._context, self._name, self._indices)

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

    def __getattr__(self, name: "str") -> "_Slot":
        """Obtain a context slot object.
        """
        return _Slot(self, name)

    def __getitem__(self, index: "int") -> "_Slot":
        """Obtain a context slot object.
        """
        if not isinstance(index, int):
            raise TypeError("Slot index must be an integer")

        return _Slot(self, indices=[index])

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

    def __call__(self, _: "Context" = None, /, **params) -> "ContextSpec":
        """Create a context specification instance.
        """
        return ContextSpec(self, Parameters(_, **params))

    def _set(self, slot_name: "str", slot_indices: "list[int]", value):
        """Append a set() instruction to the list.
        """
        if not isinstance(slot_name, str):
            raise TypeError("Slot name must be a string")
        elif not isinstance(slot_indices, list) \
                or not all(isinstance(index, int) for index in slot_indices):
            raise TypeError("Slot indices must be a list of integers")

        if isinstance(value, c._CData):
            self._app._instructions.append(Application._InstructionSetValue(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                value=CValue(value)))

        elif isinstance(value, CValue):
            self._app._instructions.append(Application._InstructionSetValue(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                value=value))

        elif isinstance(value, Context):
            self._app._instructions.append(Application._InstructionSetContext(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                source_key=value._key))

        elif isinstance(value, _Slot) or isinstance(value, _Action):
            self._app._instructions.append(Application._InstructionSetSlot(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                source_key=value._context._key,
                source_slot_name=value._name, source_slot_indices=value._indices))

        else:
            raise TypeError("Context slot can only be set to an object of type: ctypes._CData, CValue, Context, Context._Slot, or Context._Action")

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
        elif not all(isinstance(v, CValue) or isinstance(v, c._CData) for v in sparams.values()):
            raise TypeError("Values in a static parameter list must be of type CValue or ctypes._CData")

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
        NOOP, INIT, FINAL, SET_VALUE, SET_CONTEXT, SET_SLOT, ACT = range(7)

        def __init__(self, key: "str"):
            """Initialize an instruction base.
            """
            if not isinstance(key, str):
                raise TypeError("Context key must be a string")

            self._key = key

    class _InstructionNoop(_Instruction):
        """Representation of an application initialization instruction: no-op.
        """
        def __init__(self, key: "str"):
            """Initialize an instruction.
            """
            self._type = NOOP
            _Instruction.__init__(key)

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            instr = archi_exe_registry_instr_base_t()
            instr.type = self._type

            return MemoryBlock(CValue(instr))

    class _InstructionInit(_Instruction):
        """Representation of an application initialization instruction:
        initialize a new context.
        """
        def __init__(self, key: "str", interface_key: "str", dparams_key: "str", sparams: "dict"):
            """Initialize an instruction.
            """
            if not isinstance(interface_key, str):
                raise TypeError("Context interface key must be a string")
            elif not isinstance(dparams_key, str):
                raise TypeError("Dynamic parameter list key must be a string")
            elif not isinstance(sparams, dict):
                raise TypeError("Static parameter list must be of type dict")

            self._type = INIT
            _Instruction.__init__(key)
            self._interface_key = interface_key
            self._dparams_key = dparams_key
            self._sparams = sparams

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            instr = archi_exe_registry_instr_init_t()
            instr.base.type = self._type

            ptr_key = app._alloc_string(self._key)

            ptr_interface_key = app._alloc_string(self._interface_key)
            ptr_dparams_key = app._alloc_string(self._dparams_key)
            ptr_sparams = app._alloc_params(self._sparams)

            def init_instr(instr: "archi_exe_registry_instr_init_t"):
                instr.base.key = ptr_key.address()

                instr.interface_key = ptr_interface_key.address()
                instr.dparams_key = ptr_dparams_key.address()
                instr.sparams = ptr_sparams.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionFinal(_Instruction):
        """Representation of an application initialization instruction: finalize a context.
        """
        def __init__(self, key: "str"):
            """Initialize an instruction.
            """
            self._type = FINAL
            _Instruction.__init__(key)

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            instr = archi_exe_registry_instr_base_t()
            instr.type = self._type

            ptr_key = app._alloc_string(self._key)

            def init_instr(instr: "archi_exe_registry_instr_base_t"):
                instr.base.key = ptr_key.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionSetValue(_Instruction):
        """Representation of an application initialization instruction:
        set a value to a context slot.
        """
        def __init__(self, key: "str", slot_name: "str", slot_indices: "list[int]",
                     value: "CValue"):
            """Initialize an instruction.
            """
            if not isinstance(slot_name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(slot_indices, list) \
                    or not all(isinstance(index, int) for index in slot_indices):
                raise TypeError("Slot indices must be a list of integers")
            elif not isinstance(value, CValue):
                raise TypeError("Object assigned to a slot must be of type CValue")

            self._type = SET_VALUE
            _Instruction.__init__(key)
            self._slot_name = slot_name
            self._slot_indices = slot_indices
            self._value = value

            self._flags = value.attributes().get('flags', 0)
            if self._flags >= 1 << archi_pointer_t.NUM_FLAG_BITS:
                raise ValueError(f"Flags must fit into {archi_pointer_t.NUM_FLAG_BITS} lowest bits")

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            instr = archi_exe_registry_instr_set_value_t()
            instr.base.type = self._type
            instr.slot.num_indices = len(self._slot_indices)
            instr.value.flags = self._flags
            instr.value.element.num_of = self._value.num_elements()
            instr.value.element.size = self._value.element_size()
            instr.value.element.alignment = self._value.element_alignment()

            ptr_key = app._alloc_string(self._key)

            ptr_slot_name = app._alloc_string(self._slot_name)
            ptr_slot_indices = app._alloc_index_array(self._slot_indices)
            ptr_value = app._alloc_value(self._value)

            def init_instr(instr: "archi_exe_registry_instr_set_value_t"):
                instr.base.key = ptr_key.address()

                instr.slot.name = ptr_slot_name.address()
                instr.slot.index = ptr_slot_indices.address()
                instr.value.ptr = ptr_value.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionSetContext(_Instruction):
        """Representation of an application initialization instruction:
        set a source context pointer to a context slot.
        """
        def __init__(self, key: "str", slot_name: "str", slot_indices: "list[int]",
                     source_key: "str"):
            """Initialize an instruction.
            """
            if not isinstance(slot_name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(slot_indices, list) \
                    or not all(isinstance(index, int) for index in slot_indices):
                raise TypeError("Slot indices must be a list of integers")
            elif not isinstance(source_key, str):
                raise TypeError("Source context key must be a string")

            self._type = SET_CONTEXT
            _Instruction.__init__(key)
            self._slot_name = slot_name
            self._slot_indices = slot_indices
            self._source_key = source_key

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            instr = archi_exe_registry_instr_set_context_t()
            instr.base.type = self._type
            instr.slot.num_indices = len(self._slot_indices)

            ptr_key = app._alloc_string(self._key)

            ptr_slot_name = app._alloc_string(self._slot_name)
            ptr_slot_indices = app._alloc_index_array(self._slot_indices)
            ptr_source_key = app._alloc_string(self._source_key)

            def init_instr(instr: "archi_exe_registry_instr_set_context_t"):
                instr.base.key = ptr_key.address()

                instr.slot.name = ptr_slot_name.address()
                instr.slot.index = ptr_slot_indices.address()
                instr.source_key = ptr_source_key.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionSetSlot(_Instruction):
        """Representation of an application initialization instruction:
        set a source context slot to a context slot.
        """
        def __init__(self, key: "str", slot_name: "str", slot_indices: "list[int]",
                     source_key: "str", source_slot_name: "str", source_slot_indices: "list[int]"):
            """Initialize an instruction.
            """
            if not isinstance(slot_name, str):
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

            self._type = SET_SLOT
            _Instruction.__init__(key)
            self._slot_name = slot_name
            self._slot_indices = slot_indices
            self._source_key = source_key
            self._source_slot_name = source_slot_name
            self._source_slot_indices = source_slot_indices

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            instr = archi_exe_registry_instr_set_slot_t()
            instr.base.type = self._type
            instr.slot.num_indices = len(self._slot_indices)
            instr.source_slot.num_indices = len(self._source_slot_indices)

            ptr_key = app._alloc_string(self._key)

            ptr_slot_name = app._alloc_string(self._slot_name)
            ptr_slot_indices = app._alloc_index_array(self._slot_indices)
            ptr_source_key = app._alloc_string(self._source_key)
            ptr_source_slot_name = app._alloc_string(self._source_slot_name)
            ptr_source_slot_indices = app._alloc_index_array(self._source_slot_indices)

            def init_instr(instr: "archi_exe_registry_instr_set_slot_t"):
                instr.base.key = ptr_key.address()

                instr.slot.name = ptr_slot_name.address()
                instr.slot.index = ptr_slot_indices.address()
                instr.source_key = ptr_source_key.address()
                instr.source_slot.name = ptr_source_slot_name.address()
                instr.source_slot.index = ptr_source_slot_indices.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionAct(_Instruction):
        """Representation of an application initialization instruction:
        invoke a context action.
        """
        def __init__(self, key: "str",
                     action_name: "str", action_indices: "list[int]", dparams_key: "str", sparams: "dict"):
            """Initialize an instruction.
            """
            if not isinstance(action_name, str):
                raise TypeError("Action name must be a string")
            elif not isinstance(action_indices, list) \
                    or not all(isinstance(index, int) for index in action_indices):
                raise TypeError("Action indices must be a list of integers")
            elif not isinstance(dparams_key, str):
                raise TypeError("Dynamic parameter list key must be a string")
            elif not isinstance(sparams, dict):
                raise TypeError("Static parameter list must be of type dict")

            self._type = ACT
            _Instruction.__init__(key)
            self._action_name = action_name
            self._action_indices = action_indices
            self._dparams_key = dparams_key
            self._sparams = sparams

        def alloc(self, app: "Application", ptr_instructions: "list[MemoryBlock]", idx: "int"):
            """Allocate all required blocks.
            """
            instr = archi_exe_registry_instr_act_t()
            instr.base.type = self._type
            instr.action.num_indices = len(self._action_indices)

            ptr_key = app._alloc_string(self._key)

            ptr_action_name = app._alloc_string(self._action_name)
            ptr_action_indices = app._alloc_index_array(self._action_indices)
            ptr_dparams_key = app._alloc_string(self._dparams_key)
            ptr_sparams = app._alloc_params(self._sparams)

            def init_instr(instr: "archi_exe_registry_instr_act_t"):
                instr.base.key = ptr_key.address()

                instr.action.name = ptr_action_name.address()
                instr.action.index = ptr_action_indices.address()
                instr.dparams_key = ptr_dparams_key.address()
                instr.sparams = ptr_sparams.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    CONTENTS_KEY_INSTRUCTIONS = 'archi.instructions'
    CONTENTS_KEY_SIGNALS      = 'archi.signals'

    KEY_REGISTRY       = 'archi.registry'
    KEY_INTERFACES     = 'archi.interfaces'
    KEY_EXE_HANDLE     = 'archi.executable'
    KEY_INPUT_FILE     = 'archi.input.file'
    KEY_INPUT_CONTENTS = 'archi.input.contents'
    KEY_SIGNAL         = 'archi.signal'

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

        if isinstance(entity, ParameterList) or isinstance(entity, ContextSpec):
            interface_key = entity.interface()._key \
                    if isinstance(entity, ContextSpec) \
                    else None

            params = entity.parameters() \
                    if isinstance(entity, ContextSpec) \
                    else entity

            self._instructions.append(_InstructionInit(
                key=key,
                interface_key=interface_key,
                dparams_key=params.dynamic_list()._key, sparams=params.static_list()))

        elif isinstance(entity, c._CData) or isinstance(entity, CValue):
            self._instructions.append(_InstructionInit(
                key=key,
                interface_key='',
                dparams_key=None, sparams={'value': CValue(entity) \
                        if isinstance(entity, c._CData) else entity}))

        elif isinstance(entity, Context):
            self._instructions.append(_InstructionInit(
                key=key,
                interface_key='',
                dparams_key=None, sparams={}))

            self._instructions.append(_InstructionSetContext(
                key=key,
                slot_name='value', slot_indices=[],
                source_key=entity._key))

        elif isinstance(entity, Context._Slot) or isinstance(entity, Context._Action):
            self._instructions.append(_InstructionInit(
                key=key,
                interface_key='',
                dparams_key=None, sparams={}))

            self._instructions.append(_InstructionSetSlot(
                key=key,
                slot_name='value', slot_indices=[],
                source_key=entity._context._key,
                source_slot_name=entity._name, source_slot_indices=entity._indices))

        else:
            raise TypeError("Context can only be initialized from: ParameterList, ContextSpec, ctypes._CData, CValue, Context._Slot, or Context._Action")

    def __delitem__(self, key: "str"):
        """Finalize a context and remove it from the registry.
        """
        if not isinstance(key, str):
            raise TypeError("Key must be a string")

        self._instructions.append(_InstructionFinal(
            key=key))

    def contents(self) -> "dict":
        """Obtain the application contents dictionary.
        """
        return self._contents

    def set_contents(self, **contents):
        """Set the application parameter list.
        """
        if not all(isinstance(value, c._CData) or isinstance(value, CValue) \
                for value in contents.values()):
            raise TypeError("Content values must be objects of type ctypes._CData or CValue")
        elif CONTENTS_KEY_INSTRUCTIONS in contents:
            raise KeyError(f"Application contents list cannot contain key '{CONTENTS_KEY_INSTRUCTIONS}'")

        self._contents = contents

    def add_block(self, value: "CValue") -> "MemoryBlock":
        """Add a memory block to the list of blocks.
        """
        block = MemoryBlock(value)
        self._blocks[value] = block
        return block

    def reset(self):
        """Reset the list of instructions.
        """
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
        """Create a Memory object from the application file contents.
        """
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
                if idx < len(ptr_instructions):
                    node.next = ptr_instructions[idx + 1].address()

                node.instruction = ptr_instr.address()

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

            ptr_key = self._alloc_string(CONTENTS_KEY_INSTRUCTIONS)

            def init_contents_node(node: "archi_parameter_list_t",
                                   ptr_contents=ptr_contents, ptr_key=ptr_key,
                                   ptr_instructions=ptr_instructions[0]):
                if ptr_contents is not None:
                    node.next = ptr_contents.address()

                node.name = ptr_key.address()
                node.value.ptr = ptr_instructions.address()

            ptr_contents = MemoryBlock(CValue(node, callback=init_contents_node))
            self._ptr_blocks.append(ptr_contents)

        # Initialize the file header and allocate the block
        file_header = archi_exe_input_file_header_t()
        file_header.magic[:len(archi_exe_input_file_header_t.MAGIC)] = \
                archi_exe_input_file_header_t.MAGIC.encode()
        file_header.magic[len(archi_exe_input_file_header_t.MAGIC)] = 0

        memory = None
        ptr_header = None

        def init_header(file_header):
            file_header.header.addr = ptr_header.address()
            file_header.header.end  = ptr_header.address() + memory.size()

            file_header.contents = ptr_contents.address()

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
        if not isinstance(value, CValue):
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
            block = MemoryBlock(CValue(c.create_string_buffer(string.encode())))

            self._ptr_strings[string] = block
            self._ptr_blocks.append(block)
        else:
            block = self._ptr_strings[string]

        return block

    def _alloc_index_array(self, index_array: "list[int]") -> "MemoryBlock":
        if not isinstance(index_array, list):
            raise TypeError("An index array must be of type list")

        index_array = tuple(index_array)

        if index_array not in self._ptr_index_arrays:
            block = MemoryBlock(CValue((c.c_size_t * len(index_array))(*index_array)))

            self._ptr_index_arrays[index_array] = block
            self._ptr_blocks.append(block)
        else:
            block = self._ptr_index_arrays[index_array]

        return block

    def _alloc_params(self, params: "dict") -> "MemoryBlock":
        if not isinstance(params, dict):
            raise TypeError("Parameter list must be of type dict")

        ptr_nodes = [None] * len(params)

        for idx, (key, value) in enumerate(params.items()):
            if isinstance(value, c._CData):
                value = CValue(value)

            flags = value.attributes().get('flags', 0)
            if flags >= 1 << archi_pointer_t.NUM_FLAG_BITS:
                raise ValueError(f"Flags must fit into {archi_pointer_t.NUM_FLAG_BITS} lowest bits")

            node = archi_parameter_list_t()
            node.value.flags = flags
            node.value.element.num_of = value.num_elements()
            node.value.element.size = value.element_size()
            node.value.element.alignment = value.element_alignment()

            ptr_key = self._alloc_string(key)
            ptr_value = self._alloc_value(value)

            def init_list_node(node: "archi_parameter_list_t",
                               idx=idx, ptr_key=ptr_key, ptr_value=ptr_value):
                if idx < len(params) - 1:
                    node.next = ptr_nodes[idx + 1].address()

                node.name = ptr_key.address()
                node.value.ptr = ptr_value.address()

            ptr_nodes[idx] = MemoryBlock(CValue(node, callback=init_list_node))

        self._ptr_blocks += ptr_nodes
        return ptr_nodes[0] if ptr_nodes else None

###############################################################################
# C types used in a Archipelago application initialization file
###############################################################################

class archi_array_layout_t(c.Structure):
    """Array layout description.
    """
    _fields_ = [('num_of', c.c_size_t),
                ('size', c.c_size_t),
                ('alignment', c.c_size_t)]


class archi_pointer_t(c.Structure):
    """Generic wrapper for data or function pointers with metadata.
    """
    class union(c.Union):
        """Union of a generic pointer to data and a generic pointer to function.
        """
        _fields_ = [('ptr', c.c_void_p),
                    ('fptr', c.CFUNCTYPE(None))]

    NUM_FLAG_BITS = 64 - 2

    _anonymous_ = ['as']
    _fields_ = [('as', union),
                ('ref_count', c.c_void_p),
                ('flags', c.c_uint64),
                ('element', archi_array_layout_t)]


class archi_parameter_list_t(c.Structure):
    """List of named values.
    """
    pass

archi_parameter_list_t._fields_ = \
        [('next', c.POINTER(archi_parameter_list_t)),
         ('name', c.c_char_p),
         ('value', archi_pointer_t)]


class archi_context_op_designator_t(c.Structure):
    """Context operation designator.
    """
    _fields_ = [('name', c.c_char_p),
                ('index', c.POINTER(c.c_size_t)),
                ('num_indices', c.c_size_t)]

###############################################################################

class archi_file_header_t(c.Structure):
    """Header of a memory-mapped file.
    """
    _fields_ = [('addr', c.c_void_p),
                ('end', c.c_void_p)]


class archi_exe_registry_instr_base_t(c.Structure):
    """Context registry instruction base.
    """
    _fields_ = [('type', c.c_int),
                ('key', c.c_char_p)]


class archi_exe_registry_instr_list_t(c.Structure):
    """Context registry instruction list.
    """
    pass

archi_exe_registry_instr_list_t._fields_ = \
        [('next', c.POINTER(archi_exe_registry_instr_list_t)),
         ('instruction', c.POINTER(archi_exe_registry_instr_base_t))]


class archi_exe_input_file_header_t(c.Structure):
    """Description of an input file for the executable.
    """
    MAGIC = "[archi]"

    _fields_ = [('header', archi_file_header_t),
                ('magic', c.c_char * 8),
                ('contents', c.POINTER(archi_parameter_list_t))]

###############################################################################

class archi_exe_registry_instr_init_t(c.Structure):
    """Context registry instruction: initialize a new context.
    """
    _anonymous_ = ['params']
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('interface_key', c.c_char_p),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]


class archi_exe_registry_instr_set_value_t(c.Structure):
    """Context registry instruction: set context slot to pointer to a value.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('slot', archi_context_op_designator_t),
                ('value', archi_pointer_t)]


class archi_exe_registry_instr_set_context_t(c.Structure):
    """Context registry instruction: set context slot to pointer to a source context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('slot', archi_context_op_designator_t),
                ('source_key', c.c_char_p)]


class archi_exe_registry_instr_set_slot_t(c.Structure):
    """Context registry instruction: set context slot to a source context slot.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('slot', archi_context_op_designator_t),
                ('source_key', c.c_char_p),
                ('source_slot', archi_context_op_designator_t)]


class archi_exe_registry_instr_act_t(c.Structure):
    """Context registry instruction: invoke context action.
    """
    _anonymous_ = ['params']
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('action', archi_context_op_designator_t),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]

###############################################################################

class archi_signal_watch_set_t(c.Structure):
    """Mask of POSIX signals to watch and handle.
    """
    import signal

    SIGNALS = ['SIGINT', 'SIGQUIT', 'SIGTERM',              # interruption events
               'SIGCHLD', 'SIGCONT', 'SIGTSTP',             # process events
               'SIGXCPU', 'SIGXFSZ',                        # limit exceeding events
               'SIGPIPE', 'SIGPOLL', 'SIGURG',              # input/output events
               'SIGALRM', 'SIGVTALRM', 'SIGPROF',           # timer events
               'SIGHUP', 'SIGTTIN', 'SIGTTOU', 'SIGWINCH',  # terminal events
               'SIGUSR1', 'SIGUSR2']                        # user-defined events
    NUM_RT_SIGNALS = signal.SIGRTMAX - signal.SIGRTMIN + 1

    _fields_ = [(f'f_{signal}', c.c_bool) for signal in SIGNALS] \
            + [('f_SIGRTMIN', c.c_bool * NUM_RT_SIGNALS)]

