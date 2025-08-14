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

#include "archi_exe/args.fun.h"
#include "archi_exe/args.typ.h"
#include "archi_exe/registry.def.h"
#include "archipelago/log/verbosity.def.h"

#include <stdlib.h>
#include <string.h>

#include <argp.h>

#define ARCHI_LOG_VERBOSITY_DEFAULT_LOWER ARCHI_LOG_VERBOSITY_NOTICE ///< Lower of two default verbosity levels.
#define ARCHI_LOG_VERBOSITY_DEFAULT_HIGHER ARCHI_LOG_VERBOSITY_INFO  ///< Higher of two default verbosity levels.

enum {
    ARGKEY_DRY_RUN = 'n',

    ARGKEY_NO_LOGO = 'L',
    ARGKEY_NO_COLOR = 'm',
    ARGKEY_VERBOSITY = 'v',

    ARGKEY_HELP = 0xFF,
};

#define _STR(x) #x
#define STRINGIFY(x) _STR(x)

static
const struct argp_option args_options[] = {
    {.doc = "Execution:"},

    {.key = ARGKEY_DRY_RUN,     .name = "dry-run",  .group = 1,
        .doc = "Simulate initialization: instructions are logged, not executed"},

    {.doc = "Output:"},

    {.key = ARGKEY_NO_LOGO,     .name = "no-logo",  .group = 2,
                                    .doc = "Don't display the logo"},
    {.key = ARGKEY_NO_COLOR,    .name = "no-color", .group = 3,
                                    .doc = "Monochrome logging - don't use ANSI escape codes"},
    {.key = ARGKEY_VERBOSITY,   .name = "verbose",  .arg = "[LEVEL]", .flags = OPTION_ARG_OPTIONAL, .group = 3,
                                    .doc = "Set logging verbosity level (0-" STRINGIFY(ARCHI_LOG_VERBOSITY_MAX)
                                           " or quiet/error/warning/notice/info/debug/max).\n"
                                           "No argument = info. Default = notice"},

    {0}
};

static
error_t
args_parse(int key, char *arg, struct argp_state *state)
{
    archi_exe_args_t *args = state->input;

    switch (key)
    {
        case ARGKEY_DRY_RUN:
            args->dry_run = true;
            break;

        case ARGKEY_NO_LOGO:
            args->no_logo = true;
            break;

        case ARGKEY_NO_COLOR:
            args->no_color = true;
            break;

        case ARGKEY_VERBOSITY:
            if (arg == NULL)
                args->verbosity_level = ARCHI_LOG_VERBOSITY_DEFAULT_HIGHER;
            else if (!strcmp(arg, "quiet"))
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
            else if (!strcmp(arg, "max"))
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
            args->input      = state->argv + state->next;
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
archi_exe_args_parse(
        archi_exe_args_t *args,

        int argc,
        char *argv[])
{
    if (args == NULL)
        return ARCHI_STATUS_EMISUSE;

    *args = (archi_exe_args_t){.verbosity_level = ARCHI_LOG_VERBOSITY_DEFAULT_LOWER};

    // Parse command line arguments
    struct argp args_parser = {
        .options = args_options,
        .parser = args_parse,
        .args_doc = "[PATHNAME...]",
        .doc = "\n\
A versatile, modular application using plugins configured by memory-mapped initialization files that define its specific behavior.\n\
\v\
\n\
BUILT-IN CONTEXTS\n (keys in the application context registry):\n\
\n\
 \t\"" ARCHI_EXE_REGISTRY_KEY_REGISTRY "\"\t: the context registry itself\n\
 \t\"" ARCHI_EXE_REGISTRY_KEY_EXECUTABLE "\"\t: the library context of the executable itself\n\
 \t\"" ARCHI_EXE_REGISTRY_KEY_INPUT_FILE "\"\t: the context of the current input file\n\
\n\
 \t\"" ARCHI_EXE_REGISTRY_KEY_SIGNAL "\"\t: the signal management context\n\
"
    };

    error_t err = argp_parse(&args_parser, argc, argv, 0, NULL, args);
    switch (err)
    {
        case 0: // success
            return 0;

        case EINVAL: // incorrect arguments
            return ARCHI_STATUS_EVALUE;

        case ENOMEM: // memory allocation error
            return ARCHI_STATUS_ENOMEMORY;

        default: // unknown error
            return ARCHI_STATUS_EFAILURE;
    }
}

