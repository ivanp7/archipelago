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
    def __init__(self, value, /, callback: "Callable" = None, **attributes):
        """Initialize a ctypes value wrapper.
        """
        import types

        if callback is not None and not isinstance(callback, types.FunctionType):
            raise TypeError("Callback must be a function or None")

        if isinstance(value, str):
            value = c.create_string_buffer(value.encode())
        elif isinstance(value, bytes):
            value = c.create_string_buffer(value)

        self._object = value
        self._callback = callback
        self._attributes = attributes

        if isinstance(value, c.Array):
            if len(value) == 0:
                raise ValueError("Array of values must not be empty")

            self._num_of = len(value)
            self._size = c.sizeof(value._type_)
            self._alignment = c.alignment(value._type_)
        else:
            self._num_of = 1
            self._size = c.sizeof(value)
            self._alignment = c.alignment(value)

    def object(self):
        """Obtain the original value object.
        """
        return self._object

    def callback(self) -> "Callable":
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

    def object(self):
        """Obtain actual block object in the memory.
        """
        return self._object


class Memory:
    """Representation of memory consisting of relocatable memory blocks.
    """
    def __init__(self, header: "MemoryBlock" = None, blocks: "list[MemoryBlock]" = []):
        """Initialize a memory object.
        """
        self._header = None
        self._blocks = []

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

    def fossilize(self, address: "int" = None) -> "bytearray":
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
            dst_object = type(src_object).from_address(buffer_address + offset)

            c.memmove(c.addressof(dst_object), c.addressof(src_object), block.size())

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
            self._app._instructions.append(Application._InstructionSetToContext(
                key=self._key,
                slot_name=slot_name, slot_indices=slot_indices,
                source_key=value._key))

        elif isinstance(value, Context._Slot) or isinstance(value, Context._Action):
            self._app._instructions.append(Application._InstructionSetToSlot(
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
        NOOP, INIT_FROM_CONTEXT, INIT_FROM_SLOT, INIT_POINTER, COPY, FINAL, \
                SET_TO_VALUE, SET_TO_CONTEXT_DATA, SET_TO_CONTEXT_SLOT, ACT = range(10)

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
                    instr.interface_source_slot.index = ptr_interface_source_slot_indices.address()

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
                    instr.slot.index = ptr_slot_indices.address()

                if ptr_value is not None:
                    instr.value.ptr = ptr_value.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionSetToContext(_Instruction):
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
                    instr.slot.index = ptr_slot_indices.address()
                instr.source_key = ptr_source_key.address()

            return MemoryBlock(CValue(instr, callback=init_instr))

    class _InstructionSetToSlot(_Instruction):
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
                    instr.slot.index = ptr_slot_indices.address()
                instr.source_key = ptr_source_key.address()
                instr.source_slot.name = ptr_source_slot_name.address()
                if ptr_source_slot_indices is not None:
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
                    instr.action.index = ptr_action_indices.address()

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

        elif isinstance(entity, Context._Slot) or isinstance(entity, Context._Action):
            self._instructions.append(Application._InstructionInitPointer(
                key=key,
                value=None))

            self._instructions.append(Application._InstructionSetToSlot(
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
        if not isinstance(params, dict):
            raise TypeError("Parameter list must be of type dict")

        ptr_nodes = [None] * len(params)

        for idx, (key, value) in enumerate(params.items()):
            if not isinstance(value, CValue):
                value = CValue(value)

            if value is not None:
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
                    node.next = c.cast(ptr_nodes[idx + 1].address(), type(node.next))

                node.name = ptr_key.address()
                if ptr_value is not None:
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
    _fields_ = [('type', c.c_int)]


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

class archi_exe_registry_instr_init_from_context_t(c.Structure):
    """Context registry instruction: initialize a new context using interface of a source context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('interface_source_key', c.c_char_p),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]


class archi_exe_registry_instr_init_from_slot_t(c.Structure):
    """Context registry instruction: initialize a new context using interface from a context slot.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('interface_source_key', c.c_char_p),
                ('interface_source_slot', archi_context_op_designator_t),
                ('dparams_key', c.c_char_p),
                ('sparams', c.POINTER(archi_parameter_list_t))]


class archi_exe_registry_instr_init_pointer_t(c.Structure):
    """Context registry instruction: initialize a new pointer context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('value', archi_pointer_t)]


class archi_exe_registry_instr_copy_t(c.Structure):
    """Context registry instruction: create a context alias.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('original_key', c.c_char_p)]


class archi_exe_registry_instr_final_t(c.Structure):
    """Context registry instruction: finalize a context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p)]


class archi_exe_registry_instr_set_to_value_t(c.Structure):
    """Context registry instruction: set context slot to pointer to a value.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_op_designator_t),
                ('value', archi_pointer_t)]


class archi_exe_registry_instr_set_to_context_data_t(c.Structure):
    """Context registry instruction: set context slot to pointer to a source context.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_op_designator_t),
                ('source_key', c.c_char_p)]


class archi_exe_registry_instr_set_to_context_slot_t(c.Structure):
    """Context registry instruction: set context slot to a source context slot.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
                ('slot', archi_context_op_designator_t),
                ('source_key', c.c_char_p),
                ('source_slot', archi_context_op_designator_t)]


class archi_exe_registry_instr_act_t(c.Structure):
    """Context registry instruction: invoke context action.
    """
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('key', c.c_char_p),
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

