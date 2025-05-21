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

#include "archi/ctx/interface.fun.h"
#include "archi/exe/registry.def.h"
#include "archi/exe/args.fun.h"
#include "archi/exe/args.typ.h"
#include "archi/exe/input.typ.h"
#include "archi/exe/instruction.fun.h"
#include "archi/exe/instruction.typ.h"

// Logging
#include "archi/exe/logging.fun.h"
#include "archi/log/context.fun.h"
#include "archi/log/print.fun.h"
#include "archi/log/print.def.h"
#include "archi/log/color.def.h"

// Built-in context interfaces
#include "archi/ctx/interface/parameters.var.h"
#include "archi/ctx/interface/pointer.var.h"
#include "archi/builtin/ds_hashmap/context.var.h"
#include "archi/builtin/ds_lfqueue/context.var.h"
#include "archi/builtin/hsp/context.var.h"
#include "archi/builtin/ipc_env/context.var.h"
#include "archi/builtin/ipc_signal/context.var.h"
#include "archi/builtin/mem/context.var.h"
#include "archi/builtin/res_file/context.var.h"
#include "archi/builtin/res_library/context.var.h"
#include "archi/builtin/res_thread_group/context.var.h"

// Signal management
#include "archi/ipc/signal/api.fun.h"

#include <stdlib.h>
#include <string.h> // for strcmp(), strncmp()
#include <stdio.h> // for snprintf()
#include <stdalign.h> // for alignof()

///////////////////////////////////////////////////////////////////////////////

static
struct {
    archi_exe_args_t args; ///< Command line arguments.

    archi_context_t registry;   ///< The context registry.
    archi_context_t interfaces; ///< The hashmap of built-in context interfaces.
    archi_context_t exe_handle; ///< The library handle of the executable itself.

    archi_context_t *input_file; ///< The array of input files.

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
create_hashmap_of_interfaces(void);

static
void
obtain_handle_of_executable(void);

static
void
open_and_map_input_files(void);

static
void
decrement_refcount_of_input_files(void);

static
void
process_parameters_of_input_files(void);

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
        int verbosity,
        const archi_signal_watch_set_t *signal_watch_set
);

///////////////////////////////////////////////////////////////////////////////

int
main(
        int argc,
        char *argv[])
{
#define M "main()"

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
            archi_print(0, "Error: incorrect command line arguments.\n");
            return 2;

        case ARCHI_STATUS_ENOMEMORY: // memory allocation error
            archi_print(0, "Error: memory allocation failure while parsing command line arguments.\n");
            return 3;

        case ARCHI_STATUS_EFAILURE:
        default: // unknown error
            archi_print(0, "Error: unknown failure while parsing command line arguments.\n");
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
        archi_log_warning(M, "atexit() failed, attempting to continue...");

    if (at_quick_exit(exit_quick) != 0)
        archi_log_warning(M, "at_quick_exit() failed, attempting to continue...");

    archi_log_info(M, "Preparing the application context registry...");

    // Create the context registry
    create_context_registry();

    // Create the hashmap of built-in context interfaces
    create_hashmap_of_interfaces();

    // Obtain handle of the executable
    obtain_handle_of_executable();

    // Prepare signal management interface and allocate the signal watch set
    prepare_signal_management();

    // Open and map input files
    open_and_map_input_files();

    // Process parameter lists of input files
    process_parameters_of_input_files();

    // Create signal management context
    start_signal_management();

    //////////////////////////////////////
    // Initialization & execution phase //
    //////////////////////////////////////

    archi_log_info(M, "Initializing and executing the application...");

    // Execute instructions in input files
    execute_instructions();

    return 0;

#undef M
}

///////////////////////////////////////////////////////////////////////////////

void
exit_cleanup(void) // is called on exit() or if main() returns
{
#define M "exit_cleanup()"

    ////////////////////////
    // Finalization phase //
    ////////////////////////

    archi_log_info(M, "Finalizing the application...");

    // Decrement reference count of the signal management context
    decrement_refcount_of_signal_management();

    // Decrement reference counts of input file contexts
    decrement_refcount_of_input_files();

    // Destroy the context registry
    destroy_context_registry();

    // Finalization is done
    archi_log_info(M, "The application has exited successfully.");

#undef M
}

