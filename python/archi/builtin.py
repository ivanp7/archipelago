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
# @brief Context types built in the Archipelago executable.

import ctypes as c

import archi.ctypes as actype
from .object import PrimitiveData, String
from .context import TypeAttributes, ContextBase, ContextWhitelist, ParametersWhitelist, \
        PointerContext, DataPointerArrayContext


_TYPE_DATA = TypeAttributes.complex_data()
_TYPE_FUNCTION = TypeAttributes.function()
_TYPE_BOOL = (TypeAttributes.from_type(c.c_char), lambda value: PrimitiveData(c.c_char(bool(value))))
_TYPE_INT = (TypeAttributes.from_type(c.c_int), lambda value: PrimitiveData(c.c_int(value)))
_TYPE_LONG = (TypeAttributes.from_type(c.c_long), lambda value: PrimitiveData(c.c_long(value)))
_TYPE_LONGLONG = (TypeAttributes.from_type(c.c_longlong), lambda value: PrimitiveData(c.c_longlong(value)))
_TYPE_SIZE = (TypeAttributes.from_type(c.c_size_t), lambda value: PrimitiveData(c.c_size_t(value)))
_TYPE_ATTR = (TypeAttributes.from_type(TypeAttributes), lambda value: PrimitiveData(TypeAttributes(value)))
_TYPE_DATA_PTR = TypeAttributes.from_type(c.c_void_p)
_TYPE_STRING = (TypeAttributes.from_type(c.c_char * 1), lambda value: String(value))

##############################################################################

### archi/context ###

class DataPointerContext(ContextWhitelist):
    """Pointer to data.
    """
    C_NAME = 'dpointer'

    CONTEXT_TYPE = _TYPE_DATA

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': _TYPE_DATA,
                  'writable': _TYPE_BOOL}

    GETTER_SLOTS = InitParameters.PARAMS
    SETTER_SLOTS = GETTER_SLOTS


class PrimitiveDataPointerContext(ContextWhitelist):
    """Pointer to primitive data.
    """
    C_NAME = 'pdpointer'

    CONTEXT_TYPE = _TYPE_DATA

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': _TYPE_DATA,
                  'offset': _TYPE_LONGLONG,
                  'offset_unit': _TYPE_SIZE,
                  'writable': _TYPE_BOOL,
                  'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE}

    class ShiftPtrCallParameters(ParametersWhitelist):
        PARAMS = {'offset': _TYPE_LONGLONG}

    class SetAttrCallParameters(ParametersWhitelist):
        PARAMS = {'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE}

    class CopyCallParameters(ParametersWhitelist):
        PARAMS = {'src': _TYPE_DATA,
                  'src_offset': _TYPE_SIZE,
                  'offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    class FillCallParameters(ParametersWhitelist):
        PARAMS = {'pattern': _TYPE_DATA,
                  'offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    GETTER_SLOTS = {'pointee': _TYPE_DATA,
                    '': {1: _TYPE_DATA},
                    'writable': _TYPE_BOOL,
                    'length': _TYPE_SIZE,
                    'stride': _TYPE_SIZE,
                    'size': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE}

    CALL_SLOTS = {'shift_ptr': (None, ShiftPtrCallParameters),
                  'set_attr': (None, SetAttrCallParameters),
                  'copy': (None, CopyCallParameters),
                  'fill': (None, FillCallParameters)}

    SETTER_SLOTS = {'pointee': _TYPE_DATA,
                    'writable': _TYPE_BOOL,
                    'length': _TYPE_SIZE,
                    'stride': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE}


class ComplexDataPointerContext(ContextWhitelist):
    """Pointer to complex data.
    """
    C_NAME = 'cdpointer'

    CONTEXT_TYPE = _TYPE_DATA

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': _TYPE_DATA,
                  'offset': _TYPE_LONGLONG,
                  'offset_unit': _TYPE_SIZE,
                  'writable': _TYPE_BOOL,
                  'tag': _TYPE_ATTR}

    GETTER_SLOTS = {'pointee': _TYPE_DATA,
                    'writable': _TYPE_BOOL,
                    'tag': _TYPE_ATTR}

    SETTER_SLOTS = {'pointee': _TYPE_DATA,
                    'writable': _TYPE_BOOL,
                    'tag': _TYPE_ATTR}


class FunctionPointerContext(ContextWhitelist):
    """Pointer to function.
    """
    C_NAME = 'fpointer'

    CONTEXT_TYPE = _TYPE_FUNCTION

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pointee': _TYPE_FUNCTION,
                  'tag': _TYPE_ATTR}

    GETTER_SLOTS = {'pointee': _TYPE_FUNCTION,
                    'tag': _TYPE_ATTR}

    SETTER_SLOTS = {'pointee': _TYPE_FUNCTION,
                    'tag': _TYPE_ATTR}

