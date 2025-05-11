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

#include "archi/ctx/interface.fun.h"
#include "archi/ipc/signal/api.typ.h"

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

///////////////////////////////////////////////////////////////////////////////

static
struct {
    archi_exe_args_t args; ///< Command line arguments.

    struct archi_context *registry;   ///< The context registry.
    struct archi_context *interfaces; ///< The dictionary of built-in context interfaces.

    struct archi_context *signal_management; ///< The signal management context.
    archi_context_interface_t signal_management_interface; ///< The signal management context interface.
    archi_signal_watch_set_t *signal_watch_set; ///< The signal watch set.
} archi_process;

///////////////////////////////////////////////////////////////////////////////

static
void
exit_cleanup(void);

static
void
exit_quick(void);

static
void
print_logo(void);

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

    // Set logging stream
    archi_exe_log_init_stream(stderr);

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

    // Display the logo
    if (!archi_process.args.no_logo)
        print_logo();

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

void
print_logo(void)
{
#define LINES    2
#define LETTERS  13

    static const char *logo[LINES][LETTERS] = {
        {"⡏", "⢀⣀", "⡀⣀", "⢀⣀", "⣇⡀", "⠄", "⣀⡀", "⢀⡀", "⡇", "⢀⣀", "⢀⡀", "⢀⡀", "⢹"},
        {"⣇", "⠣⠼", "⠏ ", "⠣⠤", "⠇⠸", "⠇", "⡧⠜", "⠣⠭", "⠣", "⠣⠼", "⣑⡺", "⠣⠜", "⣸"},
    };

    static const char *colors[LETTERS] = {
        ARCHI_COLOR_FG_BRI_WHITE,   // [
        ARCHI_COLOR_FG_BRI_RED,     // a
        ARCHI_COLOR_FG_BRI_YELLOW,  // r
        ARCHI_COLOR_FG_BRI_GREEN,   // c
        ARCHI_COLOR_FG_BRI_BLUE,    // h
        ARCHI_COLOR_FG_BRI_MAGENTA, // i
        ARCHI_COLOR_FG_BRI_BLACK,   // p
        ARCHI_COLOR_FG_BRI_BLACK,   // e
        ARCHI_COLOR_FG_BRI_BLACK,   // l
        ARCHI_COLOR_FG_BRI_BLACK,   // a
        ARCHI_COLOR_FG_BRI_BLACK,   // g
        ARCHI_COLOR_FG_BRI_BLACK,   // o
        ARCHI_COLOR_FG_BRI_WHITE,   // ]
    };

    static const char *space = " ";

    if (!archi_process.args.no_color)
        archi_print(ARCHI_COLOR_RESET);

    archi_print("\n");

    for (int i = 0; i < LINES; i++)
    {
        archi_print("%s", space);

        for (int j = 0; j < LETTERS; j++)
        {
            if (!archi_process.args.no_color)
                archi_print("%s", colors[j]);

            archi_print("%s%s", space, logo[i][j]);
        }

        archi_print("\n");
    }

    if (!archi_process.args.no_color)
        archi_print(ARCHI_COLOR_RESET);

    archi_print("\n\n");

#undef LINES
#undef LETTERS
}

