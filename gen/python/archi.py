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
        HALT = -1
        NOOP, INIT_STATIC, INIT_DYNAMIC, FINAL, \
                SET_VALUE, SET_CONTEXT, SET_SLOT, \
                ACT_STATIC, ACT_DYNAMIC = range(9)

        def __init__(self, type: int, **kwargs):
            """Initialize an instruction.
            """
            if not isinstance(type, int):
                raise TypeError("Instruction type must an integer")

            self.type = type
            self.info = kwargs

    def __init__(self, **params):
        """Initialize a file.
        """
        self._params = params
        self._instructions = []

    def __getitem__(self, alias: str) -> Context:
        """Obtain a context from the registry.
        """
        if not isinstance(alias, str):
            raise TypeError("Alias must be a string")

        return Context(self, alias)

    def __setitem__(self, alias: str, entity):
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
                        dparams_key=entity.dparams().alias())
            else: # temporary (dynamic + static) parameter list
                temp_params_alias = f'.~{alias}:{entity.dparams().alias()}'

                self._app._add_instruction_init_dynamic(
                        key=temp_params_alias, interface_key=None,
                        dparams_key=entity.dparams().alias())
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
                    source_key=entity.alias())
        elif isinstance(entity, Context.Slot) or isinstance(entity, Context.Action):
            self._add_instruction_init_static(
                    key=alias, interface_key='')
            self._add_instruction_set_slot(
                    key=alias, slot_name='value', slot_indices=[],
                    source_key=entity.context().alias(),
                    source_slot_name=entity.name(), source_slot_indices=entity.indices())
        else:
            raise TypeError("Context can only be initialized from: ContextSpec, Context.Slot, or Context.Action")

    def __delitem__(self, alias: str):
        """Finalize a context and remove it from the registry.
        """
        if not isinstance(alias, str):
            raise TypeError("Alias must be a string")

        self._add_instruction_final(key=alias)

    def params(self) -> dict:
        """Obtain the application parameter list.
        """
        return self._params

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
    def __init__(self, _: Context=None, **params):
        """Create a context parameter list representation instance.
        """
        if _ is not None and not isinstance(_, Context):
            raise TypeError("Dynamic parameter list object must be of type Context or None")
        elif not all(isinstance(v, Pointer) for v in params.values()):
            raise TypeError("Static parameter list object must be a dictionary of type str->Pointer")

        self._dparams = _
        self._sparams = params

    def dparams(self) -> Context:
        """Obtain the dynamic context parameter list.
        """
        return self._dparams

    def sparams(self) -> dict:
        """Obtain the static context parameter list.
        """
        return self._sparams


class ContextSpec:
    """Representation of a context interface bundled with context initialization parameters.
    """
    def __init__(self, interface: Context, dparams: Context=None, sparams: dict={}):
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

    def interface(self) -> Context:
        """Obtain the context interface from the specification.
        """
        return self._interface

    def dparams(self) -> Context:
        """Obtain the dynamic context parameter list from the specification.
        """
        return self._dparams

    def sparams(self) -> dict:
        """Obtain the static context parameter list from the specification.
        """
        return self._sparams


