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
# @brief Context creation helpers.

import ctypes as c
import re
from types import NoneType

import archi.ctypes as typ
from .object import PrimitiveData, String
from archi.context import (
        TypeAttr,
        Context,
        AggregateContext,
        DexgraphOperationSeqContext,
        DexgraphNodeContext,
        ThreadGroupContext,
        AggregateInterfaceSymbol,
        AggregateTypeSymbol,
        DexgraphOperationDataSymbol,
        DexgraphTransitionDataSymbol,
        DexgraphConditionDataSymbol,
        MemoryInterfaceSymbol,
        )
from .registry import Registry


class Helper:
    """A collection of helper functions.
    """
    def __init__(self, registry, /, prefix=''):
        """Initialize a helper.
        """
        if not isinstance(registry, Registry):
            raise TypeError(f"{registry} is not a context registry")
        elif not isinstance(prefix, str):
            raise TypeError(f"Key prefix must be a string")

        self._registry = registry
        self._prefix = prefix

    @property
    def registry(self):
        """Get the context registry.
        """
        return self._registry

    @property
    def prefix(self):
        """Get the key prefix.
        """
        return self._prefix

    def key(self, key, /):
        """Get context key with the prefix.
        """
        return Registry.key(key, prefix=self.prefix)

    def interface(self, context_cls, /):
        """Get a built-in context interface from the context class.
        """
        if not issubclass(context_cls, Context):
            raise TypeError

        return context_cls.interface_in(self.registry.BUILTIN.executable)

    def _check_type(self, arg, type, /):
        """Check if the argument type is the specified one.
        """
        if arg is not None and not isinstance(arg, type):
            raise TypeError(f"Argument {arg} is not of type {type}")

    def _check_attr(self, arg, attr, /):
        """Check compatibility of the argument with the sample attributes.
        """
        if arg is not None and not TypeAttr.compatible(TypeAttr.of(arg), attr):
            raise TypeError(f"Argument {arg} is not compatible with {attr}")

### archi/aggr ###

class AggregateHelper(Helper):
    """Helper functions for archi/aggr module.
    """
    def new_object(self, key, /, interface=None, metadata=None, fam_length=None):
        """Create an aggregate object.

        Generic aggregate object interface is used by default.
        """
        if interface is None:
            if metadata is None:
                raise ValueError('No aggregate type description is specified')
            elif isinstance(metadata, str):
                metadata = AggregateTypeSymbol.slot(metadata, self.registry.BUILTIN.executable)

            interface = AggregateInterfaceSymbol.slot('generic', self.registry.BUILTIN.executable)

        params = {}
        if fam_length is not None:
            params['fam_length'] = fam_length

        I_AGGREGATE = self.interface(AggregateContext)
        return self.registry.new_context(self.key(key), I_AGGREGATE(
            interface=interface, metadata=metadata, **params))

### archi/exec ###

