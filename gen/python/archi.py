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

###############################################################################

class Application:
    """Representation of a whole application initialization file.
    """
    class _Instruction:
        """Internal representation of an application initialization instruction.
        """
        NOOP, INIT_STATIC, INIT_DYNAMIC, FINAL, \
                SET_VALUE, SET_CONTEXT, SET_SLOT, \
                ACT_STATIC, ACT_DYNAMIC = range(9)

        def __init__(self, type: "int", **kwargs):
            """Initialize an instruction.
            """
            if not isinstance(type, int):
                raise TypeError("Instruction type must an integer")

            self.type = type
            self.info = kwargs

    def __init__(self):
        """Initialize a file.
        """
        self._sections = []
        self._params = {}
        self._instructions = []

    def __getitem__(self, alias: "str") -> "Context":
        """Obtain a context from the registry.
        """
        if not isinstance(alias, str):
            raise TypeError("Alias must be a string")

        return Context(self, alias)

    def __setitem__(self, alias: "str", entity):
        """Initialize a context and insert it to the registry.
        """
        if not isinstance(alias, str):
            raise TypeError("Alias must be a string")

        if isinstance(entity, ParameterList) or isinstance(entity, ContextSpec):
            if entity.dparams() is None: # static parameter list
                self._add_instruction_init_static(
                        key=alias, interface_key=entity.interface() \
                                if isinstance(entity, ContextSpec) else None,
                        sparams=entity.sparams())
            elif len(entity.sparams()) == 0: # dynamic parameter list
                self._add_instruction_init_dynamic(
                        key=alias, interface_key=entity.interface() \
                                if isinstance(entity, ContextSpec) else None,
                        dparams_key=entity.dparams()._alias)
            else: # temporary (dynamic + static) parameter list
                temp_params_alias = f'.~{alias}:{entity.dparams()._alias}'

                self._app._add_instruction_init_dynamic(
                        key=temp_params_alias, interface_key=None,
                        dparams_key=entity.dparams()._alias)
                self._app._add_instruction_act_static(
                        key=temp_params_alias,
                        action_name='_', action_indices=[],
                        sparams=entity.sparams())

                self._add_instruction_init_dynamic(
                        key=alias, interface_key=entity.interface() \
                                if isinstance(entity, ContextSpec) else None,
                        dparams_key=temp_params_alias)

                self._app._add_instruction_final(key=temp_params_alias)
        elif isinstance(entity, Pointer):
            self._add_instruction_init_static(
                    key=alias, interface_key='',
                    sparams={'value': entity})
        elif isinstance(entity, Context):
            self._add_instruction_init_static(
                    key=alias, interface_key='')
            self._add_instruction_set_context(
                    key=alias, slot_name='value', slot_indices=[],
                    source_key=entity._alias)
        elif isinstance(entity, Context._Slot) or isinstance(entity, Context._Action):
            self._add_instruction_init_static(
                    key=alias, interface_key='')
            self._add_instruction_set_slot(
                    key=alias, slot_name='value', slot_indices=[],
                    source_key=entity._context._alias,
                    source_slot_name=entity._name, source_slot_indices=entity._indices)
        else:
            raise TypeError("Context can only be initialized from: ContextSpec, Context._Slot, or Context._Action")

    def __delitem__(self, alias: "str"):
        """Finalize a context and remove it from the registry.
        """
        if not isinstance(alias, str):
            raise TypeError("Alias must be a string")

        self._add_instruction_final(key=alias)

    def params(self) -> "dict[str, Pointer]":
        """Obtain the application parameter list.
        """
        return self._params

    def set_params(self, **params):
        """Set the application parameter list.
        """
        self._params = params

    def allocate(self, data: "DataSpec") -> "Pointer":
        """Request a data section an initialization file.
        """
        if not isinstance(data, DataSpec):
            raise TypeError("Data specification object must be of type DataSpec")

        ptr = Pointer.__new__(Pointer)

        ptr._app = self
        ptr._data = data
        ptr._base_ptr = None
        ptr._num_of = data._num_of
        ptr._offset = 0
        ptr._address = None

        self._sections.append(ptr)

        return ptr

    def _add_instruction_init_static(self, **kwargs):
        self._instructions.append(_Instruction(_Instruction.INIT_STATIC, **kwargs))
    def _add_instruction_init_dynamic(self, **kwargs):
        self._instructions.append(_Instruction(_Instruction.INIT_DYNAMIC, **kwargs))
    def _add_instruction_final(self, **kwargs):
        self._instructions.append(_Instruction(_Instruction.FINAL, **kwargs))
    def _add_instruction_set_value(self, **kwargs):
        self._instructions.append(_Instruction(_Instruction.SET_VALUE, **kwargs))
    def _add_instruction_set_context(self, **kwargs):
        self._instructions.append(_Instruction(_Instruction.SET_CONTEXT, **kwargs))
    def _add_instruction_set_slot(self, **kwargs):
        self._instructions.append(_Instruction(_Instruction.SET_SLOT, **kwargs))
    def _add_instruction_act_static(self, **kwargs):
        self._instructions.append(_Instruction(_Instruction.ACT_STATIC, **kwargs))
    def _add_instruction_act_dynamic(self, **kwargs):
        self._instructions.append(_Instruction(_Instruction.ACT_DYNAMIC, **kwargs))