void
exit_quick(void) // is called on quick_exit()
{
#define M "exit_quick()"

    archi_log_info(M, "The application has exited without finalizing contexts and releasing resources.");

#undef M
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

    if (!archi_process.args.no_color)
        archi_print(0, ARCHI_COLOR_RESET);

    archi_print(0, "\n");

    for (int i = 0; i < LINES; i++)
    {
        archi_print(0, "  ");

        for (int j = 0; j < LETTERS; j++)
        {
            if (!archi_process.args.no_color)
                archi_print(0, "%s", colors[j]);

            archi_print(0, "%s%s", space, logo[i][j]);
        }

        if (!archi_process.args.no_color)
            archi_print(0, ARCHI_COLOR_RESET);

        archi_print(0, "\n");
    }

    archi_print(0, "\n\n");

    archi_print_unlock(0);

#undef LINES
#undef LETTERS
}

void
create_context_registry(void)
{
#define M "main@create_context_registry()"

    archi_log_debug(M, "Creating the context registry...");

    size_t hashmap_capacity = 1024;

    archi_parameter_list_t params[] = {
        {
            .name = "capacity",
            .value.ptr = &hashmap_capacity,
        },
    };

    archi_status_t code;

    archi_process.registry = archi_context_initialize(
            (archi_pointer_t){.ptr = (void*)&archi_context_ds_hashmap_interface},
            params, &code);

    if (archi_process.registry == NULL)
    {
        archi_log_error(M, "Couldn't create the context registry (error %i).", code);
        exit(EXIT_FAILURE);
    }

    archi_log_debug(M, "Inserting the context registry into itself...");

    code = archi_context_set_slot(archi_process.registry,
            (archi_context_op_designator_t){.name = ARCHI_EXE_REGISTRY_KEY_REGISTRY},
            (archi_pointer_t){
                .ptr = archi_process.registry,
                .ref_count = archi_context_data(archi_process.registry).ref_count,
                .element.num_of = 1,
            });

    if (code != 0)
    {
        archi_log_error(M, "Couldn't insert the context registry into itself (error %i).", code);
        exit(EXIT_FAILURE);
    }

#undef M
}

void
destroy_context_registry(void)
{
#define M "exit@destroy_context_registry()"

    if (archi_process.registry != NULL)
    {
        archi_log_debug(M, "Destroying the context registry...");

        archi_context_finalize(archi_process.registry);

        archi_process.registry = NULL;
    }

#undef M
}