### archi/aggr ###

class AggregateContext(ContextWhitelist):
    """Aggregate object.
    """
    C_NAME = 'aggregate'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__AGGR)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'interface': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE),
                  'metadata': _TYPE_DATA,
                  'fam_length': _TYPE_SIZE}

    GETTER_SLOTS = {'interface': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE),
                    'metadata': _TYPE_DATA,
                    'layout': TypeAttributes.from_type(actype.archi_layout_struct_t),
                    'size': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE,
                    'fam_stride': _TYPE_SIZE,
                    'fam_length': _TYPE_SIZE,
                    'full_size': _TYPE_SIZE,
                    'object': _TYPE_DATA}

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if not call:
            if name.startswith('member.'):
                return _TYPE_DATA
            elif name.startswith('ref.') and not setter:
                return ...

        return super()._slot_attr(name, indices, setter, call)

### archi/exec ###

class DexgraphNodeContext(ContextWhitelist):
    """Directed execution graph node.
    """
    C_NAME = 'dexgraph_node'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'name': _TYPE_STRING,
                  'sequence_length': _TYPE_SIZE,
                  'transition_func': TypeAttributes.function(actype.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                  'transition_data': _TYPE_DATA,
                  'branches': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}

    class ExecuteCallParameters(ParametersWhitelist):
        PARAMS = {'index': (TypeAttributes.from_type(actype.archi_dexgraph_branch_index_t),
                            lambda value: PrimitiveData(actype.archi_dexgraph_branch_index_t(value)))}

    GETTER_SLOTS = {'name': _TYPE_STRING,
                    'sequence.length': _TYPE_SIZE,
                    'sequence.function': {1: TypeAttributes.function(actype.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)},
                    'sequence.data': {1: _TYPE_DATA},
                    'transition.function': TypeAttributes.function(actype.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                    'transition.data': _TYPE_DATA,
                    'branches': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}

    CALL_SLOTS = {'execute': (None, ExecuteCallParameters)}

    SETTER_SLOTS = {'sequence.function': {1: TypeAttributes.function(actype.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)},
                    'sequence.data': {1: _TYPE_DATA},
                    'transition.function': TypeAttributes.function(actype.ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION),
                    'transition.data': _TYPE_DATA,
                    'branches': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}


class DexgraphNodeArrayContext(ContextWhitelist):
    """Directed execution graph node array.
    """
    C_NAME = 'dexgraph_node_array'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'num_nodes': _TYPE_SIZE}

    GETTER_SLOTS = {'num_nodes': _TYPE_SIZE,
                    'node': {1: TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)}}

    SETTER_SLOTS = {'node': {1: TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)}}

### archi/thread ###

class ThreadGroupContext(ContextWhitelist):
    """Thread group.
    """
    C_NAME = 'thread_group'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__THREAD_GROUP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttributes.from_type(actype.archi_thread_group_start_params_t),
                             lambda value: PrimitiveData(value)),
                  'num_threads': _TYPE_SIZE}

    GETTER_SLOTS = {'num_threads': _TYPE_SIZE}


class LockFreeQueueContext(ContextWhitelist):
    """Lock-free queue.
    """
    C_NAME = 'thread_lfqueue'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__THREAD_LFQUEUE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttributes.from_type(actype.archi_thread_lfqueue_alloc_params_t),
                             lambda value: PrimitiveData(value)),
                  'capacity': _TYPE_SIZE,
                  'elt_size': _TYPE_SIZE}

    GETTER_SLOTS = {'capacity': _TYPE_SIZE,
                    'elt_size': _TYPE_SIZE}

### archi/signal ###

