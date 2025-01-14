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
 * @brief Default main() implementation.
 */

#include "archi/exe/context.fun.h"
#include "archi/exe/args.fun.h"
#include "archi/exe/args.typ.h"
#include "archi/fsm/algorithm.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"
#include "archi/util/print.def.h"
#include "archi/util/os/shm.fun.h"

#include <stdlib.h> // for atexit()
#include <errno.h> // for error codes

/*****************************************************************************/

#define ARCHI_PELAGO_LOGO "\
  ⡏ ⢀⣀ ⡀⣀ ⢀⣀ ⣇⡀ ⠄ ⣀⡀ ⢀⡀ ⡇ ⢀⣀ ⢀⡀ ⢀⡀ ⢹  \n\
  ⣇ ⠣⠼ ⠏  ⠣⠤ ⠇⠸ ⠇ ⡧⠜ ⠣⠭ ⠣ ⠣⠼ ⣑⡺ ⠣⠜ ⣸  \n\
"

#define M "main()" // module name for logging

/*****************************************************************************/

static
void **archi_main_shmaddr;

static
archi_application_t archi_main_app;

static
void
archi_exit_cleanup(void) // will be called on exit(), or if main() returns
{
    archi_log_debug(M, "Finalizing application...");
    archi_app_finalize(&archi_main_app);

    archi_log_debug(M, "Detaching shared memory...");
    archi_shared_memory_detach(archi_main_shmaddr);
}

int
main(
        int argc,
        char *argv[])
{
    archi_status_t code; // status of operations

    //////////////////
    // Preparations //
    //////////////////

    // Set the application start time
    archi_log_set_start_time();

    // Parse command line arguments
    archi_cmdline_args_t args;
    switch (archi_parse_cmdline_args(&args, argc, argv))
    {
        case 0: // success
            break;

        case EINVAL: // invalid arguments
            return ARCHI_EXIT_CODE(ARCHI_ERROR_MISUSE);
        case ENOMEM: // memory allocation error
            return ARCHI_EXIT_CODE(ARCHI_ERROR_ALLOC);
        default: // unknown error
            return ARCHI_EXIT_CODE(ARCHI_ERROR_UNKNOWN);
    }

    // Set logging verbosity
    archi_log_set_verbosity(args.verbosity_level);

    // Set cleanup function
    atexit(archi_exit_cleanup);

    //////////////////////////////
    // Display application logo //
    //////////////////////////////

    archi_print(ARCHI_COLOR_RESET "\n");
    if (!args.no_logo)
        archi_print(ARCHI_COLOR_FG_BRI_WHITE "%s" ARCHI_COLOR_RESET "\n\n", ARCHI_PELAGO_LOGO);

    //////////////////////////
    // Attach shared memory //
    //////////////////////////

    archi_log_debug(M, "Attaching shared memory...");

    archi_main_shmaddr = archi_shared_memory_attach(args.pathname, args.proj_id, false);
    if (archi_main_shmaddr == NULL)
    {
        archi_log_error(M, "Couldn't attach to shared memory at pathname '%s', project id %i.",
                args.pathname, args.proj_id);

        return ARCHI_EXIT_CODE(ARCHI_ERROR_ATTACH);
    }

    /////////////////////////
    // Parse shared memory //
    /////////////////////////

    archi_log_debug(M, "Parsing shared memory...");

    const archi_signal_watch_set_t *signal_watch_set = {0};
    archi_container_t plugin_libraries = {0};
    archi_container_t plugin_interfaces = {0};
    archi_container_t app_config_steps = {0};

    archi_app_parse_shm(archi_main_shmaddr, &signal_watch_set,
            &plugin_libraries, &plugin_interfaces, &app_config_steps);

    ////////////////////////////
    // Initialize application //
    ////////////////////////////

    archi_log_debug(M, "Initializing application...");

    code = archi_app_initialize(&archi_main_app, signal_watch_set);
    if (code != 0)
    {
        archi_log_error(M, "Couldn't initialize the application.");
        return ARCHI_EXIT_CODE(code);
    }

    ///////////////////////////
    // Configure application //
    ///////////////////////////

    archi_log_info(M, "Configuring application...");

    code = archi_app_configure(&archi_main_app,
            plugin_libraries, plugin_interfaces, app_config_steps);
    if (code != 0)
    {
        archi_log_error(M, "Couldn't configure the application.");
        return ARCHI_EXIT_CODE(code);
    }

    /////////////////////////
    // Execute application //
    /////////////////////////

    archi_log_info(M, "Executing application...");

    code = archi_finite_state_machine(archi_main_app.context.entry_state,
            archi_main_app.context.transition);

    return ARCHI_EXIT_CODE(code);
}