class DexgraphHelper(Helper):
    """Helper functions for archi/exec module.
    """
    def new_operation_seq(self, key, /, name=None, operations=None):
        """Create a directed execution graph operation sequence.
        """
        self._check_type(name, str)

        if operations is not None:
            for operation in operations:
                if operation is not None:
                    operation_func, operation_data = None, None

                    if isinstance(operation, tuple):
                        if len(operation) > 2:
                            raise ValueError
                        elif len(operation) == 2:
                            operation_func, operation_data = operation
                        elif len(operation) == 1:
                            operation_data = operation[0]
                    else:
                        operation_data = operation

                    if operation_func is not None:
                        self._check_attr(operation_func, TypeAttr.function(
                            typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION))
                        self._check_attr(operation_data, TypeAttr.complex_data())
                    else:
                        self._check_attr(operation_data, TypeAttr.complex_data(
                            typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ))

        params = {}
        if name is not None:
            params['name'] = name
        if operations is not None:
            params['length'] = PrimitiveData(c.c_size_t(len(operations)))

        I_DEXGRAPH_OPERATION_SEQ = self.interface(DexgraphOperationSeqContext)
        sequence = self.registry.new_context(self.key(key), I_DEXGRAPH_OPERATION_SEQ(**params))

        if operations is not None:
            for i, operation in enumerate(operations):
                if operation is not None:
                    if isinstance(operation, tuple):
                        if len(operation) == 1:
                            if operation[0] is not None:
                                self.registry(sequence.data[i] << operation[0])
                        elif len(operation) == 2:
                            if operation[0] is not None:
                                self.registry(sequence.function[i] << operation[0])
                            if operation[1] is not None:
                                self.registry(sequence.data[i] << operation[1])
                    else:
                        self.registry(sequence.data[i] << operation)

        return sequence

    def new_node(self, key, /, name=None, operation=None, transition=None):
        """Create a directed execution graph node.
        """
        self._check_type(name, str)

        if operation is not None:
            operation_func, operation_data = None, None

            if isinstance(operation, tuple):
                if len(operation) > 2:
                    raise ValueError
                elif len(operation) == 2:
                    operation_func, operation_data = operation
                elif len(operation) == 1:
                    operation_data = operation[0]
            else:
                operation_data = operation

            if operation_func is not None:
                self._check_attr(operation_func, TypeAttr.function(
                    typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION))
                self._check_attr(operation_data, TypeAttr.complex_data())
            else:
                self._check_attr(operation_data, TypeAttr.complex_data(
                    typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ))

        if transition is not None:
            transition_func, transition_data = None, None

            if isinstance(transition, tuple):
                if len(transition) > 2:
                    raise ValueError
                elif len(transition) == 2:
                    transition_func, transition_data = transition
                elif len(transition) == 1:
                    transition_data = transition[0]
            else:
                transition_data = transition

            if transition_func is not None:
                self._check_attr(transition_func, TypeAttr.function(
                    typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION))
                self._check_attr(transition_data, TypeAttr.complex_data())
            else:
                self._check_attr(transition_data, TypeAttr.complex_data(
                    typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE))

        params = {}
        if name is not None:
            params['name'] = name
        if operation is not None:
            if isinstance(operation, tuple):
                if len(operation) == 1:
                    if operation[0] is not None:
                        params['operation_data'] = operation[0]
                elif len(operation) == 2:
                    if operation[0] is not None:
                        params['operation_func'] = operation[0]
                    if operation[1] is not None:
                        params['operation_data'] = operation[1]
            else:
                params['operation_data'] = operation
        if transition is not None:
            if isinstance(transition, tuple):
                if len(transition) == 1:
                    if transition[0] is not None:
                        params['transition_data'] = transition[0]
                elif len(transition) == 2:
                    if transition[0] is not None:
                        params['transition_func'] = transition[0]
                    if transition[1] is not None:
                        params['transition_data'] = transition[1]
            else:
                params['transition_data'] = transition

        I_DEXGRAPH_NODE = self.interface(DexgraphNodeContext)
        node = self.registry.new_context(self.key(key), I_DEXGRAPH_NODE(**params))

        return node

    def new_branch_transition_data(self, key, /,
                                   condition_func=None, condition_data=None,
                                   target_false=None, target_true=None):
        """Create transition data for the branching function.
        """
        self._check_attr(condition_func, TypeAttr.function(
            typ.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_CONDITION))
        self._check_attr(condition_data, TypeAttr.complex_data())
        self._check_attr(target_false, TypeAttr.complex_data(
            typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE))
        self._check_attr(target_true, TypeAttr.complex_data(
            typ.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE))

        branch_data = AggregateHelper(self.registry, prefix=self.prefix).new_object(
                key, metadata=DexgraphTransitionDataSymbol.full_name('branch'))

        if condition_func is not None:
            self.registry(branch_data.member.function << condition_func)
        if condition_data is not None:
            self.registry(branch_data.member.data << condition_data)
        if target_false is not None:
            self.registry(branch_data.member.target[0] << target_false)
        if target_true is not None:
            self.registry(branch_data.member.target[1] << target_true)

        return branch_data

    def new_loop_times_condition_data(self, key, /, num_iterations=None, iteration=None):
        """Create condition data for the loop-times function.
        """
        if isinstance(num_iterations, int):
            if num_iterations < 0:
                raise ValueError

            num_iterations = PrimitiveData(c.c_size_t(num_iterations))
        else:
            self._check_attr(num_iterations, TypeAttr.from_type(c.c_size_t))

        if isinstance(iteration, int):
            if iteration < 0:
                raise ValueError

            iteration = PrimitiveData(c.c_size_t(iteration))
        else:
            self._check_attr(iteration, TypeAttr.from_type(c.c_size_t))

        loop_data = AggregateHelper(self.registry, prefix=self.prefix).new_object(
                key, metadata=DexgraphConditionDataSymbol.full_name('loop_times'))

        if num_iterations is not None:
            self.registry(loop_data.member.num_iterations << num_iterations)
        if iteration is not None:
            self.registry(loop_data.member.iteration << iteration)

        return loop_data

