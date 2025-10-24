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

#include "archipelago/context/interface.fun.h"
#include "archipelago/base/ref_count.fun.h"
#include "archi_exe/args.fun.h"
#include "archi_exe/args.typ.h"
#include "archi_exe/input.typ.h"
#include "archi_exe/input.def.h"
#include "archi_exe/registry.def.h"
#include "archi_exe/instruction.fun.h"
#include "archi_exe/instruction.typ.h"
#include "archi/ds_hashmap/ctx/hashmap.var.h"
#include "archi/res_file/ctx/file.var.h"
#include "archi/res_library/ctx/library.var.h"

// Signal management
#include "archi/ipc_signal/api/management.fun.h"
#include "archi/ipc_signal/api/signal.fun.h"
#include "archi/ipc_signal/ctx/management.fun.h"

// Logging
#include "archi_exe/logging.fun.h"
#include "archipelago/log/context.fun.h"
#include "archipelago/log/print.fun.h"
#include "archipelago/log/verbosity.def.h"
#include "archipelago/log/color.def.h"

#include <stdlib.h>
#include <string.h> // for strcmp(), strncmp()
#include <stdio.h> // for snprintf()
#include <stdalign.h> // for alignof()

///////////////////////////////////////////////////////////////////////////////

static
struct {
    archi_exe_args_t args; ///< Command line arguments.

    archi_context_t registry;   ///< The context registry.
    archi_context_t exe_handle; ///< The library handle of the executable itself.
    archi_context_t *input_file; ///< Array of the input file contexts.