class SignalHandlerDataHashmapContext(ContextBase):
    """Data for the signal meta-handler for multiple handler support.
    """
    C_NAME = 'signal_handler_data__hashmap'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__SIGNAL_HANDLER_DATA__HASHMAP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': (TypeAttributes.from_type(actype.archi_hashmap_alloc_params_t),
                             lambda value: PrimitiveData(value)),
                  'capacity': _TYPE_SIZE}

    def _slot_attr(cls, name, indices, setter, call):
        if call:
            raise KeyError
        elif indices:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE

        return TypeAttributes.from_type(actype.archi_signal_handler_t)

    @classmethod
    def _slot_unsettable(cls, name, indices):
        return True

### archi/memory ###

class MemoryContext(ContextWhitelist):
    """Memory handle.
    """
    C_NAME = 'memory'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__MEMORY)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'interface': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE),
                  'alloc_data': _TYPE_DATA,
                  'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE,
                  'ext_alignment': _TYPE_SIZE}

    GETTER_SLOTS = {'interface': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE),
                    'allocation': _TYPE_DATA,
                    'length': _TYPE_SIZE,
                    'stride': _TYPE_SIZE,
                    'size': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE,
                    'ext_alignment': _TYPE_SIZE}


class MemoryMappingContext(ContextWhitelist):
    """Memory mapping.
    """
    C_NAME = 'memory_mapping'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__MEMORY_MAPPING)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'memory': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__MEMORY),
                  'map_data': _TYPE_DATA,
                  'offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    GETTER_SLOTS = {'memory': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__MEMORY),
                    'ptr': _TYPE_DATA,
                    'offset': _TYPE_SIZE,
                    'length': _TYPE_SIZE,
                    'size': _TYPE_SIZE}

### archi/file ###

class FileContext(ContextWhitelist):
    """File handle.
    """
    C_NAME = 'file'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__FILE_STREAM)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pathname': _TYPE_STRING,
                  'fd': (TypeAttributes.from_type(actype.archi_file_descriptor_t),
                         lambda value: PrimitiveData(actype.archi_file_descriptor_t(value))),
                  'params': TypeAttributes.from_type(actype.archi_file_open_params_t),
                  'size': _TYPE_SIZE,
                  'readable': _TYPE_BOOL,
                  'writable': _TYPE_BOOL,
                  'create': _TYPE_BOOL,
                  'exclusive': _TYPE_BOOL,
                  'truncate': _TYPE_BOOL,
                  'append': _TYPE_BOOL,
                  'flags': _TYPE_INT,
                  'mode': _TYPE_INT,
                  'stream': _TYPE_BOOL}

    class ReadCallParameters(ParametersWhitelist):
        PARAMS = {'dest': _TYPE_DATA,
                  'dest_offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    class WriteCallParameters(ParametersWhitelist):
        PARAMS = {'src': _TYPE_DATA,
                  'src_offset': _TYPE_SIZE,
                  'length': _TYPE_SIZE}

    class SyncCallParameters(ParametersWhitelist):
        PARAMS = {}

    GETTER_SLOTS = {'fd': (TypeAttributes.from_type(actype.archi_file_descriptor_t),
                           lambda value: PrimitiveData(actype.archi_file_descriptor_t(value))),
                    'offset': _TYPE_LONGLONG}

    CALL_SLOTS = {'read': (None, ReadCallParameters),
                  'write': (None, WriteCallParameters),
                  'sync': (None, SyncCallParameters)}

    SETTER_SLOTS = {'offset': _TYPE_LONGLONG,
                    'offset.end': _TYPE_LONGLONG,
                    'offset.shift': _TYPE_LONGLONG}


class FileMappingContext(ContextWhitelist):
    """File mapping.
    """
    C_NAME = 'file_mapping'

    CONTEXT_TYPE = TypeAttributes.complex_data()

    class InitParameters(ParametersWhitelist):
        PARAMS = {'fd': (TypeAttributes.from_type(actype.archi_file_descriptor_t),
                         lambda value: PrimitiveData(actype.archi_file_descriptor_t(value))),
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE,
                  'params': TypeAttributes.from_type(actype.archi_file_map_params_t),
                  'size': _TYPE_SIZE,
                  'offset': _TYPE_SIZE,
                  'ptr_support': _TYPE_BOOL,
                  'readable': _TYPE_BOOL,
                  'writable': _TYPE_BOOL,
                  'shared': _TYPE_BOOL,
                  'flags': _TYPE_INT}

    GETTER_SLOTS = {'length': _TYPE_SIZE,
                    'stride': _TYPE_SIZE,
                    'size': _TYPE_SIZE,
                    'alignment': _TYPE_SIZE}

### archi/library ###

class LibraryContext(ContextBase):
    """Shared library.
    """
    C_NAME = 'library'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__LIBRARY_HANDLE)

    SYMBOLS = None

    class InitParameters(ParametersWhitelist):
        PARAMS = {'pathname': _TYPE_STRING,
                  'params': TypeAttributes.from_type(actype.archi_library_load_params_t),
                  'lazy': _TYPE_BOOL,
                  'global': _TYPE_BOOL,
                  'flags': _TYPE_INT}

    class CallParameters(ParametersWhitelist):
        PARAMS = {'function': _TYPE_BOOL,
                  'tag': _TYPE_ATTR,
                  'length': _TYPE_SIZE,
                  'stride': _TYPE_SIZE,
                  'alignment': _TYPE_SIZE}

    def __init_subclass(cls):
        """Initialize a subclass.
        """
        if not isinstance(cls.SYMBOLS, (type(None), dict)):
            raise TypeError

        if cls.SYMBOLS is not None:
            for key, value in cls.SYMBOLS.items():
                if not isinstance(key, str):
                    raise TypeError
                elif not isinstance(value, (type(...), TypeAttributes)):
                    raise TypeError

    @classmethod
    def _call_params_class(cls, name, indices):
        if indices:
            raise KeyError
        elif cls.SYMBOLS is not None and name and name not in cls.SYMBOLS:
            raise KeyError

        return cls.CallParameters

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if indices or setter:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE if not call else None

        return cls.SYMBOLS[name] if cls.SYMBOLS is not None else ...

