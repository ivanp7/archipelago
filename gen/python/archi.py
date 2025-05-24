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
# Representation of generic memory-mapped files with pointer support
###############################################################################

class CValue:
    """Wrapper around a ctypes value plus a callback function to finalize writing to the mapped memory.
    """
    def __init__(self, value: "c._CData", callback: "Callable[[c._CData]]" = None, **attributes):
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

        This callback function is called on the value copy written to the mapped memory,
        after all file sections have been assigned addresses.
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


class FileSection:
    """Representation of a memory-mapped file section.
    """
    def __init__(self, value: "CValue"):
        """Initialize a file section.
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
        """Obtain section size in bytes.
        """
        return c.sizeof(self._value.object())

    def alignment(self) -> "int":
        """Obtain section alignment requirement in bytes.
        """
        return c.alignment(self._value.object())

    def owner(self) -> "File":
        """Obtain file owning this section.
        """
        return self._owner

    def address(self) -> "int":
        """Obtain actual section address in the mapped memory.
        """
        return self._address


class File:
    """Representation of a memory-mapped file.
    """
    def __init__(self, header: "FileSection" = None, sections: "list[FileSection]" = []):
        """Initialize a file.
        """
        self.set_header_section(header)
        self.set_sections(sections)

    def header_section(self) -> "FileSection":
        """Obtain the header section of the file.
        """
        return self._header

    def set_header_section(self, header: "FileSection"):
        """Set the header section of the file.
        """
        if header is self._header:
            return

        if header is not None and not isinstance(header, FileSection):
            raise TypeError("File header object must be of type FileSection")
        elif header is not None and header._owner is not None:
            raise RuntimeError("File section is already owned by another file")
        elif header in self._sections:
            raise RuntimeError("Header section cannot be in the list of regular sections")

        if self._header is not None:
            self._header._owner = None

        self._header = header
        self._header._owner = self

        self._reset_size()

    def sections(self) -> "list[FileSection]":
        """Obtain the list of sections of the file.
        """
        return self._sections.copy()

    def set_sections(self, sections: "list[FileSection]"):
        """Set the list of sections of the file.
        """
        if not isinstance(sections, list) \
                or not all(isinstance(section, FileSection) for section in sections):
            raise TypeError("List of sections must be of type list[FileSection]")
        elif not all(section._owner is None or section._owner is self for section in sections):
            raise RuntimeError("Some of sections in the list are owned by another file")
        elif self._header in sections:
            raise RuntimeError("Header section cannot be in the list of regular sections")
        elif len(sections) != len(set(sections)):
            raise TypeError("List of sections cannot contain duplicate elements")

        for section in self._sections:
            section._owner = None

        self._sections = sections.copy()

        for section in self._sections:
            section._owner = self

        self._reset_size()

    def pack(self):
        """Reorder the sections to decrease the file size.
        """
        from functools import cmp_to_key

        # Sort sections by descending alignment (for equal alignments, by descending size)
        def compare(section1, section2):
            return section2.alignment() - section1.alignment() \
                    if section2.alignment() != section1.alignment() \
                    else section2.size() - section1.size()

        self._sections.sort(key=cmp_to_key(compare))

        # Pack sorted list of sections
        sections = {} # offset -> FileSection

        gaps = []
        end = self._header.size() if self._header is not None else 0

        for section in self._sections:
            alignment = section.alignment()
            size = section.size()

            # Try to fit the section into one of gaps
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

            # Insert the section into the dictionary and update gaps
            if gap_used is None:
                address = (end + (alignment - 1)) & ~(alignment - 1)
                if address > end:
                    gaps.append((end, address - end))

                end = address + size
            else:
                address = gap_addr_align
                gaps[gap_used:gap_used+1] = new_gaps

            sections[address] = section

        # Create the new list of sections
        self._sections = [section for _, section in sorted(sections.items())]
        self._reset_size()

    def size(self) -> "int":
        """Obtain the total size of the file in bytes.
        """
        if self._size is None:
            self._calculate_size()

        return self._size

    def padding(self) -> "int":
        """Obtain the total number of padding bytes in the file.
        """
        if self._padding is None:
            self._calculate_size()

        return self._padding

    def write(self, pathname: "str", map_address: "int" = None):
        """Write the file to file system.
        """
        import mmap
        import os

        # Calculate the total size
        size = self.size()

        # Create a memory-mapped file
        fd = os.open(pathname, os.O_CREAT | os.O_TRUNC | os.O_RDWR)

        try:
            # Allocate space
            os.write(fd, b'\x00' * size)

            # Map the file to memory
            mm = mmap.mmap(fd, size, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)

        finally:
            os.close(fd)

        try:
            address = c.addressof(c.c_char.from_buffer(mm))

            if map_address is None:
                map_address = address

            sections = [self._header] if self._header is not None else []
            sections += self._sections

            # Assign addresses to the sections and copy their contents to the mapped memory
            offset = 0

            for section in sections:
                alignment = section.alignment()
                offset = (offset + (alignment - 1)) & ~(alignment - 1)

                src_object = section.value().object()
                dst_object = src_object.from_address(address + offset)

                c.memmove(dst_object, src_object, section.size())

                section._address = map_address + offset
                section._object = dst_object

                offset += section.size()

            # Call the section callbacks
            for section in sections:
                callback = section.value().callback()

                if callback is not None:
                    callback(section._object)

            # Reset section addresses
            for section in sections:
                section._address = None
                section._object = None

        finally:
            # Unmap the file memory
            mm.close()

    def _reset_size(self):
        self._size = None
        self._padding = None

    def _calculate_size(self):
        size = self._header.size() if self._header is not None else 0
        padding = 0

        for section in self._sections:
            alignment = section.alignment()
            padded_size = (size + (alignment - 1)) & ~(alignment - 1)

            padding += padded_size - size
            size = padded_size

            size += section.size()

        self._size = size
        self._padding = padding

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

        if dparams is None: # static parameter list
            self._app._instructions.append(Application._InstructionActStatic(
                key=self._key,
                action_name=action_name, action_indices=action_indices,
                sparams=sparams))

        elif len(sparams) == 0: # dynamic parameter list
            self._app._instructions.append(Application._InstructionActDynamic(
                key=self._key,
                action_name=action_name, action_indices=action_indices,
                dparams_key=dparams._key))

        else: # temporary (dynamic + static) parameter list
            temp_params_key = f'.~{self._key}:{dparams._key}'

            self._app._instructions.append(Application._InstructionInitDynamic(
                key=temp_params_key,
                interface_key=None, dparams_key=dparams._key))

            self._app._instructions.append(Application._InstructionActStatic(
                key=temp_params_key,
                action_name='_', action_indices=[],
                sparams=sparams))

            self._app._instructions.append(Application._InstructionActDynamic(
                key=self._key,
                action_name=action_name, action_indices=action_indices,
                dparams_key=temp_params_key))

            self._app._instructions.append(Application._InstructionFinal(
                key=temp_params_key))

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
        NOOP, INIT_STATIC, INIT_DYNAMIC, FINAL, \
                SET_VALUE, SET_CONTEXT, SET_SLOT, \
                ACT_STATIC, ACT_DYNAMIC = range(9)

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

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
            """
            instr = archi_exe_registry_instr_base_t()
            instr.type = self._type

            return FileSection(CValue(instr))

    class _InstructionInitStatic(_Instruction):
        """Representation of an application initialization instruction:
        initialize a context using static parameter list.
        """
        def __init__(self, key: "str", interface_key: "str", sparams: "dict"):
            """Initialize an instruction.
            """
            if not isinstance(interface_key, str):
                raise TypeError("Context interface key must be a string")
            elif not isinstance(sparams, dict):
                raise TypeError("Context specification object must be of type ContextSpec")

            self._type = INIT_STATIC
            _Instruction.__init__(key)
            self._interface_key = interface_key
            self._sparams = sparams

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
            """
            instr = archi_exe_registry_instr_init_t()
            instr.base.type = self._type

            ptr_key = app._alloc_string(self._key)

            ptr_interface_key = app._alloc_string(self._interface_key)
            ptr_sparams = app._alloc_params(self._sparams)

            def init_instr(instr: "archi_exe_registry_instr_init_t"):
                instr.base.key = ptr_key.address()

                instr.interface_key = ptr_interface_key.address()
                instr.sparams = ptr_sparams.address()

            return FileSection(CValue(instr, callback=init_instr))

    class _InstructionInitDynamic(_Instruction):
        """Representation of an application initialization instruction:
        initialize a context using dynamic parameter list.
        """
        def __init__(self, key: "str", interface_key: "str", dparams_key: "str"):
            """Initialize an instruction.
            """
            if not isinstance(interface_key, str):
                raise TypeError("Context interface key must be a string")
            elif not isinstance(dparams_key, str):
                raise TypeError("Dynamic parameter list key must be a string")

            self._type = INIT_DYNAMIC
            _Instruction.__init__(key)
            self._interface_key = interface_key
            self._dparams_key = dparams_key

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
            """
            instr = archi_exe_registry_instr_init_t()
            instr.base.type = self._type

            ptr_key = app._alloc_string(self._key)

            ptr_interface_key = app._alloc_string(self._interface_key)
            ptr_dparams_key = app._alloc_string(self._dparams_key)

            def init_instr(instr: "archi_exe_registry_instr_init_t"):
                instr.base.key = ptr_key.address()

                instr.interface_key = ptr_interface_key.address()
                instr.dparams_key = ptr_dparams_key.address()

            return FileSection(CValue(instr, callback=init_instr))

    class _InstructionFinal(_Instruction):
        """Representation of an application initialization instruction: finalize a context.
        """
        def __init__(self, key: "str"):
            """Initialize an instruction.
            """
            self._type = FINAL
            _Instruction.__init__(key)

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
            """
            instr = archi_exe_registry_instr_base_t()
            instr.type = self._type

            ptr_key = app._alloc_string(self._key)

            def init_instr(instr: "archi_exe_registry_instr_base_t"):
                instr.base.key = ptr_key.address()

            return FileSection(CValue(instr, callback=init_instr))

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

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
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

            return FileSection(CValue(instr, callback=init_instr))

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

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
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

            return FileSection(CValue(instr, callback=init_instr))

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

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
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

            return FileSection(CValue(instr, callback=init_instr))

    class _InstructionActStatic(_Instruction):
        """Representation of an application initialization instruction:
        invoke a context action using static parameter list.
        """
        def __init__(self, key: "str",
                     action_name: "str", action_indices: "list[int]", sparams: "dict"):
            """Initialize an instruction.
            """
            if not isinstance(action_name, str):
                raise TypeError("Action name must be a string")
            elif not isinstance(action_indices, list) \
                    or not all(isinstance(index, int) for index in action_indices):
                raise TypeError("Action indices must be a list of integers")
            elif not isinstance(sparams, dict):
                raise TypeError("Context specification object must be of type ContextSpec")

            self._type = ACT_STATIC
            _Instruction.__init__(key)
            self._action_name = action_name
            self._action_indices = action_indices
            self._sparams = sparams

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
            """
            instr = archi_exe_registry_instr_act_t()
            instr.base.type = self._type
            instr.action.num_indices = len(self._action_indices)

            ptr_key = app._alloc_string(self._key)

            ptr_action_name = app._alloc_string(self._action_name)
            ptr_action_indices = app._alloc_index_array(self._action_indices)
            ptr_sparams = app._alloc_params(self._sparams)

            def init_instr(instr: "archi_exe_registry_instr_act_t"):
                instr.base.key = ptr_key.address()

                instr.action.name = ptr_action_name.address()
                instr.action.index = ptr_action_indices.address()
                instr.sparams = ptr_sparams.address()

            return FileSection(CValue(instr, callback=init_instr))

    class _InstructionActDynamic(_Instruction):
        """Representation of an application initialization instruction:
        invoke a context action using dynamic parameter list.
        """
        def __init__(self, key: "str",
                     action_name: "str", action_indices: "list[int]", dparams_key: "str"):
            """Initialize an instruction.
            """
            if not isinstance(action_name, str):
                raise TypeError("Action name must be a string")
            elif not isinstance(action_indices, list) \
                    or not all(isinstance(index, int) for index in action_indices):
                raise TypeError("Action indices must be a list of integers")
            elif not isinstance(dparams_key, str):
                raise TypeError("Dynamic parameter list key must be a string")

            self._type = ACT_DYNAMIC
            _Instruction.__init__(key)
            self._action_name = action_name
            self._action_indices = action_indices
            self._dparams_key = dparams_key

        def alloc(self, app: "Application", ptr_instructions: "list[FileSection]", idx: "int"):
            """Allocate all required sections.
            """
            instr = archi_exe_registry_instr_act_t()
            instr.base.type = self._type
            instr.action.num_indices = len(self._action_indices)

            ptr_key = app._alloc_string(self._key)

            ptr_action_name = app._alloc_string(self._action_name)
            ptr_action_indices = app._alloc_index_array(self._action_indices)
            ptr_dparams_key = app._alloc_string(self._dparams_key)

            def init_instr(instr: "archi_exe_registry_instr_act_t"):
                instr.base.key = ptr_key.address()

                instr.action.name = ptr_action_name.address()
                instr.action.index = ptr_action_indices.address()
                instr.dparams_key = ptr_dparams_key.address()

            return FileSection(CValue(instr, callback=init_instr))

    INSTRUCTIONS_KEY = 'archi.instructions'

    def __init__(self):
        """Initialize a file.
        """
        self._contents = {}
        self._instructions = []
        self._sections = {} # CValue -> FileSection

        self._ptr_sections = []
        self._ptr_values = {}       # CValue -> FileSection
        self._ptr_strings = {}      # str -> FileSection
        self._ptr_index_arrays = {} # list[int] -> FileSection

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

            if params.dynamic_list() is None: # static parameter list
                self._instructions.append(_InstructionInitStatic(
                    key=key,
                    interface_key=interface_key,
                    sparams=params.static_list()))

            elif len(params.static_list()) == 0: # dynamic parameter list
                self._instructions.append(_InstructionInitDynamic(
                    key=key,
                    interface_key=interface_key,
                    dparams_key=params.dynamic_list()._key))

            else: # temporary (dynamic + static) parameter list
                temp_params_key = f'.~{key}:{entity.dynamic_list()._key}'

                self._instructions.append(_InstructionInitDynamic(
                    key=temp_params_key,
                    interface_key=None,
                    dparams_key=params.dynamic_list()._key))

                self._instructions.append(_InstructionActStatic(
                    key=temp_params_key,
                    action_name='_', action_indices=[],
                    sparams=params.static_list()))

                self._instructions.append(_InstructionInitDynamic(
                    key=key,
                    interface_key=interface_key,
                    dparams_key=temp_params_key))

                self._instructions.append(_InstructionFinal(
                    key=temp_params_key))

        elif isinstance(entity, c._CData) or isinstance(entity, CValue):
            self._instructions.append(_InstructionInitStatic(
                key=key,
                interface_key='',
                sparams={'value': CValue(entity) if isinstance(entity, c._CData) else entity}))

        elif isinstance(entity, Context):
            temp_params_key = f'.~{key}:{entity._key}'

            self._instructions.append(_InstructionInitStatic(
                key=temp_params_key,
                interface_key=None,
                sparams={}))

            self._instructions.append(_InstructionSetContext(
                key=temp_params_key,
                slot_name='value', slot_indices=[],
                source_key=entity._key))

            self._instructions.append(_InstructionInitDynamic(
                key=key,
                interface_key='',
                dparams_key=temp_params_key))

            self._instructions.append(_InstructionFinal(
                key=temp_params_key))

        elif isinstance(entity, Context._Slot) or isinstance(entity, Context._Action):
            temp_params_key = f'.~{key}:{entity._name}{entity._indices}'

            self._instructions.append(_InstructionInitStatic(
                key=temp_params_key,
                interface_key=None,
                sparams={}))

            self._instructions.append(_InstructionSetSlot(
                key=temp_params_key,
                slot_name='value', slot_indices=[],
                source_key=entity._context._key,
                source_slot_name=entity._name, source_slot_indices=entity._indices))

            self._instructions.append(_InstructionInitDynamic(
                key=key,
                interface_key='',
                dparams_key=temp_params_key))

            self._instructions.append(_InstructionFinal(
                key=temp_params_key))

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
        elif INSTRUCTIONS_KEY in contents:
            raise KeyError(f"Application contents list cannot contain key '{INSTRUCTIONS_KEY}'")

        self._contents = contents

    def add_section(self, value: "CValue") -> "FileSection":
        """Add a file section to the list of sections.
        """
        section = FileSection(value)
        self._sections[value] = section
        return section

    def reset(self):
        """Reset the list of instructions.
        """
        self._instructions = []
        self._sections = {}

    def address_of(self, entity) -> "int":
        """Obtain actual address of an entity in the mapped memory.
        This method is to be used in CValue callbacks while .file() is executing,
        at other times it will return None.
        """
        if isinstance(entity, CValue):
            section = self._ptr_values.get(entity) or self._sections.get(entity)
        elif isinstance(entity, str):
            section = self._ptr_strings.get(entity)
        elif isinstance(entity, list):
            section = self._ptr_index_arrays.get(tuple(entity))
        else:
            raise TypeError("Application only stores entities of type: CValue, str, list[int]")

        return section.address() if section is not None else None

    def file(self) -> "File":
        """Generate a File object from the application contents.
        """
        # Initialize the list of sections
        self._ptr_sections = list(self._sections.values())

        # Allocate all sections required for the instructions
        ptr_instructions = [None] * len(self._instructions)
        for idx, instruction in enumerate(self._instructions):
            ptr_instr = instruction.alloc(self, ptr_instructions, idx)
            self._ptr_sections.append(ptr_instr)

            node = archi_exe_registry_instr_list_t()

            def init_instructions_node(node: "archi_exe_registry_instr_list_t",
                                       idx=idx, ptr_instr=ptr_instr):
                if idx < len(ptr_instructions):
                    node.next = ptr_instructions[idx + 1].address()

                node.instruction = ptr_instr.address()

            ptr_instructions[idx] = FileSection(CValue(node, callback=init_instructions_node))

        self._ptr_sections += ptr_instructions

        # Allocate custom file contents
        ptr_contents = self._alloc_params(self._contents)

        # Insert the instructions to the file contents:
        if ptr_instructions:
            node = archi_parameter_list_t()

            node.value.element.num_of = 1
            node.value.element.size = c.sizeof(archi_exe_registry_instr_list_t)
            node.value.element.alignment = c.alignment(archi_exe_registry_instr_list_t)

            ptr_key = self._alloc_string(INSTRUCTIONS_KEY)

            def init_contents_node(node: "archi_parameter_list_t",
                                   ptr_contents=ptr_contents, ptr_key=ptr_key,
                                   ptr_instructions=ptr_instructions[0]):
                if ptr_contents is not None:
                    node.next = ptr_contents.address()

                node.name = ptr_key.address()
                node.value.ptr = ptr_instructions.address()

            ptr_contents = FileSection(CValue(node, callback=init_contents_node))
            self._ptr_sections.append(ptr_contents)

        # Initialize the file header and allocate the section
        file_header = archi_exe_input_file_header_t()
        file_header.magic[:len(archi_exe_input_file_header_t.MAGIC)] = \
                archi_exe_input_file_header_t.MAGIC.encode()
        file_header.magic[len(archi_exe_input_file_header_t.MAGIC)] = 0

        file = None
        ptr_header = None

        def init_header(file_header):
            file_header.header.addr = ptr_header.address()
            file_header.header.end  = ptr_header.address() + file.size()

            file_header.contents = ptr_contents.address()

        ptr_header = FileSection(CValue(file_header, callback=init_header))

        # Initialize the file object
        file = File(ptr_header, self._ptr_sections)

        # Reset auxiliary storage
        self._ptr_sections = []
        self._ptr_values = {}
        self._ptr_strings = {}
        self._ptr_index_arrays = {}

        return file

    def _alloc_value(self, value) -> "FileSection":
        if not isinstance(value, CValue):
            raise TypeError("A value must be of type CValue")

        if value not in self._ptr_values:
            section = FileSection(value)

            self._ptr_values[value] = section
            self._ptr_sections.append(section)
        else:
            section = self._ptr_values[value]

        return section

    def _alloc_string(self, string: "str") -> "FileSection":
        if not isinstance(string, str):
            raise TypeError("A string must be of type str")

        if string not in self._ptr_strings:
            section = FileSection(CValue(c.create_string_buffer(string.encode())))

            self._ptr_strings[string] = section
            self._ptr_sections.append(section)
        else:
            section = self._ptr_strings[string]

        return section

    def _alloc_index_array(self, index_array: "list[int]") -> "FileSection":
        if not isinstance(index_array, list):
            raise TypeError("An index array must be of type list")

        index_array = tuple(index_array)

        if index_array not in self._ptr_index_arrays:
            section = FileSection(CValue((c.c_size_t * len(index_array))(*index_array)))

            self._ptr_index_arrays[index_array] = section
            self._ptr_sections.append(section)
        else:
            section = self._ptr_index_arrays[index_array]

        return section

    def _alloc_params(self, params: "dict") -> "FileSection":
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

            ptr_nodes[idx] = FileSection(CValue(node, callback=init_list_node))

        self._ptr_sections += ptr_nodes
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

class archi_exe_registry_instr_params_union(c.Union):
    """Union of a pointer to static parameter list and a pointer to dynamic parameter list key.
    """
    _fields_ = [('sparams', c.POINTER(archi_parameter_list_t)),
                ('dparams_key', c.c_char_p)]


class archi_exe_registry_instr_init_t(c.Structure):
    """Context registry instruction: initialize a new context.
    """
    _anonymous_ = ['params']
    _fields_ = [('base', archi_exe_registry_instr_base_t),
                ('interface_key', c.c_char_p),
                ('params', archi_exe_registry_instr_params_union)]


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
                ('params', archi_exe_registry_instr_params_union)]

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