### archi/thread ###

class ThreadHelper(Helper):
    """Helper functions for archi/thread module.
    """
    def new_dispatch_data(self, key, /, thread_group=None,
                          work_func=None, work_data=None,
                          callback_func=None, callback_data=None,
                          work_offset=None, work_size=None, batch_size=None):
        """Create thread group dispatching function data.
        """
        self._check_attr(thread_group, TypeAttr.complex_data(
            typ.ARCHI_POINTER_DATA_TAG__THREAD_GROUP))
        self._check_attr(work_func, TypeAttr.function(
            typ.ARCHI_POINTER_FUNC_TAG__THREAD_WORK))
        self._check_attr(work_data, TypeAttr.complex_data())
        self._check_attr(callback_func, TypeAttr.function(
            typ.ARCHI_POINTER_FUNC_TAG__THREAD_CALLBACK))
        self._check_attr(callback_data, TypeAttr.complex_data())

        if isinstance(work_offset, int):
            if work_offset < 0:
                raise ValueError

            work_offset = PrimitiveData(c.c_size_t(work_offset))
        else:
            self._check_attr(work_offset, TypeAttr.from_type(c.c_size_t))

        if isinstance(work_size, int):
            if work_size < 0:
                raise ValueError

            work_size = PrimitiveData(c.c_size_t(work_size))
        else:
            self._check_attr(work_size, TypeAttr.from_type(c.c_size_t))

        if isinstance(batch_size, int):
            if batch_size < 0:
                raise ValueError

            batch_size = PrimitiveData(c.c_size_t(batch_size))
        else:
            self._check_attr(batch_size, TypeAttr.from_type(c.c_size_t))

        dispatch_data = AggregateHelper(self.registry, prefix=self.prefix).new_object(
                key, metadata=DexgraphOperationDataSymbol.full_name('thread_group_dispatch'))

        if thread_group is not None:
            self.registry(dispatch_data.member.thread_group << thread_group)
        if work_func is not None:
            self.registry(dispatch_data.member.work.function << work_func)
        if work_data is not None:
            self.registry(dispatch_data.member.work.data << work_data)
        if callback_func is not None:
            self.registry(dispatch_data.member.callback.function << callback_func)
        if callback_data is not None:
            self.registry(dispatch_data.member.callback.data << callback_data)
        if work_offset is not None:
            self.registry(dispatch_data.member.param.offset << work_offset)
        if work_size is not None:
            self.registry(dispatch_data.member.param.size << work_size)
        if batch_size is not None:
            self.registry(dispatch_data.member.param.batch_size << batch_size)

        return dispatch_data

### archi/memory ###

class MemoryHelper(Helper):
    """Helper functions for archi/memory module.
    """
    def heap_interface(self):
        """Obtain the heap memory interface.
        """
        return MemoryInterfaceSymbol.slot('heap', self.registry.BUILTIN.executable)