###############################################################################

class ParameterList:
    """Representation of a parameter list.
    """
    def __init__(self, _: "Context" = None, **params):
        """Create a context parameter list representation instance.
        """
        if _ is not None and not isinstance(_, Context):
            raise TypeError("Dynamic parameter list object must be of type Context or None")
        elif not all(isinstance(v, Pointer) for v in params.values()):
            raise TypeError("Static parameter list object must be a dictionary of type str->Pointer")

        self._dparams = _
        self._sparams = params

    def dparams(self) -> "Context":
        """Obtain the dynamic context parameter list.
        """
        return self._dparams

    def sparams(self) -> "dict[str, Pointer]":
        """Obtain the static context parameter list.
        """
        return self._sparams


class ContextSpec:
    """Representation of a context interface bundled with context initialization parameters.
    """
    def __init__(self, interface: "Context", dparams: "Context" = None,
                 sparams: "dict[str, Pointer]" = {}):
        """Create a context specification instance.
        """
        if not isinstance(interface, Context):
            raise TypeError("Context interface object must be of type Context")
        elif dparams is not None and not isinstance(dparams, Context):
            raise TypeError("Dynamic parameter list object must be of type Context or None")
        elif not isinstance(sparams, dict) \
                or not all(isinstance(k, str) and isinstance(v, Pointer) \
                for k, v in sparams.items()):
            raise TypeError("Static parameter list object must be a dictionary of type str->Pointer")

        self._interface = interface
        self._dparams = dparams
        self._sparams = sparams

    def interface(self) -> "Context":
        """Obtain the context interface from the specification.
        """
        return self._interface

    def dparams(self) -> "Context":
        """Obtain the dynamic context parameter list from the specification.
        """
        return self._dparams

    def sparams(self) -> "dict[str, Pointer]":
        """Obtain the static context parameter list from the specification.
        """
        return self._sparams