void
create_hashmap_of_interfaces(void)
{
#define M "main@create_hashmap_of_interfaces()"

    archi_log_debug(M, "Creating the hashmap of built-in context interfaces...");

    if (archi_process.args.dry_run)
        return;

    size_t hashmap_capacity = 1024;

    archi_parameter_list_t params[] = {
        {
            .name = "capacity",
            .value.ptr = &hashmap_capacity,
        },
    };

    archi_status_t code;

    archi_process.interfaces = archi_context_initialize(
            (archi_pointer_t){.ptr = (void*)&archi_context_ds_hashmap_interface},
            params, &code);

    if (archi_process.interfaces == NULL)
    {
        archi_log_error(M, "Couldn't create the hashmap of built-in context interfaces (error %i).", code);
        exit(EXIT_FAILURE);
    }

    archi_log_debug(M, "Inserting the hashmap of interfaces into the registry...");

    // Insert the context into the registry, which also increments the reference count
    code = archi_context_set_slot(archi_process.registry,
            (archi_context_op_designator_t){.name = ARCHI_EXE_REGISTRY_KEY_INTERFACES},
            (archi_pointer_t){
                .ptr = archi_process.interfaces,
                .ref_count = archi_context_data(archi_process.interfaces).ref_count,
                .element.num_of = 1,
            });

    // Decrement the reference count back to 1
    archi_reference_count_decrement(archi_context_data(archi_process.interfaces).ref_count);

    // Reset the separate context pointer as it isn't needed anymore
    archi_process.interfaces = NULL;

    if (code != 0)
    {
        archi_log_error(M, "Couldn't insert the hashmap of interfaces into the registry (error %i).", code);
        exit(EXIT_FAILURE);
    }

    archi_log_debug(M, "Inserting built-in context interfaces into the hashmap...");

#define BUILTIN_INTERFACE(iface)  do {                                  \
    archi_log_debug(M, " + archi_context_" #iface "_interface");        \
    \
    code = archi_context_set_slot(archi_process.interfaces,             \
            (archi_context_op_designator_t){.name = #iface},            \
            (archi_pointer_t){                                          \
                .ptr = (void*)&archi_context_##iface##_interface,       \
                .element = {                                            \
                    .num_of = 1,                                        \
                    .size = sizeof(archi_context_interface_t),          \
                    .alignment = alignof(archi_context_interface_t),    \
                }});                                                    \
    \
    if (code != 0) {                                                    \
        archi_log_error(M, "Couldn't insert archi_context_" #iface      \
                "_interface into the hashmap of interfaces (error %i).", code); \
        exit(EXIT_FAILURE);                                             \
    } } while (0)

    // After modifying this list, don't forget to update the help message in `exe/args.c`

    BUILTIN_INTERFACE(parameters);
    BUILTIN_INTERFACE(pointer);

    BUILTIN_INTERFACE(memory);
    BUILTIN_INTERFACE(memory_mapping);

    BUILTIN_INTERFACE(hsp_state);
    BUILTIN_INTERFACE(hsp_transition);
    BUILTIN_INTERFACE(hsp);
    BUILTIN_INTERFACE(hsp_frame);
    BUILTIN_INTERFACE(hsp_branch_state_data);

    BUILTIN_INTERFACE(ds_hashmap);
    BUILTIN_INTERFACE(ds_lfqueue);

    BUILTIN_INTERFACE(ipc_env);
    BUILTIN_INTERFACE(ipc_signal_handler);

    BUILTIN_INTERFACE(res_file);
    BUILTIN_INTERFACE(res_library);
    BUILTIN_INTERFACE(res_thread_group);
    BUILTIN_INTERFACE(res_thread_group_work);
    BUILTIN_INTERFACE(res_thread_group_callback);
    BUILTIN_INTERFACE(res_thread_group_dispatch_data);

#undef BUILTIN_INTERFACE

#undef M
}

void
obtain_handle_of_executable(void)
{
#define M "main@obtain_handle_of_executable()"

    archi_log_debug(M, "Obtaining library handle of the executable itself...");

    if (archi_process.args.dry_run)
        return;

    archi_parameter_list_t params[] = {
        {
            .name = "pathname",
            // value.ptr is NULL
        },
    };

    archi_status_t code;

    archi_process.exe_handle = archi_context_initialize(
            (archi_pointer_t){.ptr = (void*)&archi_context_res_library_interface},
            params, &code);

    if (archi_process.exe_handle == NULL)
    {
        archi_log_error(M, "Couldn't obtain library handle of the executable itself (error %i).", code);
        exit(EXIT_FAILURE);
    }

    archi_log_debug(M, "Inserting handle of the executable into the registry...");

    // Insert the context into the registry, which also increments the reference count
    code = archi_context_set_slot(archi_process.registry,
            (archi_context_op_designator_t){.name = ARCHI_EXE_REGISTRY_KEY_EXE_HANDLE},
            (archi_pointer_t){
                .ptr = archi_process.exe_handle,
                .ref_count = archi_context_data(archi_process.exe_handle).ref_count,
                .element.num_of = 1,
            });

    // Decrement the reference count back to 1
    archi_reference_count_decrement(archi_context_data(archi_process.exe_handle).ref_count);

    // Reset the separate context pointer as it isn't needed anymore
    archi_process.exe_handle = NULL;

    if (code != 0)
    {
        archi_log_error(M, "Couldn't insert handle of the executable into the registry (error %i).", code);
        exit(EXIT_FAILURE);
    }

#undef M
}

void
open_and_map_input_files(void)
{
#define M "main@open_and_map_input_files()"

    archi_log_debug(M, "Allocating the array of input file contexts...");

    archi_process.input_file = malloc(sizeof(*archi_process.input_file) * archi_process.args.num_inputs);

    if (archi_process.input_file == NULL)
    {
        archi_log_error(M, "Couldn't allocate the array of input file contexts.");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        archi_process.input_file[i] = NULL;

    archi_log_debug(M, "Opening and mapping input files...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(M, " * opening file #%zu ('%s')", i, archi_process.args.input[i]);
        {
            bool value_true = true;

            archi_parameter_list_t params[] = {
                {
                    .name = "pathname",
                    .value = (archi_pointer_t){
                        .ptr = archi_process.args.input[i],
                    },
                },
                {
                    .name = "readable",
                    .value = (archi_pointer_t){
                        .ptr = &value_true,
                    },
                },
            };
            params[0].next = &params[1];

            archi_status_t code;

            archi_process.input_file[i] = archi_context_initialize(
                    (archi_pointer_t){.ptr = (void*)&archi_context_res_file_interface},
                    params, &code);

            if (archi_process.input_file[i] == NULL)
            {
                archi_log_error(M, "Couldn't open the input file (error %i).", code);
                exit(EXIT_FAILURE);
            }
        }

        archi_log_debug(M, " * mapping file #%zu", i);
        {
            bool value_true = true;

            archi_parameter_list_t params[] = {
                {
                    .name = "has_header",
                    .value = (archi_pointer_t){
                        .ptr = &value_true,
                    },
                },
                {
                    .name = "readable",
                    .value = (archi_pointer_t){
                        .ptr = &value_true,
                    },
                },
                {
                    .name = "close_fd",
                    .value = (archi_pointer_t){
                        .ptr = &value_true,
                    },
                },
            };
            params[0].next = &params[1];
            params[1].next = &params[2];

            archi_status_t code = archi_context_act(archi_process.input_file[i],
                    (archi_context_op_designator_t){.name = "map"},
                    params);

            if (code != 0)
            {
                archi_log_error(M, "Couldn't map the input file into memory (error %i).", code);
                exit(EXIT_FAILURE);
            }

            archi_pointer_t file = archi_context_data(archi_process.input_file[i]);

            if (file.element.num_of < sizeof(archi_exe_input_t))
            {
                archi_log_error(M, "Input file #%zu is invalid (file size is smaller than the header structure size).", i);
                exit(EXIT_FAILURE);
            }

            const archi_exe_input_t *input = file.ptr;
            if (strncmp(ARCHI_EXE_INPUT_MAGIC, input->magic, sizeof(input->magic)) != 0)
            {
                archi_log_error(M, "Input file #%zu is invalid (magic bytes are incorrect).", i);
                exit(EXIT_FAILURE);
            }
        }

        archi_log_debug(M, " * inserting file context #%zu into the registry...", i);
        {
            char file_context_key[64]; // should be enough
            snprintf(file_context_key, sizeof(file_context_key), "%s%zu", ARCHI_EXE_REGISTRY_KEY_INPUT_FILE, i);

            // Insert the context into the registry, which also increments the reference count
            archi_status_t code = archi_context_set_slot(archi_process.registry,
                    (archi_context_op_designator_t){.name = file_context_key},
                    (archi_pointer_t){
                        .ptr = archi_process.input_file[i],
                        .ref_count = archi_context_data(archi_process.input_file[i]).ref_count,
                        .element.num_of = 1,
                    });

            if (code != 0)
            {
                archi_log_error(M, "Couldn't insert file context #%zu into the registry (error %i).", i, code);
                exit(EXIT_FAILURE);
            }
        }
    }

#undef M
}

void
decrement_refcount_of_input_files(void)
{
#define M "exit@decrement_refcount_of_input_files()"

    archi_log_debug(M, "Decrementing reference counts of input file contexts...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        archi_reference_count_decrement(archi_context_data(archi_process.input_file[i]).ref_count);

    free(archi_process.input_file);
    archi_process.input_file = NULL;

#undef M
}

void
process_parameters_of_input_files(void)
{
#define M "main@process_parameters_of_input_files()"

    archi_log_debug(M, "Processing parameter lists of input files...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(M, " * file #%zu ('%s')", i, archi_process.args.input[i]);

        const archi_exe_input_t *input = archi_context_data(archi_process.input_file[i]).ptr;

        for (archi_parameter_list_t *params = input->params; params != NULL; params = params->next)
        {
            if (strcmp("signals", params->name) == 0)
            {
                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (params->value.ptr == NULL))
                {
                    archi_log_error(M, "Parameter '%s' has invalid value.", params->name);
                    exit(EXIT_FAILURE);
                }

                archi_signal_watch_set_t *signal_watch_set = params->value.ptr;
                archi_signal_watch_set_join(archi_process.signal_watch_set, signal_watch_set);

                {
                    archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG);
                    archi_print(ARCHI_LOG_VERBOSITY_DEBUG, ARCHI_LOG_INDENT "  signals =");
                    print_signal_watch_set(ARCHI_LOG_VERBOSITY_DEBUG, signal_watch_set);
                    archi_print(ARCHI_LOG_VERBOSITY_DEBUG, "\n");
                    archi_print_unlock(ARCHI_LOG_VERBOSITY_DEBUG);
                }
            }
            else
            {
                archi_log_error(M, "Met unrecognized parameter '%s'.", params->name);
                exit(EXIT_FAILURE);
            }
        }
    }

#undef M
}

void
prepare_signal_management(void)
{
#define M "main@prepare_signal_management()"

    archi_process.signal_interface = (archi_context_interface_t){
        .init_fn = archi_context_ipc_signal_management_init,
        .final_fn = archi_context_ipc_signal_management_final,
        .get_fn = archi_context_ipc_signal_management_get,
        .set_fn = archi_context_ipc_signal_management_set,
    };

    archi_log_debug(M, "Allocating the signal watch set...");

    archi_process.signal_watch_set = archi_signal_watch_set_alloc();

    if (archi_process.signal_watch_set == NULL)
    {
        archi_log_error(M, "Couldn't allocate the signal watch set.");
        exit(EXIT_FAILURE);
    }

#undef M
}

void
start_signal_management(void)
{
#define M "main@start_signal_management()"

    if (archi_signal_watch_set_not_empty(archi_process.signal_watch_set))
    {
        archi_log_debug(M, "Creating the signal management context...");

        {
            archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG);
            archi_print(ARCHI_LOG_VERBOSITY_DEBUG, ARCHI_LOG_INDENT "  signals =");
            print_signal_watch_set(ARCHI_LOG_VERBOSITY_DEBUG, archi_process.signal_watch_set);
            archi_print(ARCHI_LOG_VERBOSITY_DEBUG, "\n");
            archi_print_unlock(ARCHI_LOG_VERBOSITY_DEBUG);
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
            archi_log_error(M, "Couldn't create the signal management context (error %i).", code);
            exit(EXIT_FAILURE);
        }

        archi_log_debug(M, "Inserting the signal management context into the registry...");

        // Insert the context into the registry, which also increments the reference count
        code = archi_context_set_slot(archi_process.registry,
                (archi_context_op_designator_t){.name = ARCHI_EXE_REGISTRY_KEY_SIGNAL},
                (archi_pointer_t){
                    .ptr = archi_process.signal,
                    .ref_count = archi_context_data(archi_process.signal).ref_count,
                    .element.num_of = 1,
                });

        if (code != 0)
        {
            archi_log_error(M, "Couldn't insert the signal management context into the registry (error %i).", code);
            exit(EXIT_FAILURE);
        }

    }
    else
        archi_log_debug(M, "No signals in the watch set, skipping the signal management context creation.");

    free(archi_process.signal_watch_set);
    archi_process.signal_watch_set = NULL;

#undef M
}

