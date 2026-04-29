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
# @brief Helpers for pointer contexts.

from archi.context import (
        Registry,
        DataPointerContext,
        PrimitiveDataPointerContext,
        ComplexDataPointerContext,
        FunctionPointerContext,
        )


def data_pointer(registry, pointee, /, writable=False, key=None):
    """Context manager for arbitrary data pointers.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(writable, bool):
        raise TypeError

    executable = registry[Registry.KEY_EXECUTABLE]

    I_DPOINTER = DataPointerContext.interface(library=executable)

    return registry.new_context(I_DPOINTER(pointee=pointee, writable=writable),
                                key=registry.key(key, tmp_prefix='dptr'))


def primitive_data_pointer(registry, pointee, /, writable=False,
                           offset=None, offset_unit=None,
                           length=None, stride=None, alignment=None, key=None):
    """Context manager for primitive data pointers.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(writable, bool):
        raise TypeError
    elif not isinstance(offset, (type(None), int)):
        raise TypeError
    elif not isinstance(offset_unit, (type(None), int)):
        raise TypeError
    elif offset_unit is not None and offset_unit <= 0:
        raise ValueError
    elif not isinstance(length, (type(None), int)):
        raise TypeError
    elif length is not None and length < 0:
        raise ValueError
    elif not isinstance(stride, (type(None), int)):
        raise TypeError
    elif stride is not None and stride <= 0:
        raise ValueError
    elif not isinstance(alignment, (type(None), int)):
        raise TypeError
    elif alignment is not None and alignment <= 0:
        raise ValueError

    executable = registry[Registry.KEY_EXECUTABLE]

    I_PDPOINTER = PrimitiveDataPointerContext.interface(library=executable)

    params = {}
    if offset is not None:
        params['offset'] = offset
    if offset_unit is not None:
        params['offset_unit'] = offset_unit
    if length is not None:
        params['length'] = length
    if stride is not None:
        params['stride'] = stride
    if alignment is not None:
        params['alignment'] = alignment

    return registry.new_context(I_PDPOINTER(pointee=pointee, writable=writable, **params),
                                key=registry.key(key, tmp_prefix='pdptr'))


def complex_data_pointer(registry, pointee, /, writable=False,
                         offset=None, offset_unit=None, tag=None, key=None):
    """Context manager for complex data pointers.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(writable, bool):
        raise TypeError
    elif not isinstance(offset, (type(None), int)):
        raise TypeError
    elif not isinstance(offset_unit, (type(None), int)):
        raise TypeError
    elif offset_unit is not None and offset_unit <= 0:
        raise ValueError
    elif not isinstance(tag, (type(None), int)):
        raise TypeError
    elif tag is not None and tag < 0:
        raise ValueError

    executable = registry[Registry.KEY_EXECUTABLE]

    I_CDPOINTER = ComplexDataPointerContext.interface(library=executable)

    params = {}
    if offset is not None:
        params['offset'] = offset
    if offset_unit is not None:
        params['offset_unit'] = offset_unit
    if tag is not None:
        params['tag'] = tag

    return registry.new_context(I_CDPOINTER(pointee=pointee, writable=writable, **params),
                                key=registry.key(key, tmp_prefix='cdptr'))


def function_pointer(registry, pointee, /, tag=None, key=None):
    """Context manager for function pointers.
    """
    if not isinstance(registry, Registry):
        raise TypeError
    elif not isinstance(tag, (type(None), int)):
        raise TypeError
    elif tag is not None and tag < 0:
        raise ValueError

    executable = registry[Registry.KEY_EXECUTABLE]

    I_FPOINTER = FunctionPointerContext.interface(library=executable)

    params = {}
    if tag is not None:
        params['tag'] = tag

    return registry.new_context(I_FPOINTER(pointee=pointee, **params),
                                key=registry.key(key, tmp_prefix='fptr'))