class Context:
    """Representation of a context.
    """
    class _Slot:
        """Representation of a context slot.
        """
        def __init__(self, context: "Context", name: "str", indices: "list[int]" = []):
            """Create a context slot representation instance.
            """
            if not isinstance(context, Context):
                raise TypeError("Context object must be of type Context")
            elif not isinstance(name, str):
                raise TypeError("Slot name must be a string")
            elif not isinstance(indices, list) \
                    or not all(isinstance(index, int) for index in indices):
                raise TypeError("Slot indices must be a list of integers")

            self._context = context
            self._name = name
            self._indices = indices

            self._app = context._app

        def __getattr__(self, name: "str") -> "_Slot":
            """Obtain a context slot object.
            """
            if self.indices():
                raise RuntimeError("Indexed slot cannot extend its name using dot (.)")

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
            if isinstance(value, Pointer):
                self._app._add_instruction_set_value(
                        key=self._context._alias,
                        slot_name=f'{self._name}.{name}', slot_indices=self._indices,
                        value=value)
            elif isinstance(value, Context):
                self._app._add_instruction_set_context(
                        key=self._context._alias,
                        slot_name=f'{self._name}.{name}', slot_indices=self._indices,
                        source_key=value._alias)
            elif isinstance(value, _Slot) or isinstance(value, _Action):
                self._app._add_instruction_set_slot(
                        key=self._context._alias,
                        slot_name=f'{self._name}.{name}', slot_indices=self._indices,
                        source_key=value._context._alias,
                        source_slot_name=value._name, source_slot_indices=value._indices)
            else:
                raise TypeError("Context slot can only be set to an object of Pointer, Context, Context._Slot, or Context._Action")

        def __setitem__(self, index: "int", value):
            """Perform a slot setting operation.
            """
            if not isinstance(index, int):
                raise TypeError("Context slot index must be an integer")

            if isinstance(value, Pointer):
                self._app._add_instruction_set_value(
                        key=self._context._alias,
                        slot_name=self._name, slot_indices=self._indices + [index],
                        value=value)
            elif isinstance(value, Context):
                self._app._add_instruction_set_context(
                        key=self._context._alias,
                        slot_name=self._name, slot_indices=self._indices + [index],
                        source_key=value._alias)
            elif isinstance(value, _Slot) or isinstance(value, _Action):
                self._app._add_instruction_set_slot(
                        key=self._context._alias,
                        slot_name=self._name, slot_indices=self._indices + [index],
                        source_key=value._context._alias,
                        source_slot_name=value._name, source_slot_indices=value._indices)
            else:
                raise TypeError("Context slot can only be set to: Pointer, Context, Context._Slot, or Context._Action")

        def __call__(self, _: "Context" = None, **params) -> "_Action":
            """Perform an action.
            """
            if _ is not None and not isinstance(_, Context):
                raise TypeError("Dynamic parameter list object must be of type Context")
            elif not all(isinstance(v, Pointer) for v in params.values()):
                raise TypeError("Static parameter list object must be a dictionary of type str->Pointer")

            if _ is None: # static parameter list
                self._app._add_instruction_act_static(
                        key=self._context._alias,
                        action_name=self._name, action_indices=self._indices,
                        sparams=params)
            elif len(params) == 0: # dynamic parameter list
                self._app._add_instruction_act_dynamic(
                        key=self._context._alias,
                        action_name=self._name, action_indices=self._indices,
                        dparams_key=_._alias)
            else: # temporary (dynamic + static) parameter list
                temp_params_alias = f'.~{self._context._alias}:{_._alias}'

                self._app._add_instruction_init_dynamic(
                        key=temp_params_alias, interface_key=None,
                        dparams_key=_._alias)
                self._app._add_instruction_act_static(
                        key=temp_params_alias,
                        action_name='_', action_indices=[],
                        sparams=params)

                self._app._add_instruction_act_dynamic(
                        key=self._context._alias,
                        action_name=self._name, action_indices=self._indices,
                        dparams_key=temp_params_alias)

                self._app._add_instruction_final(key=temp_params_alias)

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

    def __init__(self, app: "Application", alias: "str"):
        """Create a context representation instance.
        """
        if not isinstance(app, Application):
            raise TypeError("Application object must be of type Application")
        elif not isinstance(alias, str):
            raise TypeError("Alias must be a string")

        self._app = app
        self._alias = alias

    def __getattr__(self, name: "str") -> "_Slot":
        """Obtain a context slot object.
        """
        return _Slot(self, name)

    def __getitem__(self, index: "int") -> "_Slot":
        """Obtain a context slot object.
        """
        if not isinstance(index, int):
            raise TypeError("Slot index must be an integer")

        return _Slot(self, '', [index])

    def __setattr__(self, name: "str", value):
        """Perform a slot setting operation.
        """
        if isinstance(value, Pointer):
            self._app._add_instruction_set_value(
                    key=self._alias, slot_name=name, slot_indices=[],
                    value=value)
        elif isinstance(value, Context):
            self._app._add_instruction_set_context(
                    key=self._alias, slot_name=name, slot_indices=[],
                    source_key=value._alias)
        elif isinstance(value, _Slot) or isinstance(value, _Action):
            self._app._add_instruction_set_slot(
                    key=self._alias, slot_name=name, slot_indices=[],
                    source_key=value._context._alias,
                    source_slot_name=value._name, source_slot_indices=value._indices)
        else:
            raise TypeError("Context slot can only be set to: Pointer, Context, Context._Slot, or Context._Action")

    def __setitem__(self, index: "int", value):
        """Perform a slot setting operation.
        """
        if not isinstance(index, int):
            raise TypeError("Context slot index must be an integer")

        if isinstance(value, Pointer):
            self._app._add_instruction_set_value(
                    key=self._alias, slot_name='', slot_indices=[index],
                    value=value)
        elif isinstance(value, Context):
            self._app._add_instruction_set_context(
                    key=self._alias, slot_name='', slot_indices=[index],
                    source_key=value._alias)
        elif isinstance(value, _Slot) or isinstance(value, _Action):
            self._app._add_instruction_set_slot(
                    key=self._alias, slot_name='', slot_indices=[index],
                    source_key=value._context._alias,
                    source_slot_name=value._name, source_slot_indices=value._indices)
        else:
            raise TypeError("Context slot can only be set to: Pointer, Context, Context._Slot, or Context._Action")

    def __call__(self, _: "Context" = None, **params) -> "ContextSpec":
        """Create a context specification instance.
        """
        return ContextSpec(self, dparams=_, sparams=params)

    @staticmethod
    def alias_of(context: "Context") -> "str":
        """Obtain alias of a context.
        """
        if not isinstance(context, Context):
            raise TypeError("Context object must be of type Context")

        return context._alias

    @staticmethod
    def application_of(context: "Context") -> "str":
        """Obtain owning application of a context.
        """
        if not isinstance(context, Context):
            raise TypeError("Context object must be of type Context")

        return context._app

