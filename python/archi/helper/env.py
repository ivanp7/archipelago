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
# @brief Helpers for environment variable contexts.


from contextlib import contextmanager

from archi.context import (
        Registry,
        EnvVariableContext,
        NumberParserContext,
        )


@contextmanager
def env_variables(registry, /, **vars):
    """Environment variables optionally parsed as numbers.

    Each keyword argument corresponds to an environment variable.
    If the argument value is a string, it is used as the default value,
    and the string is not converted to a number.
    Otherwise, the argument value type must be a tuple of two or three elements:
    `(default_value, type_name)`
    or
    `(default_value, type_name, number_base)`
    where:
    `default_value` is a number;
    `type_name` is the name of the target number type as in
    number parser context initialization parameters;
    `number_base` is an integer in range [2; 36].
    """
    if not isinstance(registry, Registry):
        raise TypeError

    executable = registry[Registry.KEY_EXECUTABLE]

    # Check variable specifications for correctness
    for spec in vars.values():
        if isinstance(spec, tuple):
            if len(spec) < 2 or len(spec) > 3:
                raise ValueError
            elif not isinstance(spec[0], (int, float)):
                raise TypeError
            elif not isinstance(spec[1], str):
                raise TypeError
            elif not spec[1] or spec[1] == 'base':
                raise ValueError

            if len(spec) > 2:
                if not isinstance(spec[2], int):
                    raise TypeError
                elif spec[2] < 2 or spec[2] > 36:
                    raise ValueError

    # Form the set of default values
    default_values = {(str(spec[0]) if isinstance(spec, tuple) else spec) for spec in vars.values()}

    # Create environment variable contexts
    I_ENV_VARIABLE = EnvVariableContext.interface(library=executable)

    envvar_contexts = {}
    for default_value in default_values:
        envvar_contexts[default_value] = registry.new_context(I_ENV_VARIABLE(default_value=default_value),
                                                              key=registry.key(tmp_prefix='env_var'))

    del default_values

    # Parse the numbers and form the resulting list of contexts
    I_NUMBER_PARSER = NumberParserContext.interface(library=executable)

    parser_contexts = []
    values = []

    for var, spec in vars.items():
        if isinstance(spec, tuple):
            params = {spec[1]: getattr(envvar_contexts[str(spec[0])], var)}
            if len(spec) > 2:
                params['base'] = spec[2]

            context = registry.new_context(I_NUMBER_PARSER(**params), key=registry.key(tmp_prefix='number'))
            parser_contexts.append(context)

            values.append(context)
        else:
            values.append(getattr(envvar_contexts[spec], var))

    # Yield the values
    try:
        yield tuple(values)
    finally:
        # Delete all contexts
        for context in parser_contexts:
            registry.del_context(context)

        for context in envvar_contexts.values():
            registry.del_context(context)

