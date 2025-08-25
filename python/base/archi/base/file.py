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

from .memory import CValue, MemoryBlock, MemoryBlockCache, MemoryCluster, Encoder


class File:
    """Representation of an Archipelago input file.
    """
    def __init__(self):
        """Initialize a file.
        """
        self._contents = {}

    def __getitem__(self, key: 'str'):
        """Get element of the file contents dictionary.
        """
        if key is None:
            return None
        elif not isinstance(key, str):
            raise TypeError

        return self._contents[key]

    def __setitem__(self, key: 'str', encoder):
        """Set element of the file contents dictionary.
        """
        if not isinstance(key, str):
            raise TypeError
        elif not isinstance(encoder, Encoder):
            raise TypeError

        self._contents[key] = encoder

    def __delitem__(self, key: 'str'):
        """Unset element of the file contents dictionary.
        """
        if key is None:
            return
        elif not isinstance(key, str):
            raise TypeError

        del self._contents[key]

    def __contains__(self, key: 'str') -> 'bool':
        """Check if a key is in the file contents dictionary.
        """
        if not isinstance(key, str):
            raise TypeError

        return key in self._contents

    def __iter__(self):
        """Return an iterator.
        """
        return iter(self._contents)

    def contents(self) -> 'dict[str]':
        """Obtain the dictionary of file contents.
        """
        from types import MappingProxyType

        return MappingProxyType(self._contents)

    def reset(self):
        """Reset the internal state.
        """
        self._contents = {}

    def encode(self) -> 'MemoryCluster':
        """Encode a file.
        """
        from .memory import StringEncoder
        from .ctypes.base import archi_array_layout_t, archi_pointer_t, archi_parameter_list_t
        from .ctypes.file import archi_exe_input_file_header_t

        cache = MemoryBlockCache()

        block_nodes = [None] * len(self._contents)

        for idx, (key, value) in enumerate(self._contents.items()):
            block_key = StringEncoder.encode(key, cache)
            block_value = value(cache)

            node = archi_parameter_list_t()

            if block_value is not None:
                cvalue = block_value.initializer()
                node.value = archi_pointer_t(archi_array_layout_t(cvalue.num_elements(),
                                                                  cvalue.element_size(),
                                                                  cvalue.element_alignment()),
                                             cvalue.attributes().get('flags', 0))

            def callback_node(node: 'archi_parameter_list_t',
                              num_nodes=len(self._contents), idx=idx,
                              block_key=block_key, block_value=block_value):
                if idx < num_nodes - 1:
                    node.next = c.cast(block_nodes[idx + 1].address(), type(node.next))

                node.name = block_key.address()

                if block_value is not None:
                    node.value.ptr = block_value.address()

            block_node = MemoryBlock(CValue(node), callback=callback_node)
            cache[block_node] = block_node

            block_nodes[idx] = block_node

        block_header = None
        memory = None

        def callback_header(header):
            header.header.addr = block_header.address()
            header.header.end  = block_header.address() + memory.size()

            if block_nodes:
                header.contents = c.cast(block_nodes[0].address(), type(header.contents))

        block_header = MemoryBlock(CValue(archi_exe_input_file_header_t()), callback=callback_header)

        memory = MemoryCluster(block_header, cache.blocks())
        memory.pack()

        return memory