###############################################################################

class DataSpec:
    """Representation of a data section within an initialization file.
    """
    def __init__(self, contents_fn: "Callable[[int], bytes]",
                 num_of: "int", size: "int", alignment: "int" = 1, flags: "int" = 0):
        """Initialize a data section base.
        """
        import inspect
        import types

        if not isinstance(contents_fn, types.FunctionType):
            raise TypeError("Contents must be a function accepting address and returning bytes object")

        sig = inspect.signature(contents_fn)
        params = list(sig.parameters.values())

        if len(params) != 1:
            raise TypeError("Contents must be a function accepting address and returning bytes object")
        elif params[0].annotation is inspect.Parameter.empty or params[0].annotation != int:
            raise TypeError("Contents must be a function accepting address and returning bytes object")
        elif sig.return_annotation is inspect.Signature.empty or sig.return_annotation != bytes:
            raise TypeError("Contents must be a function accepting address and returning bytes object")

        if not isinstance(num_of, int):
            raise TypeError("Number of data elements must be a positive integer")
        elif num_of <= 0:
            raise ValueError("Number of data elements must be a positive integer")
        elif not isinstance(size, int):
            raise TypeError("Size of a data element must be a positive integer")
        elif size <= 0:
            raise ValueError("Size of a data element must be a positive integer")
        elif not isinstance(alignment, int):
            raise TypeError("Alignment requirement of a data element must be a power of two")
        elif alignment <= 0 or (alignment & (alignment - 1)) != 0:
            raise ValueError("Alignment requirement of a data element must be a power of two")
        elif not isinstance(flags, int):
            raise TypeError("Flags must be a positive integer")
        elif flags < 0:
            raise ValueError("Flags must be a positive integer")

        self._contents_fn = None

        self._num_of = num_of
        self._size = size
        self._alignment = alignment

        self._padded_size = (size + (alignment - 1)) & ~(alignment - 1)
        self._total_size = (num_of - 1) * self._padded_size + size
        self._total_padding = (num_of - 1) * (self._padded_size - size)

        self._flags = flags

    def contents_function(self) -> "Callable[[int], bytes]":
        """Obtain contents generation function.
        """
        return self._contents_fn

    def num_elements(self) -> "int":
        """Obtain number of data elements.
        """
        return self._num_of

    def element_size(self) -> "int":
        """Obtain size of a data element in bytes.
        """
        return self._size

    def element_alignment(self) -> "int":
        """Obtain alignment requirement of a data element in bytes.
        """
        return self._alignment

    def element_size_padded(self) -> "int":
        """Obtain padded size of a data element in bytes.
        """
        return self._padded_size

    def total_size(self) -> "int":
        """Obtain total data section size in bytes.
        """
        return self._total_size

    def total_padding(self) -> "int":
        """Obtain total number of padding bytes.
        """
        return self._total_padding

    def flags(self) -> "int":
        """Obtain flags.
        """
        return self._flags


class Pointer:
    """Representation of a pointer to a data section of initialization file.
    """
    def __init__(self, base_ptr: "Pointer", offset: "int"):
        """Create a pointer representation instance from the base pointer and offset.
        """
        if not isinstance(base_ptr, Pointer):
            raise TypeError("Base pointer object must be of type Pointer")
        elif not isinstance(offset, int):
            raise TypeError("Offset must be a non-negative integer")
        elif offset < 0:
            raise ValueError("Offset must be a non-negative integer")
        elif offset >= base_ptr._num_of:
            raise ValueError("Offset must not point past the data end")

        self._app = base_ptr._app
        self._data = None
        self._base_ptr = base_ptr._base_ptr if base_ptr._base_ptr is not None else base_ptr
        self._num_of = base_ptr._num_of - offset
        self._offset = base_ptr._offset + offset
        self._address = None

    def application(self) -> "Application":
        """Obtain owning application of the pointer.
        """
        return self._app

    def data(self) -> "DataSpec":
        """Obtain data section representation the pointer uses.
        """
        return self._data

    def base_pointer(self) -> "Pointer":
        """Obtain base pointer of the pointer.
        """
        return self._base_ptr

    def num_elements(self) -> "int":
        """Obtain number of data elements pointed to by the pointer.
        """
        return self._num_of

    def offset(self) -> "int":
        """Obtain offset relative to the base pointer.
        """
        return self._offset

    def address(self) -> "int":
        """Obtain address of the pointer.

        The address is known only during fossilization.
        At other times, the function returns None.
        """
        if self._base_ptr is None:
            return self._address
        else:
            base_address = self._base_ptr._address
            padded_size = self._base_ptr._data._padded_size
            return (base_address + self._offset * padded_size) if base_address is not None else None

    @staticmethod
    def address_size() -> "int":
        """Get address size in bytes.
        """
        import ctype
        return ctypes.sizeof(ctypes.c_void_p)

