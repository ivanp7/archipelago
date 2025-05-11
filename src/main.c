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
 * @brief Implementation of the Archipelago executable.
 */

#include "archi/exe/logging.fun.h"
#include "archi/exe/args.fun.h"
#include "archi/exe/args.typ.h"
#include "archi/log/context.fun.h"
#include "archi/log/print.fun.h"
#include "archi/log/color.def.h"

#include "archi/app/context.fun.h"
#include "archi/ipc/signal/interface.typ.h"

#include "archi/builtin/ds_hashmap/context.var.h"
#include "archi/builtin/ds_lfqueue/context.var.h"
#include "archi/builtin/hsp/context.var.h"
#include "archi/builtin/ipc_env/context.var.h"
#include "archi/builtin/ipc_signal/context.var.h"
#include "archi/builtin/mem/context.var.h"
#include "archi/builtin/res_file/context.var.h"
#include "archi/builtin/res_library/context.var.h"
#include "archi/builtin/res_thread_group/context.var.h"

#include <stdlib.h> // for atexit(), at_quick_exit()


#define ARCHIPELAGO "\
  ⡏ ⢀⣀ ⡀⣀ ⢀⣀ ⣇⡀ ⠄ ⣀⡀ ⢀⡀ ⡇ ⢀⣀ ⢀⡀ ⢀⡀ ⢹  \n\
  ⣇ ⠣⠼ ⠏  ⠣⠤ ⠇⠸ ⠇ ⡧⠜ ⠣⠭ ⠣ ⠣⠼ ⣑⡺ ⠣⠜ ⣸  \n\
"

///////////////////////////////////////////////////////////////////////////////

static
struct {
    archi_exe_args_t args; ///< Command line arguments.

    struct archi_context *registry; ///< Application context registry.
    struct archi_context *interfaces; ///< Dictionary of built-in context interfaces.

    struct archi_context *signal_management; ///< Signal management context.
    archi_context_interface_t signal_management_interface; ///< Signal management context interface.
    archi_signal_watch_set_t *signal_watch_set; ///< Signal watch set.
} archi_process;

///////////////////////////////////////////////////////////////////////////////

static
void
exit_cleanup(void);

static
void
exit_quick(void);

///////////////////////////////////////////////////////////////////////////////

int
main(
        int argc,
        char *argv[])
{
    //////////////////
    // Preparations //
    //////////////////

    // Record the application start time
    archi_exe_log_init_start_time();

    // Parse command line arguments
    switch ((int)archi_exe_args_parse(&archi_process.args, argc, argv))
    {
        case 0: // success
            break;

        case ARCHI_STATUS_EVALUE: // incorrect arguments
            archi_print("Error: incorrect command line arguments.\n");
            return 2;

        case ARCHI_STATUS_ENOMEMORY: // memory allocation error
            archi_print("Error: memory allocation failure while parsing command line arguments.\n");
            return 3;

        case ARCHI_STATUS_EFAILURE:
        default: // unknown error
            archi_print("Error: unknown failure while parsing command line arguments.\n");
            return 1;
    }

    // Initialize logging subsystem
    archi_exe_log_init_verbosity(archi_process.args.verbosity_level);
    archi_exe_log_init_colors(!archi_process.args.no_color);

    archi_log_initialize(archi_exe_log_context());

    // Display application logo
    if (!archi_process.args.no_logo)
        archi_print(ARCHI_COLOR_RESET "\n" ARCHI_COLOR_FG_BRI_WHITE "%s"
                ARCHI_COLOR_RESET "\n\n", ARCHIPELAGO);

    // Exit if there is nothing to do
    if (archi_process.args.num_inputs == 0)
        return 0;

    // Set exit functions
    atexit(exit_cleanup);
    at_quick_exit(exit_quick);

    //////////////////////////
    // Initialization phase //
    //////////////////////////

    // TODO

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void
exit_cleanup(void) // is called on exit() or if main() returns
{
    ////////////////////////
    // Finalization phase //
    ////////////////////////

    // TODO
}

void
exit_quick(void) // is called on quick_exit()
{
    // TODO
}

