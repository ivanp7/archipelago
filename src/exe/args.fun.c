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
#include "archi/log/print.def.h" // for ARCHI_LOG_VERBOSITY_*

#include <stdlib.h>
#include <string.h>

#include <argp.h>

#define ARCHI_LOG_VERBOSITY_DEFAULT_LOWER ARCHI_LOG_VERBOSITY_NOTICE ///< Lower of two default verbosity levels.
#define ARCHI_LOG_VERBOSITY_DEFAULT_HIGHER ARCHI_LOG_VERBOSITY_INFO  ///< Higher of two default verbosity levels.

enum {
    ARGKEY_DRY_RUN = 'n',
    ARGKEY_NO_HSP  = 'p',

    ARGKEY_NO_LOGO = 'q',
    ARGKEY_VERBOSITY = 'v',
    ARGKEY_NO_COLOR = 'm',

    ARGKEY_HELP = 0xFF,
};

#define _STR(x) #x
#define STRINGIFY(x) _STR(x)

static
const struct argp_option args_options[] = {
    {.doc = "Execution options:"},

    {.key = ARGKEY_DRY_RUN,     .name = "dry-run",  .doc = "Do a dry run - only print what is to be done"},
    {.key = ARGKEY_NO_HSP,      .name = "no-hsp",   .doc = "Don't execute the HSP, do initialization only"},

    {.doc = "Verbosity options:"},

    {.key = ARGKEY_NO_LOGO,     .name = "no-logo",  .doc = "Don't display the logo"},
    {.key = ARGKEY_NO_COLOR,    .name = "no-color", .doc = "Disable use of colors for log messages"},
    {.key = ARGKEY_VERBOSITY,   .name = "verbose",  .arg = "[LEVEL]", .flags = OPTION_ARG_OPTIONAL,
                                    .doc = "Set logging verbosity level: 0-" STRINGIFY(ARCHI_LOG_VERBOSITY_MAX)
                                           ", or one of: quiet/error/warning/notice/info/debug/max, "
                                           "or no arg (same as 'notice'); default level is 'info'"},

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

        case ARGKEY_NO_HSP:
            args->no_hsp = true;
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
A versatile, modular application configured by memory-mapped files and plugins that define its specific behavior.\n\
\v\
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

