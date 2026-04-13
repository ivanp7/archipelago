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
# @brief High-level wrapper for aggregate object contexts.

from contextlib import contextmanager

from archi.ctypes import (
        ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE,
        ARCHI_POINTER_DATA_TAG__AGGR_TYPE,
        )
from archi.context import Registry, TypeAttributes, get_entity_attr
from archi.builtin import AggregateContext


def aggregate_type(slot):
    """Set aggregate type data tag for a library slot.
    """
    return slot(function=False, tag=ARCHI_POINTER_DATA_TAG__AGGR_TYPE)


@contextmanager
def aggregate_object(registry, aggr_type, /, fam_length=None, key=None):
    """Context manager of an aggregate object context (generic aggregate type interface).
    """
    if not isinstance(registry, Registry):
        raise TypeError

    attr = get_entity_attr(aggr_type)

    if attr is not ... and not attr.is_compatible_to(
            TypeAttributes.complex_data(ARCHI_POINTER_DATA_TAG__AGGR_TYPE)):
        raise TypeError("Metadata is not an aggregate type description")

    executable = registry[Registry.KEY_EXECUTABLE]

    I_AGGREGATE = AggregateContext.interface(library=executable)

    params = {}
    if fam_length is not None:
        params['fam_length'] = fam_length

    if key is None:
        key = registry.temp_key(prefix='aggregate', rnd_len=6)

    with registry.temp_context(I_AGGREGATE(interface=executable.archi_aggr_interface__generic(
                                                function=False, tag=ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE),
                                           metadata=aggr_type,
                                           **params),
                               key=key) as aggregate:
        yield aggregate