class Context:
    """Representation of a context.
    """
    class Slot:
        """Representation of a context slot.
        """
        def __init__(self, context: Context, name: str, indices: list=[]):
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

        def __getattr__(self, name: str) -> Slot:
            """Obtain a context slot object.
            """
            if self.indices():
                raise RuntimeError("Indexed slot cannot extend its name using dot (.)")

            return Slot(self.context(), f'{self.name()}.{name}')

        def __getitem__(self, index: int) -> Slot:
            """Obtain a context slot object.
            """
            if not isinstance(index, int):
                raise TypeError("Slot index must be an integer")

            return Slot(self.context(), self.name(), self.indices() + [index])

        def __setattr__(self, name: str, value):
            """Perform a slot setting operation.
            """
            if isinstance(value, Pointer):
                self._app._add_instruction_set_value(
                        key=self.context().alias(),
                        slot_name=f'{self.name()}.{name}', slot_indices=self.indices(),
                        value=value)
            elif isinstance(value, Context):
                self._app._add_instruction_set_context(
                        key=self.context().alias(),
                        slot_name=f'{self.name()}.{name}', slot_indices=self.indices(),
                        source_key=value.alias())
            elif isinstance(value, Slot) or isinstance(value, Action):
                self._app._add_instruction_set_slot(
                        key=self.context().alias(),
                        slot_name=f'{self.name()}.{name}', slot_indices=self.indices(),
                        source_key=value.context().alias(),
                        source_slot_name=value.name(), source_slot_indices=value.indices())
            else:
                raise TypeError("Context slot can only be set to an object of Pointer, Context, Context.Slot, or Context.Action")

        def __setitem__(self, index: int, value):
            """Perform a slot setting operation.
            """
            if not isinstance(index, int):
                raise TypeError("Context slot index must be an integer")

            if isinstance(value, Pointer):
                self._app._add_instruction_set_value(
                        key=self.context().alias(),
                        slot_name=self.name(), slot_indices=self.indices() + [index],
                        value=value)
            elif isinstance(value, Context):
                self._app._add_instruction_set_context(
                        key=self.context().alias(),
                        slot_name=self.name(), slot_indices=self.indices() + [index],
                        source_key=value.alias())
            elif isinstance(value, Slot) or isinstance(value, Action):
                self._app._add_instruction_set_slot(
                        key=self.context().alias(),
                        slot_name=self.name(), slot_indices=self.indices() + [index],
                        source_key=value.context().alias(),
                        source_slot_name=value.name(), source_slot_indices=value.indices())
            else:
                raise TypeError("Context slot can only be set to: Pointer, Context, Context.Slot, or Context.Action")

        def __call__(self, _: Context=None, **params) -> Action:
            """Perform an action.
            """
            if _ is not None and not isinstance(_, Context):
                raise TypeError("Dynamic parameter list object must be of type Context")
            elif not all(isinstance(v, Pointer) for v in params.values()):
                raise TypeError("Static parameter list object must be a dictionary of type str->Pointer")

            if _ is None: # static parameter list
                self._app._add_instruction_act_static(
                        key=self.context().alias(),
                        action_name=self.name(), action_indices=self.indices(),
                        sparams=params)
            elif len(params) == 0: # dynamic parameter list
                self._app._add_instruction_act_dynamic(
                        key=self.context().alias(),
                        action_name=self.name(), action_indices=self.indices(),
                        dparams_key=_.alias())
            else: # temporary (dynamic + static) parameter list
                temp_params_alias = f'.~{self.context().alias()}:{_.alias()}'

                self._app._add_instruction_init_dynamic(
                        key=temp_params_alias, interface_key=None,
                        dparams_key=_.alias())
                self._app._add_instruction_act_static(
                        key=temp_params_alias,
                        action_name='_', action_indices=[],
                        sparams=params)

                self._app._add_instruction_act_dynamic(
                        key=self.context().alias(),
                        action_name=self.name(), action_indices=self.indices(),
                        dparams_key=temp_params_alias)

                self._app._add_instruction_final(key=temp_params_alias)

            return Action(self.context(), self.name(), self.indices())

        def context(self) -> Context:
            """Get the context the slot belongs to.
            """
            return self._context

        def name(self) -> str:
            """Get the slot name.
            """
            return self._name

        def indices(self):
            """Get the slot indices
            """
            return self._indices

    class Action:
        """Representation of a context action.
        """
        def __init__(self, context: Context, name: str, indices: list=[]):
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

        def context(self) -> Context:
            """Get the context the action belongs to.
            """
            return self._context

        def name(self) -> str:
            """Get the action name.
            """
            return self._name

        def indices(self):
            """Get the action indices.
            """
            return self._indices

    def __init__(self, app: Application, alias: str):
        """Create a context representation instance.
        """
        if not isinstance(app, Application):
            raise TypeError("Application object must be of type Application")
        elif not isinstance(alias, str):
            raise TypeError("Alias must be a string")

        self._app = app
        self._alias = alias

    def __getattr__(self, name: str) -> Slot:
        """Obtain a context slot object.
        """
        return Slot(self, name)

    def __getitem__(self, index: int) -> Slot:
        """Obtain a context slot object.
        """
        if not isinstance(index, int):
            raise TypeError("Slot index must be an integer")

        return Slot(self, '', [index])

    def __setattr__(self, name: str, value):
        """Perform a slot setting operation.
        """
        if isinstance(value, Pointer):
            self._app._add_instruction_set_value(
                    key=self.alias(), slot_name=name, slot_indices=[],
                    value=value)
        elif isinstance(value, Context):
            self._app._add_instruction_set_context(
                    key=self.alias(), slot_name=name, slot_indices=[],
                    source_key=value.alias())
        elif isinstance(value, Slot) or isinstance(value, Action):
            self._app._add_instruction_set_slot(
                    key=self.alias(), slot_name=name, slot_indices=[],
                    source_key=value.context().alias(),
                    source_slot_name=value.name(), source_slot_indices=value.indices())
        else:
            raise TypeError("Context slot can only be set to: Pointer, Context, Context.Slot, or Context.Action")

    def __setitem__(self, index: int, value):
        """Perform a slot setting operation.
        """
        if not isinstance(index, int):
            raise TypeError("Context slot index must be an integer")

        if isinstance(value, Pointer):
            self._app._add_instruction_set_value(
                    key=self.alias(), slot_name='', slot_indices=[index],
                    value=value)
        elif isinstance(value, Context):
            self._app._add_instruction_set_context(
                    key=self.alias(), slot_name='', slot_indices=[index],
                    source_key=value.alias())
        elif isinstance(value, Slot) or isinstance(value, Action):
            self._app._add_instruction_set_slot(
                    key=self.alias(), slot_name='', slot_indices=[index],
                    source_key=value.context().alias(),
                    source_slot_name=value.name(), source_slot_indices=value.indices())
        else:
            raise TypeError("Context slot can only be set to: Pointer, Context, Context.Slot, or Context.Action")

    def __call__(self, _: Context=None, **params) -> ContextSpec:
        """Create a context specification instance.
        """
        return ContextSpec(self, dparams=_, sparams=params)

    def application(self) -> Application:
        """Get the application the context belongs to.
        """
        return self._app

    def alias(self) -> str:
        """Get the context alias.
        """
        return self._alias

###############################################################################

class Pointer:
    """Representation of a pointer into a data section of initialization file.
    """
    def __init__(self, num_of: int, size: int, alignment: int=1): # TODO: take lambda accepting memory pointer and generating content bytes
        """Create a pointer representation instance.
        """
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

        self._num_of = num_of
        self._size = size
        self._alignment = alignment

        # TODO calculate full size of the memory

    # TODO: getters & setters

###############################################################################

def fossilize(app: Application, pathname: str):
    """Fossilize an Application object to a memory-mapped initialization file.
    """
    if not isinstance(app, Application):
        raise TypeError("Application must be an object of type Application")
    elif not isinstance(pathname, str):
        raise TypeError("Pathname must be a string")

    # TODO

