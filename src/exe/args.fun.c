/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
#include "archi/app/plugin.def.h" // for ARCHI_PLUGIN_VTABLE_DEFAULT
#include "archi/app/version.def.h" // for ARCHI_API_VERSION
#include "archi/util/print.def.h" // for ARCHI_LOG_VERBOSITY_*

#include <stdlib.h>
#include <string.h>

#include <argp.h>

#define _STR(x) #x
#define STRINGIFY(x) _STR(x)

enum {
    ARGKEY_EXEC_SHM_PATHNAME = 'i',
    ARGKEY_EXEC_SHM_PROJ_ID = 'j',

    ARGKEY_PROBE_PLUGIN_PATHNAME = 'P',
    ARGKEY_PROBE_PLUGIN_VTABLE = 'T',
    ARGKEY_PROBE_PLUGIN_HELP = 'H',

    ARGKEY_NO_LOGO = 'q',
    ARGKEY_VERBOSITY = 'v',

    ARGKEY_VERSION = 'V',
    ARGKEY_HELP = 0xFF,
};

static
const struct argp_option args_options[] = {
    {.doc = "Application execution mode:"},

    {.key = ARGKEY_EXEC_SHM_PATHNAME,   .name = "shm-path", .arg = "PATHNAME",
                                        .doc = "Change pathname (default: argv[0]) of shared memory "
                                               "containing app configuration"},
    {.key = ARGKEY_EXEC_SHM_PROJ_ID,    .name = "shm-proj", .arg = "ID",
                                        .doc = "Change project identifier (1-255, default: 1) of shared memory "
                                               "containing app configuration"},

    {.doc = "Plugin probe mode:"},

    {.key = ARGKEY_PROBE_PLUGIN_PATHNAME,   .name = "plugin", .arg = "PATHNAME",
                                            .doc = "Specify plugin pathname, enable probe mode"},
    {.key = ARGKEY_PROBE_PLUGIN_VTABLE,     .name = "plugin-vtable", .arg = "SYMBOL",
                                            .doc = "Change plugin virtual table symbol (default: \""
                                                   STRINGIFY(ARCHI_PLUGIN_VTABLE_DEFAULT) "\")"},

    {.key = ARGKEY_PROBE_PLUGIN_HELP,       .name = "plugin-help", .arg = "TOPIC", .flags = OPTION_ARG_OPTIONAL,
                                            .doc = "Display plugin help (for topic, if specified)"},

    {.doc = "Verbosity options:"},

    {.key = ARGKEY_NO_LOGO,     .name = "no-logo",  .doc = "Don't display the logo"},
    {.key = ARGKEY_VERBOSITY,   .name = "verbose", .arg = "LEVEL", .flags = OPTION_ARG_OPTIONAL,
                                .doc = "Set verbosity level (0-" STRINGIFY(ARCHI_LOG_VERBOSITY_MAX)
                                       ", or one of: quiet, error, warning, notice, info, debug, max; "
                                       "default: info)"},

    {.key = ARGKEY_VERSION,     .name = "version",  .doc = "Display application version and exit",      .group = -1},
    {.key = ARGKEY_HELP,        .name = "help",     .doc = "Display a short help message and exit",     .group = -1},

    {0}
};

static
error_t
args_parse(int key, char *arg, struct argp_state *state)
{
    archi_cmdline_args_t *args = state->input;

    switch (key)
    {
        case ARGKEY_EXEC_SHM_PATHNAME:
            if (args->exec_mode.pathname != NULL)
                return EINVAL; // setting multiple configuration pathnames is not supported

            args->exec_mode.pathname = arg;
            break;

        case ARGKEY_EXEC_SHM_PROJ_ID:
            if (args->exec_mode.proj_id != 0)
                return EINVAL; // setting multiple configuration project identifiers is not supported

            int proj_id = atoi(arg);
            if ((proj_id <= 0) || (proj_id > 0xFF))
                return EINVAL; // project identifier is invalid

            args->exec_mode.proj_id = proj_id;
            break;

        case ARGKEY_PROBE_PLUGIN_PATHNAME:
            if (args->probe_mode.pathname != NULL)
                return EINVAL; // displaying help for multiple plugins is not supported

            args->probe_mode.pathname = arg;
            break;

        case ARGKEY_PROBE_PLUGIN_VTABLE:
            if (args->probe_mode.vtable_symbol != NULL)
                return EINVAL; // displaying help for multiple vtables is not supported

            args->probe_mode.vtable_symbol = arg;
            break;

        case ARGKEY_PROBE_PLUGIN_HELP:
            if (args->probe_mode.help_topic_set)
                return EINVAL; // displaying help for multiple topics is not supported

            args->probe_mode.help_topic = arg;
            args->probe_mode.help_topic_set = true;
            break;

        case ARGKEY_NO_LOGO:
            args->no_logo = true;
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

        case ARGKEY_VERSION:
            printf("%u.%.2u.%.2u\n",
                    ARCHI_API_VERSION / 10000,
                    ARCHI_API_VERSION / 100 % 100,
                    ARCHI_API_VERSION % 100);
            exit(EXIT_SUCCESS);

        case ARGKEY_HELP:
            argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
            exit(EXIT_SUCCESS);

        case ARGP_KEY_ARG:
        case ARGP_KEY_ARGS:
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
archi_parse_cmdline_args(
        archi_cmdline_args_t *args,

        int argc,
        char *argv[])
{
    if (args == NULL)
        return false;

    *args = (archi_cmdline_args_t){.verbosity_level = ARCHI_LOG_VERBOSITY_DEFAULT_LOWER};

    // Parse command line arguments
    struct argp args_parser = {
        .options = args_options,
        .parser = args_parse,
        .doc = "\n\
Initialize an application according to configuration\n\
provided via shared memory and execute the finite state machine,\n\
or, alternatively, display plugin virtual table information and help.\n\
\v\
Application execution mode is the default.\n\
If plugin pathname is provided, plugin probe mode is enabled instead.\n\
"
    };

    archi_status_t code = argp_parse(&args_parser, argc, argv,
            ARGP_NO_EXIT | ARGP_NO_HELP, NULL, args);

    if (args->exec_mode.pathname == NULL)
        args->exec_mode.pathname = argv[0];

    if (args->exec_mode.proj_id == 0)
        args->exec_mode.proj_id = 1;

    if ((args->probe_mode.pathname != NULL) && (args->probe_mode.vtable_symbol == NULL))
        args->probe_mode.vtable_symbol = STRINGIFY(ARCHI_PLUGIN_VTABLE_DEFAULT);

    return code;
}

