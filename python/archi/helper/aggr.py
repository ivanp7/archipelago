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
# @brief Helpers for aggregate object contexts.

from archi.context import (
        Registry,
        AggregateContext,
        AggregateInterfaceSymbol,
        AggregateTypeSymbol,
        )


def aggregate_object_spec(registry, plugin, type_name, /, fam_length=None):
    """Context specification for an aggregate object context (generic aggregate type interface).
    """
    if not isinstance(registry, Registry):
        raise TypeError

    I_AGGREGATE = AggregateContext.interface(library=registry[Registry.KEY_EXECUTABLE])

    params = {}
    if fam_length is not None:
        params['fam_length'] = fam_length

    return I_AGGREGATE(interface=AggregateInterfaceSymbol.slot(registry[Registry.KEY_EXECUTABLE], 'generic'),
                       metadata=AggregateTypeSymbol.slot(plugin, type_name),
                       **params)

