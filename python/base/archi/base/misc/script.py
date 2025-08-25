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
# @brief Utilities for file generation scripts.


def eprint(*args, **kwargs):
    """Print to standard error.
    """
    import sys

    print(*args, file=sys.stderr, **kwargs)


def random_map_address() -> 'int':
    """Generate random map address.
    """
    import random

    return 0x7f0000000000 + (random.randrange(0, 0x10000000000) & 0xfffffff000)


def dump_file(file_object: 'File', mapaddr: 'int', pathname: 'str' = None, report: 'bool' = True):
    """Dump File contents into a file or standard output.
    """
    import sys

    from ..memory import MemoryBlockCache, MemoryCluster
    from ..file import File

    if not isinstance(file_object, File):
        raise TypeError

    file_memory = file_object.encode()
    file_buffer = file_memory.marshal(mapaddr)

    # Write the file
    if pathname is not None:
        with open(pathname, mode='wb') as file:
            file.write(file_buffer)
    else:
        sys.stdout.buffer.write(file_buffer)
        sys.stdout.flush()

    if report:
        # Report the sizes
        file_size_b = file_memory.size()
        file_size_kib = file_size_b / 1024
        file_size_mib = file_size_kib / 1024
        file_size_gib = file_size_mib / 1024

        file_padding_b = file_memory.padding()
        file_padding_kib = file_padding_b / 1024
        file_padding_mib = file_padding_kib / 1024

        eprint(f"Wrote {file_size_b} bytes ({file_size_kib:.1f} KiB, {file_size_mib:.1f} MiB, \
{file_size_gib:.1f} GiB) to {f"'{pathname}'" if pathname is not None else "<stdout>"},")
        eprint(f"including {file_padding_b} padding bytes ({file_padding_kib:.1f} KiB, \
{file_padding_mib:.1f} MiB)")
        eprint()