    archi_context_t signal; ///< The signal management context.
    archi_context_interface_t signal_interface; ///< The signal management context interface.
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

static
void
create_context_registry(void);

static
void
destroy_context_registry(void);

static
void
obtain_handle_of_executable(void);

static
void
open_and_map_input_files(void);

static
void
preliminary_pass_of_input_files(void);

static
void
decrement_refcount_of_input_files(void);

static
void
prepare_signal_management(void);

static
void
start_signal_management(void);

static
void
decrement_refcount_of_signal_management(void);

static
void
execute_instructions(void);

///////////////////////////////////////////////////////////////////////////////

static
void
print_signal_watch_set(
        const archi_signal_watch_set_t *signal_watch_set
);

///////////////////////////////////////////////////////////////////////////////

int
main(
        int argc,
        char *argv[])
{
    ///////////////////////
    // Preparation phase //
    ///////////////////////

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

    // Set logging verbosity level
    archi_exe_log_init_verbosity(archi_process.args.verbosity_level);

    // Enable or disable color in log messages
    archi_exe_log_init_color(!archi_process.args.no_color);

    // Initialize the logging module
    archi_log_initialize(archi_exe_log_context());

    // Display the logo
    if (!archi_process.args.no_logo)
        print_logo();

    // Exit if there is nothing to do
    if (archi_process.args.num_inputs == 0)
        return 0;

    // Set exit functions
    if (atexit(exit_cleanup) != 0)
        archi_log_warning(__func__, "atexit() failed, attempting to continue...");

    if (at_quick_exit(exit_quick) != 0)
        archi_log_warning(__func__, "at_quick_exit() failed, attempting to continue...");

    archi_log_info(__func__, "Preparing the application context registry...");

    // Create the context registry
    create_context_registry();

    // Obtain handle of the executable
    obtain_handle_of_executable();

    // Prepare signal management interface and allocate the signal watch set
    prepare_signal_management();

    // Open and map input files
    open_and_map_input_files();

    // Do preliminary pass of input files
    preliminary_pass_of_input_files();

    // Create signal management context
    start_signal_management();

    //////////////////////////////////////
    // Initialization & execution phase //
    //////////////////////////////////////

    archi_log_info(__func__, "Initializing and executing the application...");

    // Execute instructions in input files
    execute_instructions();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void
exit_cleanup(void) // is called on exit() or if main() returns
{
    ////////////////////////
    // Finalization phase //
    ////////////////////////

    archi_log_info(__func__, "Finalizing the application...");

    // Destroy the context registry
    destroy_context_registry();

    // Decrement reference counts of input file contexts
    decrement_refcount_of_input_files();

    // Decrement reference count of the signal management context
    decrement_refcount_of_signal_management();

    // Finalization is done
    archi_log_info(__func__, "The application has exited successfully.");
}

void
exit_quick(void) // is called on quick_exit()
{
    archi_log_info(__func__, "The application has exited without finalizing contexts and releasing resources.");
}

void
print_logo(void)
{
#define LETTERS  13
#define LINES    2

    static const char *logo[LINES][LETTERS] = {
        {"⡰⠁", "⣎⣱", "⣏⡱", "⡎⠑", "⣇⣸", "⡇", "⣏⡱", "⣏⡉", "⡇ ", "⣎⣱", "⡎⠑", "⡎⢱", "⠈⢆"},
        {"⠘⠄", "⠇⠸", "⠇⠱", "⠣⠔", "⠇⠸", "⠇", "⠇ ", "⠧⠤", "⠧⠤", "⠇⠸", "⠣⠝", "⠣⠜", "⠠⠃"},
    };

    static const char *colors[LETTERS] = {
        ARCHI_COLOR_FG(255), // {
        ARCHI_COLOR_FG(204), // A
        ARCHI_COLOR_FG(223), // R
        ARCHI_COLOR_FG(116), // C
        ARCHI_COLOR_FG(68),  // H
        ARCHI_COLOR_FG(140), // I
        ARCHI_COLOR_FG(240), // P
        ARCHI_COLOR_FG(241), // E
        ARCHI_COLOR_FG(242), // L
        ARCHI_COLOR_FG(243), // A
        ARCHI_COLOR_FG(244), // G
        ARCHI_COLOR_FG(245), // O
        ARCHI_COLOR_FG(255), // }
    };

    static const char *space = " ";

    archi_print_lock(0);

    archi_print_color(ARCHI_COLOR_RESET);
    archi_print("\n");

    for (int i = 0; i < LINES; i++)
    {
        archi_print("  ");

        for (int j = 0; j < LETTERS; j++)
        {
            if (!archi_process.args.no_color)
                archi_print("%s", colors[j]);

            archi_print("%s%s", space, logo[i][j]);
        }

        archi_print_color(ARCHI_COLOR_RESET);
        archi_print("\n");
    }

    archi_print("\n");

    archi_print_unlock();

#undef LINES
#undef LETTERS
}

void
create_context_registry(void)
{
    archi_log_debug(__func__, "Creating the context registry...");

    size_t hashmap_capacity = 1024;

    archi_parameter_list_t params[] = {
        {
            .name = "capacity",
            .value.ptr = &hashmap_capacity,
        },
    };

    archi_status_t code;

    archi_process.registry = archi_context_initialize(
            (archi_pointer_t){.ptr = (void*)&archi_context_hashmap_interface},
            params, &code);

    if (archi_process.registry == NULL)
    {
        archi_log_error(__func__, "Couldn't create the context registry (error %i).", code);
        exit(EXIT_FAILURE);
    }

    archi_log_debug(__func__, "Inserting the context registry into itself...");

    code = archi_context_set_slot(archi_process.registry,
            (archi_context_slot_t){.name = ARCHI_EXE_REGISTRY_KEY_REGISTRY},
            (archi_pointer_t){
                .ptr = archi_process.registry,
                .ref_count = archi_context_data(archi_process.registry).ref_count,
                .element.num_of = 1,
            });

    if (code != 0)
    {
        archi_log_error(__func__, "Couldn't insert the context registry into itself (error %i).", code);
        exit(EXIT_FAILURE);
    }
}

void
destroy_context_registry(void)
{
    if (archi_process.registry != NULL)
    {
        archi_log_debug(__func__, "Removing the context registry from itself...");

        archi_context_set_slot(archi_process.registry,
                (archi_context_slot_t){.name = ARCHI_EXE_REGISTRY_KEY_REGISTRY},
                (archi_pointer_t){0});

        archi_log_debug(__func__, "Destroying the context registry...");

        archi_context_finalize(archi_process.registry);

        archi_process.registry = NULL;
    }
}

void
obtain_handle_of_executable(void)
{
    archi_log_debug(__func__, "Obtaining library handle of the executable itself...");

    if (archi_process.args.dry_run)
        return;

    archi_status_t code;

    archi_process.exe_handle = archi_context_initialize(
            (archi_pointer_t){.ptr = (void*)&archi_context_library_interface},
            NULL, &code);

    if (archi_process.exe_handle == NULL)
    {
        archi_log_error(__func__, "Couldn't obtain library handle of the executable itself (error %i).", code);
        exit(EXIT_FAILURE);
    }

    archi_log_debug(__func__, "Inserting handle of the executable into the registry...");

    // Insert the context into the registry, which also increments the reference count
    code = archi_context_set_slot(archi_process.registry,
            (archi_context_slot_t){.name = ARCHI_EXE_REGISTRY_KEY_EXECUTABLE},
            (archi_pointer_t){
                .ptr = archi_process.exe_handle,
                .ref_count = archi_context_data(archi_process.exe_handle).ref_count,
                .element.num_of = 1,
            });

    // Decrement the reference count back to 1
    archi_reference_count_decrement(archi_context_data(archi_process.exe_handle).ref_count);

    if (code != 0)
    {
        archi_log_error(__func__, "Couldn't insert handle of the executable into the registry (error %i).", code);
        exit(EXIT_FAILURE);
    }

    // Reset the separate context pointer as it isn't needed anymore
    archi_process.exe_handle = NULL;
}

void
open_and_map_input_files(void)
{
    archi_log_debug(__func__, "Allocating the array of input file contexts...");

    archi_process.input_file = malloc(sizeof(*archi_process.input_file) * archi_process.args.num_inputs);

    if (archi_process.input_file == NULL)
    {
        archi_log_error(__func__, "Couldn't allocate the array of input file contexts.");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        archi_process.input_file[i] = NULL;

    archi_log_debug(__func__, "Opening and mapping input files...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(__func__, " * opening file #%zu ('%s')", i, archi_process.args.input[i]);
        {
            bool value_true = true;

            archi_parameter_list_t params[] = {
                {
                    .name = "pathname",
                    .value.ptr = archi_process.args.input[i],
                },
                {
                    .name = "readable",
                    .value.ptr = &value_true,
                },
            };
            for (size_t i = 0; i < (sizeof(params) / sizeof(params[0])) - 1; i++)
                params[i].next = &params[i + 1];

            archi_status_t code;

            archi_process.input_file[i] = archi_context_initialize(
                    (archi_pointer_t){.ptr = (void*)&archi_context_file_interface},
                    params, &code);

            if (archi_process.input_file[i] == NULL)
            {
                archi_log_error(__func__, "Couldn't open the input file (error %i).", code);
                exit(EXIT_FAILURE);
            }
        }

        archi_log_debug(__func__, " * mapping file #%zu", i);
        {
            bool value_true = true;

            archi_parameter_list_t params[] = {
                {
                    .name = "has_header",
                    .value.ptr = &value_true,
                },
                {
                    .name = "readable",
                    .value.ptr = &value_true,
                },
                {
                    .name = "close_fd",
                    .value.ptr = &value_true,
                },
            };
            for (size_t i = 0; i < (sizeof(params) / sizeof(params[0])) - 1; i++)
                params[i].next = &params[i + 1];

            archi_status_t code = archi_context_act(archi_process.input_file[i],
                    (archi_context_slot_t){.name = "map"},
                    params);

            if (code != 0)
            {
                archi_log_error(__func__, "Couldn't map the input file into memory (error %i).", code);
                exit(EXIT_FAILURE);
            }

            archi_pointer_t current_file = archi_context_data(archi_process.input_file[i]);

            if (current_file.element.num_of < sizeof(archi_exe_input_file_header_t))
            {
                archi_log_error(__func__, "Input file #%zu is invalid (file size is smaller than the header structure size).", i);
                exit(EXIT_FAILURE);
            }

            const archi_exe_input_file_header_t *current_input = current_file.ptr;
            if (strncmp(ARCHI_EXE_INPUT_MAGIC, current_input->magic, sizeof(current_input->magic)) != 0)
            {
                archi_log_error(__func__, "Input file #%zu is invalid (magic bytes are incorrect).", i);
                exit(EXIT_FAILURE);
            }

            ptrdiff_t contents_size = (intptr_t)current_input->header.end - (intptr_t)current_input->header.addr;

            if (contents_size < 0)
            {
                archi_log_error(__func__, "Input file #%zu is invalid (size of contents is negative).", i);
                exit(EXIT_FAILURE);
            }
            else if ((size_t)contents_size != current_file.element.num_of)
            {
                archi_log_error(__func__, "Input file #%zu is invalid (file size is not equal to size of contents).", i);
                exit(EXIT_FAILURE);
            }

            {
                archi_log_debug(__func__, "    address = %p, size = %tu B (%.2f KiB, %.2f MiB, %.2f GiB)",
                        current_input->header.addr, contents_size, contents_size / 1024.0,
                        contents_size / (1024.0 * 1024.0), contents_size / (1024.0 * 1024.0 * 1024.0));
            }
        }
    }
}

void
preliminary_pass_of_input_files(void)
{
    archi_log_debug(__func__, "Passing lists of contents of input files...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(__func__, " * file #%zu ('%s')", i, archi_process.args.input[i]);

        const archi_exe_input_file_header_t *current_input = archi_context_data(archi_process.input_file[i]).ptr;

        for (archi_parameter_list_t *contents = current_input->contents; contents != NULL; contents = contents->next)
        {
            if (strcmp(ARCHI_EXE_INPUT_FILE_CONTENTS_KEY_SIGNALS, contents->name) == 0)
            {
                if (contents->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                {
                    archi_log_error(__func__, "Signal watch set must not be a function.");
                    exit(EXIT_FAILURE);
                }

                archi_signal_watch_set_t *signal_watch_set = contents->value.ptr;
                archi_signal_watch_set_join(archi_process.signal_watch_set, signal_watch_set);

                if (archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG))
                {
                    archi_print_color(ARCHI_LOG_COLOR_DEBUG);

                    archi_print("\tsignals =");
                    print_signal_watch_set(signal_watch_set);
                    archi_print("\n");

                    archi_print_color(ARCHI_COLOR_RESET);

                    archi_print_unlock();
                }
            }
            else if (strcmp(ARCHI_EXE_INPUT_FILE_CONTENTS_KEY_INSTRUCTIONS, contents->name) == 0)
            {
                if (contents->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                {
                    archi_log_error(__func__, "List of instructions must not be a function.");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

void
decrement_refcount_of_input_files(void)
{
    if (archi_process.input_file != NULL)
    {
        archi_log_debug(__func__, "Decrementing reference counts of input file contexts...");

        for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        {
            if (archi_process.input_file[i] != NULL)
                archi_reference_count_decrement(archi_context_data(archi_process.input_file[i]).ref_count);
        }

        free(archi_process.input_file);
        archi_process.input_file = NULL;
    }
}

void
prepare_signal_management(void)
{
    archi_process.signal_interface = (archi_context_interface_t){
        .init_fn = archi_context_signal_management_init,
        .final_fn = archi_context_signal_management_final,
        .get_fn = archi_context_signal_management_get,
        .set_fn = archi_context_signal_management_set,
    };

    archi_log_debug(__func__, "Allocating the signal watch set...");

    archi_process.signal_watch_set = archi_signal_watch_set_alloc();

    if (archi_process.signal_watch_set == NULL)
    {
        archi_log_error(__func__, "Couldn't allocate the signal watch set.");
        exit(EXIT_FAILURE);
    }
}

void
start_signal_management(void)
{
    if (archi_signal_watch_set_not_empty(archi_process.signal_watch_set))
    {
        archi_log_debug(__func__, "Creating the signal management context...");

        if (archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG))
        {
            archi_print_color(ARCHI_LOG_COLOR_DEBUG);

            archi_print("\tsignals =");
            print_signal_watch_set(archi_process.signal_watch_set);
            archi_print("\n");

            archi_print_color(ARCHI_COLOR_RESET);

            archi_print_unlock();
        }

        if (archi_process.args.dry_run)
            return;

        archi_parameter_list_t params[] = {
            {
                .name = "signals",
                .value.ptr = archi_process.signal_watch_set,
            },
        };

        archi_status_t code;

        archi_process.signal = archi_context_initialize(
                (archi_pointer_t){.ptr = (void*)&archi_process.signal_interface},
                params, &code);

        if (archi_process.signal == NULL)
        {
            archi_log_error(__func__, "Couldn't create the signal management context (error %i).", code);
            exit(EXIT_FAILURE);
        }

        archi_log_debug(__func__, "Inserting the signal management context into the registry...");

        // Insert the context into the registry, which also increments the reference count
        code = archi_context_set_slot(archi_process.registry,
                (archi_context_slot_t){.name = ARCHI_EXE_REGISTRY_KEY_SIGNAL},
                (archi_pointer_t){
                    .ptr = archi_process.signal,
                    .ref_count = archi_context_data(archi_process.signal).ref_count,
                    .element.num_of = 1,
                });

        if (code != 0)
        {
            archi_log_error(__func__, "Couldn't insert the signal management context into the registry (error %i).", code);
            exit(EXIT_FAILURE);
        }

    }
    else
        archi_log_debug(__func__, "No signals in the watch set, skipping the signal management context creation.");

    free(archi_process.signal_watch_set);
    archi_process.signal_watch_set = NULL;
}

void
decrement_refcount_of_signal_management(void)
{
    if (archi_process.signal != NULL)
    {
        archi_log_debug(__func__, "Decrementing reference count of the signal management context...");

        archi_reference_count_decrement(archi_context_data(archi_process.signal).ref_count);
        archi_process.signal = NULL;
    }

    archi_process.signal_interface = (archi_context_interface_t){0};
}

void
execute_instructions(void)
{
    archi_log_debug(__func__, "Executing instructions in input files...");

    size_t instruction_number = 0;

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_pointer_t current_file = archi_context_data(archi_process.input_file[i]);

        archi_exe_registry_instr_list_t *instructions = NULL;
        {
            const archi_exe_input_file_header_t *current_input = current_file.ptr;

            for (archi_parameter_list_t *contents = current_input->contents;
                    contents != NULL; contents = contents->next)
            {
                if (strcmp(ARCHI_EXE_INPUT_FILE_CONTENTS_KEY_INSTRUCTIONS, contents->name) == 0)
                {
                    instructions = contents->value.ptr;
                    break;
                }
            }
        }

        if (instructions != NULL)
        {
            archi_log_debug(__func__, " * inserting mapped memory of file #%zu into the registry...", i);
            {
                // Insert the context into the registry, which also increments the reference count
                ptrdiff_t slot_index = 0;

                archi_status_t code = archi_context_set_slot(archi_process.registry,
                        (archi_context_slot_t){
                        .name = ARCHI_EXE_REGISTRY_KEY_INPUT_FILE,
                        .index = &slot_index,
                        .num_indices = 1, // allow updating value for the existing key in the hashmap
                        },
                        (archi_pointer_t){
                        .ptr = archi_process.input_file[i],
                        .ref_count = current_file.ref_count,
                        .element = {
                        .num_of = 1,
                        },
                        });

                if (code != 0)
                {
                    archi_log_error(__func__, "Couldn't insert mapped memory of file #%zu into the registry (error %i).", i, code);
                    exit(EXIT_FAILURE);
                }
            }

            archi_log_debug(__func__, " * executing instructions in file #%zu ('%s')", i, archi_process.args.input[i]);

            for (archi_exe_registry_instr_list_t *instr_list = instructions;
                    instr_list != NULL; instr_list = instr_list->next)
            {
                instruction_number++;

                if (archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG))
                {
                    archi_print_color(ARCHI_LOG_COLOR_DEBUG);

                    archi_exe_registry_instr_print(instr_list->instruction, instruction_number);

                    archi_print_color(ARCHI_COLOR_RESET);
                    archi_print_unlock();
                }

                if (!archi_process.args.dry_run)
                {
                    /*****************************************************/
                    archi_status_t code = archi_exe_registry_instr_execute(
                            archi_process.registry, instr_list->instruction,
                            current_file.ref_count);
                    /******************************************************/

                    if (code > 0)
                        archi_log_warning(__func__, "Got non-zero instruction execution status %i, attempting to continue...", code);
                    else if (code < 0)
                    {
                        archi_log_error(__func__, "Couldn't execute the instruction (error %i).", code);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        else
            archi_log_debug(__func__, " * no instructions in file #%zu ('%s'), skipping...", i, archi_process.args.input[i]);

        // Decrement the reference count back to 1
        archi_reference_count_decrement(current_file.ref_count);

        // Forget the file context
        archi_process.input_file[i] = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
print_signal_watch_set(
        const archi_signal_watch_set_t *signal_watch_set)
{
#define PRINT_SIGNAL(signal) do {         \
        if (signal_watch_set->f_##signal) \
            archi_print(" %s", #signal);  \
    } while (0)

    PRINT_SIGNAL(SIGINT);
    PRINT_SIGNAL(SIGQUIT);
    PRINT_SIGNAL(SIGTERM);

    PRINT_SIGNAL(SIGCHLD);
    PRINT_SIGNAL(SIGCONT);
    PRINT_SIGNAL(SIGTSTP);

    PRINT_SIGNAL(SIGXCPU);
    PRINT_SIGNAL(SIGXFSZ);

    PRINT_SIGNAL(SIGPIPE);
    PRINT_SIGNAL(SIGPOLL);
    PRINT_SIGNAL(SIGURG);

    PRINT_SIGNAL(SIGALRM);
    PRINT_SIGNAL(SIGVTALRM);
    PRINT_SIGNAL(SIGPROF);

    PRINT_SIGNAL(SIGHUP);
    PRINT_SIGNAL(SIGTTIN);
    PRINT_SIGNAL(SIGTTOU);
    PRINT_SIGNAL(SIGWINCH);

    PRINT_SIGNAL(SIGUSR1);
    PRINT_SIGNAL(SIGUSR2);

#undef PRINT_SIGNAL

    for (size_t i = 0; i < archi_signal_number_of_rt_signals(); i++)
    {
        if (signal_watch_set->f_SIGRTMIN[i])
            archi_print(" SIGRTMIN+%zu", i);
    }
}

