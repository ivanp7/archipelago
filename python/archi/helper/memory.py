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
# @brief Helpers for memory contexts.

import ctypes as c

from archi.context import (
        Registry,
        MemoryContext,
        MemoryMappingContext,
        MemoryInterfaceSymbol,
        )
from archi.helper.pointer import primitive_data_pointer


def heap_memory_interface(plugin):
    """Obtain memory interface for heap memory.
    """
    return MemoryInterfaceSymbol.slot(plugin, 'heap')


def allocate_memory(registry, c_type, interface, /, ext_alignment=None,
                    alloc_data=None, map_data=None, contents=None, contents_file=None, key=None):
    """Create a memory context.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(key, (type(None), str)):
        raise TypeError
    elif not isinstance(ext_alignment, (type(None), int)):
        raise TypeError
    elif ext_alignment is not None and ext_alignment <= 0:
        raise ValueError
    elif contents is not None and contents_file is not None:
        raise ValueError

    if isinstance(c_type, c.Array):
        length = len(c_type)
        stride = c.sizeof(c_type._type_)
        alignment = c.alignment(c_type._type_)
    else:
        length = 1
        stride = c.sizeof(c_type)
        alignment = c.alignment(c_type)

    executable = registry[Registry.KEY_EXECUTABLE]

    I_MEMORY = MemoryContext.interface(library=executable)

    params = {}
    if alloc_data is not None:
        params['alloc_data'] = alloc_data
    if ext_alignment is not None:
        params['ext_alignment'] = ext_alignment

    memory = registry.new_context(I_MEMORY(interface=interface, length=length,
                                           stride=stride, alignment=alignment,
                                           **params),
                                  key=registry.key(key, tmp_prefix='memory'))

    if contents is not None or contents_file is not None:
        # Initialize the memory
        with registry.deleted_context(map_memory(registry, memory, map_data=map_data)) as memory_mapping:
            if contents is not None:
                with registry.deleted_context(primitive_data_pointer(registry,
                                                                     memory_mapping.ptr,
                                                                     writable=True)) as mapping_ptr_context:
                    registry(mapping_ptr_context.copy(src=contents))
            elif contents_file is not None:
                registry(contents_file.read(dest=memory_mapping.ptr))

    return memory


def map_memory(registry, memory, /, map_data=None, offset=None, length=None, key=None):
    """Create a memory mapping context.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(memory, MemoryContext):
        raise TypeError
    elif not isinstance(offset, (type(None), int)):
        raise TypeError
    elif offset is not None and offset < 0:
        raise ValueError
    elif not isinstance(length, (type(None), int)):
        raise TypeError
    elif length is not None and length < 0:
        raise ValueError

    executable = registry[Registry.KEY_EXECUTABLE]

    I_MEMORY_MAPPING = MemoryMappingContext.interface(library=executable)

    params = {}
    if map_data is not None:
        params['map_data'] = map_data
    if offset is not None:
        params['offset'] = offset
    if length is not None:
        params['length'] = length

    return registry.new_context(I_MEMORY_MAPPING(memory=memory, **params),
                                key=registry.key(key, tmp_prefix='memory_mapping'))