###############################################################################

# TODO signal watch set

# TODO string

# TODO ctypes type

###############################################################################

def fossilize(app: "Application", pathname: "str", maplist: "list[str]" = []):
    """Fossilize an Application object to a memory-mapped initialization file.
    Optionally, map a list of files to prevent possible collisions with the generated file.
    """
    import ctypes as c
    import mmap
    import os

    if not isinstance(app, Application):
        raise TypeError("Application must be an object of type Application")
    elif not isinstance(pathname, str):
        raise TypeError("Pathname must be a string")
    elif not isinstance(maplist, list) or not all(isinstance(p, str) for p in maplist):
        raise TypeError("Pathname list of mapped files must be a list of strings")

    ### Auxiliary types (base) ###

    class archi_file_header_t(c.Structure):
        """Header of a memory-mapped file.
        """
        _fields_ = [('addr', c.c_void_p),
                    ('end', c.c_void_p)]

    class archi_array_layout_t(c.Structure):
        """Array layout description.
        """
        _fields_ = [('num_of', c.c_size_t),
                    ('size', c.c_size_t),
                    ('alignment', c.c_size_t)]

    class archi_pointer_t_union(c.Union):
        """Union of a generic pointer to data and a generic pointer to function.
        """
        _fields_ = [('ptr', c.c_void_p),
                    ('fptr', c.CFUNCTYPE(None))]

    class archi_pointer_t(c.Structure):
        """Generic wrapper for data or function pointers with metadata.
        """
        _fields_ = [('as', archi_pointer_t_union),
                    ('ref_count', c.c_void_p),
                    ('flags', c.c_uint64),
                    ('element', archi_array_layout_t)]
        _anonymous_ = ['as']

    class archi_context_parameter_list_t(c.Structure):
        """List of named values.
        """
        pass

    archi_context_parameter_list_t._fields_ = \
            [('next', c.POINTER(archi_context_parameter_list_t)),
             ('name', c.c_char_p),
             ('value', archi_pointer_t)]

    class archi_context_op_designator_t(c.Structure):
        """Context operation designator.
        """
        _fields_ = [('name', c.c_char_p),
                    ('index', c.POINTER(c.c_size_t)),
                    ('num_indices', c.c_size_t)]

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

    class archi_exe_input_t(c.Structure):
        """Description of an input file for the executable.
        """
        MAGIC = "[archi]"

        _fields_ = [('header', archi_file_header_t),
                    ('magic', c.c_char * 8),
                    ('params', c.POINTER(archi_context_parameter_list_t)),
                    ('instructions', c.POINTER(archi_exe_registry_instr_list_t))]

    ### Auxiliary types (instructions) ###

    class archi_exe_registry_instr_union_params(c.Union):
        """Union of a pointer to static parameter list and a pointer to dynamic parameter list key.
        """
        _fields_ = [('sparams', c.POINTER(archi_context_parameter_list_t)),
                    ('dparams_key', c.c_char_p)]

    class archi_exe_registry_instr_init_t(c.Structure):
        """Context registry instruction: initialize a new context.
        """
        _fields_ = [('base', archi_exe_registry_instr_base_t),
                    ('interface_key', c.c_char_p),
                    ('params', archi_exe_registry_instr_union_params)]
        _anonymous_ = ['params']

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
        _fields_ = [('base', archi_exe_registry_instr_base_t),
                    ('action', archi_context_op_designator_t),
                    ('params', archi_exe_registry_instr_union_params)]
        _anonymous_ = ['params']

    ### Auxiliary functions ###

    total_size = 0      # total file size
    total_padding = 0   # total number of padding bytes used

    section_header = None       # the header section (cannot be moved)
    section_instructions = []   # list of instructions
    section_strings = {}        # dict of strings

    # TODO

    ### Fossilization steps ###

    # TODO