### archi/file ###

class FileHelper(Helper):
    """Helper functions for archi/file module.
    """
    def new_printf_operation_data(self, key, /, stream=None, format_string=None, *args):
        """Create operation data for the print function.
        """
        self._check_attr(stream, TypeAttr.complex_data(
            typ.ARCHI_POINTER_DATA_TAG__FILE_STREAM))

        if format_string is None:
            raise ValueError("Format string must not be None")
        self._check_type(format_string, str)

        if not all(isinstance(arg, (Object, Context, Context.Slot)) for arg in args):
            raise TypeError

        tokens = []

        flag_chars = ' #0+-'
        regex_specific = { # conversion specifications supported by the operation function
                          'c': re.compile(r'%[ +-]*(?:[1-9]\d*)?l?c'),
                          's': re.compile(r'%[ +-]*(?:[1-9]\d*)?(?:\.\d+)?l?s'),
                          'diu': re.compile(r'%[ 0+-]*(?:[1-9]\d*)?(?:\.\d+)?(?:hh|h|l|ll|j|z|t)?[diu]'),
                          'oxX': re.compile(r'%[ #0+-]*(?:[1-9]\d*)?(?:\.\d+)?(?:hh|h|l|ll|j|z|t)?[oxX]'),
                          'fFeEgGaA': re.compile(r'%[ #0+-]*(?:[1-9]\d*)?(?:\.\d+)?(?:l|L)?[fFeEgGaA]'),
                          'p': re.compile(r'%[ +-]*(?:[1-9]\d*)?p'),
                          'n': re.compile(r'%(?:hh|h|l|ll|j|z|t)?n'),
                          }
        conv_spec_chars = ''.join(regex_specific.keys())
        regex_generic = re.compile(rf'%[^%{conv_spec_chars}]*[%{conv_spec_chars}]')

        processed_len = 0
        arg_i = 0
        while processed_len < len(format_string):
            # Find the next conversion specification
            index = format_string.find('%', processed_len)

            if index < 0: # not found
                # Process the rest of the format string
                tokens.append((format_string[processed_len:], None))
                break
            elif index > processed_len:
                # Process the string preceding the conversion specification
                tokens.append((format_string[processed_len:index], None))
                processed_len = index

            # Match the conversion specification
            match = re.match(regex_generic, format_string[processed_len:])
            if match is None:
                raise ValueError(f"Conversion specification starting at index {processed_len} is incorrect")

            # Validate the conversion specification and set the argument
            conv_spec = format_string[processed_len:processed_len+match.end()]

            if conv_spec[-1] == '%':
                if conv_spec != '%%':
                    raise ValueError(f"Conversion specification '{conv_spec}' is incorrect")

                arg = None
            else:
                for specifier, regex in regex_specific.items():
                    if conv_spec[-1] in specifier:
                        if re.match(regex, conv_spec) is None:
                            raise ValueError(f"Conversion specification '{conv_spec}' is incorrect")
                        break
                else: # should never happen
                    raise RuntimeError

                if any(conv_spec.count(flag_char) > 1 for flag_char in flag_chars):
                    raise ValueError(f"Conversion specification '{conv_spec}' is incorrect")

                arg = args[arg_i]
                arg_i += 1

            # Process the token
            tokens.append((conv_spec, arg))
            processed_len += len(conv_spec)

        print_data = AggregateHelper(self.registry, prefix=self.prefix).new_object(
                key, metadata=DexgraphOperationDataSymbol.full_name('file_printf'),
                fam_length=len(tokens))

        if stream is not None:
            self.registry(print_data.member.stream << stream)
        self.registry(print_data.member.num_tokens << PrimitiveData(c.c_size_t(len(tokens))))

        for i, token in enumerate(tokens):
            self.registry(print_data.member.token[i].conv_spec << String(token[0]))
            if token[1] is not None:
                self.registry(print_data.member.token[i].argument << token[1])

        return print_data