void
decrement_refcount_of_signal_management(void)
{
#define M "exit@decrement_refcount_of_signal_management()"

    if (archi_process.signal != NULL)
    {
        archi_log_debug(M, "Decrementing reference count of the signal management context...");

        archi_reference_count_decrement(archi_context_data(archi_process.signal).ref_count);
    }

    archi_process.signal = NULL;
    archi_process.signal_interface = (archi_context_interface_t){0};

#undef M
}

void
execute_instructions(void)
{
#define M "main@execute_instructions()"

    archi_log_debug(M, "Executing instructions in input files...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(M, " * executing file #%zu ('%s')", i, archi_process.args.input[i]);

        const archi_exe_input_t *input = archi_context_data(archi_process.input_file[i]).ptr;

        for (archi_exe_registry_instr_list_t *instr_list = input->instructions;
                instr_list != NULL; instr_list = instr_list->next)
        {
            archi_status_t code = archi_exe_registry_instr_execute(archi_process.registry,
                    instr_list->instruction, archi_process.args.dry_run);

            if (code > 0)
                archi_log_warning(M, "Got non-zero instruction execution status %i, attempting to continue...", code);
            else if (code < 0)
            {
                archi_log_error(M, "Couldn't execute the instruction (error %i).", code);
                exit(EXIT_FAILURE);
            }
        }
    }

#undef M
}

///////////////////////////////////////////////////////////////////////////////

void
print_signal_watch_set(
        int verbosity,
        const archi_signal_watch_set_t *signal_watch_set)
{
    if (archi_log_verbosity() < verbosity)
        return;

#define PRINT_SIGNAL(signal) do { \
        if (signal_watch_set->f_##signal) \
            archi_print(verbosity, " %s", #signal); \
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
            archi_print(verbosity, " SIGRTMIN+%zu", i);
    }
}