### archi/hashmap ###

class HashmapContext(ContextBase):
    """Hashmap.
    """
    C_NAME = 'hashmap'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__HASHMAP)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'params': TypeAttributes.from_type(actype.archi_hashmap_alloc_params_t),
                  'capacity': _TYPE_SIZE}

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if indices or call:
            raise KeyError

        if not name:
            return cls.CONTEXT_TYPE

        return ...

    @classmethod
    def _slot_unsettable(cls, name, indices):
        return True

### archi/env ###

class EnvVariableContext(ContextWhitelist):
    """Environment variable.
    """
    C_NAME = 'env_variable'

    CONTEXT_TYPE = _TYPE_STRING

    class InitParameters(ParametersWhitelist):
        PARAMS = {'default_value': _TYPE_STRING}

    SETTER_SLOTS = {'default_value': _TYPE_STRING}

    @classmethod
    def _slot_attr(cls, name, indices, setter, call):
        if not setter and not call:
            if indices:
                raise KeyError

            return _TYPE_STRING[0]
        else:
            return super()._slot_attr(name, indices, setter, call)

### archi/parser ###

class NumberParserContext(ContextWhitelist):
    """Number parser.
    """
    C_NAME = 'number_parser'

    CONTEXT_TYPE = _TYPE_DATA

    class InitParameters(ParametersWhitelist):
        PARAMS = {'base': _TYPE_INT,
                  'unsigned_char': _TYPE_STRING,
                  'unsigned_short': _TYPE_STRING,
                  'unsigned_int': _TYPE_STRING,
                  'unsigned_long': _TYPE_STRING,
                  'unsigned_long_long': _TYPE_STRING,
                  'signed_char': _TYPE_STRING,
                  'signed_short': _TYPE_STRING,
                  'signed_int': _TYPE_STRING,
                  'signed_long': _TYPE_STRING,
                  'signed_long_long': _TYPE_STRING,
                  'float': _TYPE_STRING,
                  'double': _TYPE_STRING,
                  'long_double': _TYPE_STRING,
                  'size_t': _TYPE_STRING,
                  'uint8_t': _TYPE_STRING,
                  'uint16_t': _TYPE_STRING,
                  'uint32_t': _TYPE_STRING,
                  'uint64_t': _TYPE_STRING,
                  'int8_t': _TYPE_STRING,
                  'int16_t': _TYPE_STRING,
                  'int32_t': _TYPE_STRING,
                  'int64_t': _TYPE_STRING}

### archi/timer ###

class TimerContext(ContextWhitelist):
    """Timer.
    """
    C_NAME = 'timer'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__TIMER)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'name': _TYPE_STRING}

    class ResetCallParameters(ParametersWhitelist):
        PARAMS = {}

    CALL_SLOTS = {'reset': (None, ResetCallParameters)}

