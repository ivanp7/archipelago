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
    ARGKEY_HELP = 0xFF,
    ARGKEY_VERSION = 'V',

    ARGKEY_NO_LOGO = 'q',
    ARGKEY_VERBOSITY = 'v',

    ARGKEY_PLUGIN_HELP_TOPIC = 'H',

    ARGKEY_PLUGIN_PATHNAME = 'p',
    ARGKEY_PLUGIN_VTABLE = 't',

    ARGKEY_CONFIG_SHMEM_PATHNAME = 'i',
    ARGKEY_CONFIG_SHMEM_PROJ_ID = 'j',
};

static
const struct argp_option args_options[] = {
    {.doc = "Plugin help mode:"},

    {.key = ARGKEY_PLUGIN_HELP_TOPIC,   .name = "plugin-help", .arg = "TOPIC", .flags = OPTION_ARG_OPTIONAL,
                                                        .doc = "Display plugin help for the specified topic and exit"},

    {.key = ARGKEY_PLUGIN_PATHNAME,     .name = "plugin-file", .arg = "PATHNAME",
                                                        .doc = "Specify pathname to plugin file"},
    {.key = ARGKEY_PLUGIN_VTABLE,       .name = "plugin-vtable", .arg = "SYMBOL",
                                                        .doc = "Specify plugin virtual table symbol (default: "
                                                               STRINGIFY(ARCHI_PLUGIN_VTABLE_DEFAULT) ")"},

    {.doc = "Configuration options:"},

    {.key = ARGKEY_CONFIG_SHMEM_PATHNAME,   .name = "path", .arg = "PATHNAME",
                                                        .doc = "Read configuration from shared memory"
                                                               " at specified pathname (default: argv[0])"},
    {.key = ARGKEY_CONFIG_SHMEM_PROJ_ID,    .name = "proj", .arg = "ID",
                                                        .doc = "Read configuration from shared memory"
                                                               " at specified project identifier (default: 1)"},

    {.doc = "Output options:"},

    {.key = ARGKEY_NO_LOGO,         .name = "no-logo",  .doc = "Don't print the logo"},
    {.key = ARGKEY_VERBOSITY,       .name = "verbose", .arg = "LEVEL", .flags = OPTION_ARG_OPTIONAL,
                                                        .doc = "Set verbosity level (0-" STRINGIFY(ARCHI_LOG_VERBOSITY_MAX)
                                                               ", or one of: quiet, error, warning, notice, info, debug, max),\n"
                                                               "no value == info"},

    {.key = ARGKEY_HELP,            .name = "help",     .doc = "Display a short help message and exit",     .group = -1},
    {.key = ARGKEY_VERSION,         .name = "version",  .doc = "Display application version and exit",      .group = -1},

    {0}
};

static
error_t
args_parse(int key, char *arg, struct argp_state *state)
{
    archi_cmdline_args_t *args = state->input;

    switch (key)
    {
        case ARGKEY_HELP:
            argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
            exit(EXIT_SUCCESS);

        case ARGKEY_VERSION:
            printf("%u.%.2u.%.2u\n",
                    ARCHI_API_VERSION / 10000,
                    ARCHI_API_VERSION / 100 % 100,
                    ARCHI_API_VERSION % 100);
            exit(EXIT_SUCCESS);

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

        case ARGKEY_PLUGIN_HELP_TOPIC:
            if (args->plugin_help.mode)
                return EINVAL; // displaying help for multiple topics is not supported

            args->plugin_help.mode = true;
            args->plugin_help.topic = arg;
            break;

        case ARGKEY_PLUGIN_PATHNAME:
            if (args->plugin_help.pathname != NULL)
                return EINVAL; // displaying help for multiple plugins is not supported

            args->plugin_help.pathname = arg;
            break;

        case ARGKEY_PLUGIN_VTABLE:
            if (args->plugin_help.vtable_symbol != NULL)
                return EINVAL; // displaying help for multiple vtables is not supported

            args->plugin_help.vtable_symbol = arg;
            break;

        case ARGKEY_CONFIG_SHMEM_PATHNAME:
            if (args->config.pathname != NULL)
                return EINVAL; // setting multiple configuration pathnames is not supported

            args->config.pathname = arg;
            break;

        case ARGKEY_CONFIG_SHMEM_PROJ_ID:
            if (args->config.proj_id != 0)
                return EINVAL; // setting multiple configuration project identifiers is not supported

            int proj_id = atoi(arg);
            if ((proj_id <= 0) || (proj_id > 0xFF))
                return EINVAL; // project identifier is invalid

            args->config.proj_id = proj_id;
            break;

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
Initialize an application according to\n\
the provided configuration and execute it.\n\
\v\
By default, if no configuration file were provided,\n\
standard input is used instead.\n\
"
    };

    archi_status_t code = argp_parse(&args_parser, argc, argv,
            ARGP_NO_EXIT | ARGP_NO_HELP, NULL, args);

    if (args->config.pathname == NULL)
        args->config.pathname = argv[0];

    if (args->config.proj_id == 0)
        args->config.proj_id = 1;

    if (args->plugin_help.mode && (args->plugin_help.vtable_symbol == NULL))
        args->plugin_help.vtable_symbol = STRINGIFY(ARCHI_PLUGIN_VTABLE_DEFAULT);

    return code;
}

