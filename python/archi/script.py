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
# @brief Utilities for scripts.

import sys

from .object import KeyValueList, AppInputFileHeader, AppInputFile


def errprint(*args, **kwargs):
    """Print to standard error.
    """
    print(*args, file=sys.stderr, **kwargs)


def write_input_file(contents, /, pathname=None, mapaddr=None, print_report=False):
    """Write an Archipelago application input file into a file or standard output.
    """
    # Prepare the bytearray
    contents_kvlist = KeyValueList.construct(contents)

    input_file_header = AppInputFileHeader(contents=contents_kvlist)
    input_file = AppInputFile(input_file_header)

    if mapaddr is None:
        mapaddr = AppInputFile.random_address()

    input_file_buffer = input_file.to_bytearray(mapaddr)

    # Write the file
    if pathname is not None:
        with open(pathname, mode='wb') as file:
            file.write(input_file_buffer)
    else:
        sys.stdout.buffer.write(input_file_buffer)
        sys.stdout.flush()

    # Print the report
    if print_report:
        file_size_b = input_file.total_size
        file_size_kib = file_size_b / 1024
        file_size_mib = file_size_kib / 1024
        file_size_gib = file_size_mib / 1024

        file_padding_b = input_file.total_padding
        file_padding_kib = file_padding_b / 1024
        file_padding_mib = file_padding_kib / 1024

        errprint(f"Wrote {file_size_b} bytes ({file_size_kib:.1f} KiB, {file_size_mib:.1f} MiB, \
{file_size_gib:.1f} GiB) to {f"'{pathname}'" if pathname is not None else '<stdout>'},")
        errprint(f"including {file_padding_b} padding bytes ({file_padding_kib:.1f} KiB, \
{file_padding_mib:.1f} MiB)")
        errprint()

