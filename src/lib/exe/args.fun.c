/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Command line arguments parsing.
 */

#include "archi/exe/args.fun.h"
#include "archi/exe/args.typ.h"
#include "archi/util/print.def.h" // for ARCHI_LOG_VERBOSITY_*

#include <stdlib.h>
#include <string.h>

#include <argp.h>

#define _STR(x) #x
#define STRINGIFY(x) _STR(x)

enum {
    ARGKEY_NO_FSM = 'n',

    ARGKEY_NO_LOGO = 'q',
    ARGKEY_VERBOSITY = 'v',

    ARGKEY_HELP = 0xFF,
};

static
const struct argp_option args_options[] = {
    {.doc = "Application options:"},

    {.key = ARGKEY_NO_FSM,      .name = "no-fsm", .doc = "Don't execute the application FSM"},

    {.doc = "Verbosity options:"},

    {.key = ARGKEY_NO_LOGO,     .name = "no-logo", .doc = "Don't display the logo"},
    {.key = ARGKEY_VERBOSITY,   .name = "verbose", .arg = "LEVEL", .flags = OPTION_ARG_OPTIONAL,
                                    .doc = "Set verbosity level (0-" STRINGIFY(ARCHI_LOG_VERBOSITY_MAX)
                                           ", or one of: none, error, warning, notice, info, debug, all; "
                                           "default: info)"},

    {0}
};

static
error_t
args_parse(int key, char *arg, struct argp_state *state)
{
    archi_args_t *args = state->input;

    switch (key)
    {
        case ARGKEY_NO_FSM:
            args->no_fsm = true;
            break;

        case ARGKEY_NO_LOGO:
            args->no_logo = true;
            break;

        case ARGKEY_VERBOSITY:
            if (arg == NULL)
                args->verbosity_level = ARCHI_LOG_VERBOSITY_DEFAULT_HIGHER;
            else if (!strcmp(arg, "none"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_QUIET;
            else if (!strcmp(arg, "error"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_ERROR;
            else if (!strcmp(arg, "warning"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_WARNING;
            else if (!strcmp(arg, "notice"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_NOTICE;
            else if (!strcmp(arg, "info"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_INFO;
            else if (!strcmp(arg, "debug"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_DEBUG;
            else if (!strcmp(arg, "all"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_MAX;
            else if (strlen(arg) == 1)
            {
                args->verbosity_level = arg[0] - '0';
                if ((args->verbosity_level < 0) || (args->verbosity_level > ARCHI_LOG_VERBOSITY_MAX))
                    return EINVAL;
            }
            else
                return EINVAL;
            break;

        case ARGP_KEY_ARG:
            /* Let the ARGP_KEY_ARGS case parse it.  */
            return ARGP_ERR_UNKNOWN;

        case ARGP_KEY_ARGS:
            args->inputs = state->argv + state->next;
            args->num_inputs = state->argc - state->next;
            break;

        case ARGP_KEY_NO_ARGS:
        case ARGP_KEY_END:
        case ARGP_KEY_INIT:
        case ARGP_KEY_SUCCESS:
        case ARGP_KEY_ERROR:
        case ARGP_KEY_FINI:
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

archi_status_t
archi_args_parse(
        archi_args_t *args,

        int argc,
        char *argv[])
{
    if (args == NULL)
        return false;

    *args = (archi_args_t){.verbosity_level = ARCHI_LOG_VERBOSITY_DEFAULT_LOWER};

    // Parse command line arguments
    struct argp args_parser = {
        .options = args_options,
        .parser = args_parse,
        .args_doc = "[PATHNAME...]",
        .doc = "\n\
General purpose, modular application configured by memory-mapped files.\n\
\v\
"
    };

    return argp_parse(&args_parser, argc, argv, 0, NULL, args);
}

