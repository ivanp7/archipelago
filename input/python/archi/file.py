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

    def __getitem__(self, key: "str"):
        """Get element of the file contents dictionary.
        """
        if not isinstance(key, str):
            raise TypeError

        return self._contents.get(key)

    def __setitem__(self, key: "str", value):
        """Set element of the file contents dictionary.
        """
        if not isinstance(key, str):
            raise TypeError

        self._contents[key] = value

    def __delitem__(self, key: "str"):
        """Unset element of the file contents dictionary.
        """
        if not isinstance(key, str):
            raise TypeError

        del self._contents[key]

    def contents(self) -> "dict[str]":
        """Get the dictionary of file contents.
        """
        return self._contents.copy()

    def blocks(self) -> "MemoryBlockSet":
        """Get the additional memory block set of the file.
        """
        return self._blocks

    def set_blocks(self, blocks: "MemoryBlockSet"):
        """Set the additional memory block set of the file.
        """
        if not isinstance(blocks, MemoryBlockSet):
            raise TypeError

        self._blocks = blocks

    def reset(self, contents: "bool" = True, blocks: "bool" = True):
        """Reset the internal state.
        """
        if contents:
            self._contents = {}
        if blocks:
            self._blocks = MemoryBlockSet()

    def memory(self) -> "Memory":
        """Create a memory representation object of the file.
        """
        import copy
        from .aux.marshalling import MARSHALLER
        from .ctypes.common import archi_pointer_t, archi_parameter_list_t, \
                archi_exe_input_file_header_t

        # Marshal the file contents
        blocks = copy.copy(self._blocks)
        contents = {}
        value_dict = {}

        for key, value in self._contents.items():
            if value is None:
                contents[key] = None
            else:
                try:
                    contents[key] = blocks.add(value)
                except TypeError:
                    if type(value) not in MARSHALLER:
                        raise TypeError("Unsupported content type")

                    marshaller = MARSHALLER[type(value)](blocks, value_dict)
                    contents[key] = marshaller.marshal(value)

        # Marshal the list of pointers to file contents
        block_contents = [None] * len(contents)

        for idx, (key, block_value) in enumerate(contents.items()):
            # Set non-pointer fields of the contents list node
            node = archi_parameter_list_t()

            if block_value is not None:
                value = block_value.value()

                flags = value.attributes().get('flags', 0)
                if not isinstance(flags, int):
                    raise TypeError
                elif (flags < 0) or (flags >= 1 << archi_pointer_t.NUM_FLAG_BITS):
                    raise ValueError(f"Pointer flags must fit into {archi_pointer_t.NUM_FLAG_BITS} lowest bits")

                node.value.flags = flags
                node.value.element.num_of = value.num_elements()
                node.value.element.size = value.element_size()
                node.value.element.alignment = value.element_alignment()

            # Prepare the node name block
            if key in value_dict:
                block_key = blocks.get(value_dict[key])
            else:
                value_key = CValue(key)
                value_dict[key] = value_key
                block_key = blocks.add(value_key)

            # Define callback for setting pointer fields of the contents list node
            def callback_node(node: "archi_parameter_list_t",
                              idx=idx, block_key=block_key, block_value=block_value):
                if idx < len(contents) - 1:
                    node.next = c.cast(block_contents[idx + 1].address(), type(node.next))

                node.name = block_key.address()
                if block_value is not None:
                    node.value.ptr = block_value.address()

            # Create the contents list node block
            block_node = MemoryBlock(CValue(node, callback=callback_node))
            block_contents[idx] = block_node

            blocks.add(block_node)

        # Set non-pointer fields of the file header
        header = archi_exe_input_file_header_t()

        # Define callback for setting pointer fields of the file header
        block_header = None
        memory = None

        def callback_header(header):
            header.header.addr = block_header.address()
            header.header.end  = block_header.address() + memory.size()

            if block_contents:
                header.contents = c.cast(block_contents[0].address(), type(header.contents))

        # Create the file header block
        block_header = MemoryBlock(CValue(header, callback=callback_header))

        # Create the memory object
        memory = Memory(block_header, blocks.blocks())

        return memory

