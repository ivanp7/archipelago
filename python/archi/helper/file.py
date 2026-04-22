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
# @brief Helpers for file contexts.

from contextlib import contextmanager
import ctypes as c

from archi.context import (
        Registry,
        FileContext,
        FileMappingContext,
        )


@contextmanager
def existing_file_mapping(registry, pathname, elt_type, /,
                          readable=True, writable=False, shared=False,
                          open_flags=None, map_flags=None):
    """Context manager for an existing file mapping.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(pathname, str):
        raise TypeError

    executable = registry[Registry.KEY_EXECUTABLE]

    I_FILE = FileContext.interface(library=executable)
    I_FILE_MAPPING = FileMappingContext.interface(library=executable)

    params = {}
    if open_flags is not None:
        params['flags'] = open_flags

    with registry.temp_context(I_FILE(pathname=pathname, readable=readable, writable=writable,
                                      **params),
                               key=registry.temp_key(prefix='file')) as file:
        params = {}
        if map_flags is not None:
            params['flags'] = map_flags

        with registry.temp_context(I_FILE_MAPPING(fd=file.fd, stride=c.sizeof(elt_type),
                                                  alignment=c.alignment(elt_type),
                                                  readable=readable, writable=writable,
                                                  shared=shared, **params),
                                   key=registry.temp_key(prefix='file_mapping')) as file_mapping:
            yield file_mapping


@contextmanager
def new_file_mapping(registry, pathname, elt_type, length, /,
                     mode=0o644, shared=False,
                     open_flags=None, map_flags=None):
    """Context manager for a new file mapping.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(pathname, str):
        raise TypeError

    executable = registry[Registry.KEY_EXECUTABLE]

    I_FILE = FileContext.interface(library=executable)
    I_FILE_MAPPING = FileMappingContext.interface(library=executable)

    params = {}
    if open_flags is not None:
        params['flags'] = open_flags

    with registry.temp_context(I_FILE(pathname=pathname, size=c.sizeof(elt_type) * length,
                                      readable=True, writable=True,
                                      create=True, exclusive=True, truncate=True,
                                      mode=mode, **params),
                               key=registry.temp_key(prefix='file')) as file:
        params = {}
        if map_flags is not None:
            params['flags'] = map_flags

        with registry.temp_context(I_FILE_MAPPING(fd=file.fd, stride=c.sizeof(elt_type),
                                                  alignment=c.alignment(elt_type),
                                                  readable=True, writable=True,
                                                  shared=shared, **params),
                                   key=registry.temp_key(prefix='file_mapping')) as file_mapping:
            yield file_mapping

