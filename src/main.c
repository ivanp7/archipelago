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

// Contexts and operations
#include "archi_exe/registry.def.h"
#include "archi/context/api/interface.fun.h"
#include "archi/context/api/callback.fun.h"
#include "archi/context/api/operation.typ.h"
#include "archi/context/ctx-op/registry.fun.h"
#include "archi/context/ctx-op/registry.typ.h"

// Hashmaps
#include "archi/hashmap/ctx/hashmap.var.h"

// Files
#include "archi_exe/input_file.typ.h"
#include "archi_exe/input_file.def.h"
#include "archi/file/api/file.fun.h"
#include "archi/file/ctx/mapping.var.h"

// Shared libraries
#include "archi/library/ctx/library.var.h"

// Logging
#include "archi_exe/logging.fun.h"
#include "archipelago/log/context.fun.h"
#include "archipelago/log/print.fun.h"
#include "archipelago/log/verbosity.def.h"
#include "archipelago/log/color.def.h"

// Signal management
#include "archi_exe/signal_management.fun.h"
#include "archi/signal/api/management.fun.h"
#include "archi/signal/api/signal.fun.h"
#include "archi/signal/sig/hashmap.fun.h"
#include "archi/signal/sig-ctx/hashmap_data.var.h"

// Environment
#include "archi_exe/environment.fun.h"
#include "archi/env/api/context.fun.h"

// Reference counting
#include "archipelago/base/ref_count.fun.h"

// Error handling
#include "archipelago/base/error.fun.h"

// Utilities
#include "archipelago/base/pointer.def.h"
#include "archipelago/base/kvlist.fun.h"
#include "archipelago/util/string.fun.h"

// System utilities
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Command line arguments parsing
#include <argp.h> // for command line arguments parsing

///////////////////////////////////////////////////////////////////////////////

#define INIT_ERROR()    do {                                                    \
    ARCHI_ERROR_SET_VAR(&archi_process.error, 0, "error code hasn't been set"); \
} while (0)

#define LINK_KVLIST_NODES(array)    do {                            \
    for (size_t i = 0; i < ARCHI_LENGTH_ARRAY((array)) - 1; i++)    \
        (array)[i].next = &(array)[i + 1];                          \
} while (0)

///////////////////////////////////////////////////////////////////////////////

struct archi_exe_args {
    // Functionality options
    char **input;      ///< Array of pathnames of input memory-mapped initialization files.
    size_t num_inputs; ///< Number of input initialization files.

    bool dry_run; ///< Whether dry run is done: initialization instructions are logged only, not executed.

    // Logging options
    bool no_logo;  ///< True if the application logo is not displayed.
    bool no_color; ///< True if color codes are not used when printing log messages.
    int verbosity_level; ///< Logging verbosity level.
};

static
struct {
    struct archi_exe_args args; ///< Command line arguments.

    struct {
        archi_context_t registry;   ///< The context registry.
        archi_context_t operations; ///< The context registry operations.

        archi_context_t executable;  ///< The library handle of the executable itself.
        archi_context_t *input_file; ///< Array of the input file contexts.

        archi_context_t signal_handler_data; ///< The signal handler data.
    } context; ///< Built-in contexts.

    struct {
        archi_signal_watch_set_t *watch_set; ///< The signal watch set.
        archi_signal_handler_t handler;      ///< The signal handler.
    } signal;

    archi_error_t error; ///< The process error.
} archi_process; ///< The process data.

///////////////////////////////////////////////////////////////////////////////

static
void
parse_args(
        int argc,
        char *argv[]
);

static
void
print_error(void);

static
archi_context_t
initialize_context(
        archi_context_interface_t context_interface,
        const archi_kvlist_t *params
);

static
void
add_registry_context(
        archi_context_t context,
        const char *key
);

static
void
add_registry_operation(
        archi_context_operation_func_t operation,
        const char *key
);

///////////////////////////////////////////////////////////////////////////////

static
void
initialize_global_contexts(void);

static
void
open_and_map_input_files(void);

static
void
initialize_signal_management(void);

static
void
initialize_builtin_contexts(void);

static
void
add_builtin_registry_contexts(void);

static
void
add_builtin_registry_operations(void);

///////////////////////////////////////////////////////////////////////////////

static
void
finalize_signal_management(void);

static
void
finalize_builtin_contexts(void);

static
void
finalize_global_contexts(void);

///////////////////////////////////////////////////////////////////////////////

static
void
execute(void);

static
void
exit_clean(void);

static
void
exit_quick(void);

static
void
print_logo(void)
{
#define LINES_1      5
#define COLUMNS_1    5

#define LINES_2      4
#define COLUMNS_2    6

    static const char *logo1[LINES_1][COLUMNS_1] = {
        {"    ", "    ", " ⡔⠢ ", "    ", "    "},
        {"    ", " ⣏⡱ ", " ⢇⡠ ", " ⣇⣸ ", "    "},
        {" ⡔⢢ ", " ⠇⠱ ", "    ", " ⠇⠸ ", " ⢰  "},
        {" ⡏⢹ ", "   ⣠", "⡾⠛⠛⢷", "⣄   ", " ⢸  "},
        {"    ", "  ⢾⣏", " ⢾⡷ ", "⣹⡷  ", "    "},
    };
    static const char *logo2[LINES_2][COLUMNS_2] = {
        {"⢸⣉⠆", "   ", "⠙⢷⣤", "⣤⡾⠋", "   ", "⢰⠉⡆"},
        {"⠸  ", "⢸⣉⠁", "   ", "   ", "⢰⠉⠂", "⠘⠤⠃"},
        {"   ", "⠸⠤⠄", "⢸  ", "⢰⣉⡆", "⠘⠬⠃", "   "},
        {"   ", "   ", "⠸⠤⠄", "⠸ ⠇", "   ", "   "},
    };

    static const char *padding1 = "";
    static const char *padding2 = " ";

#define RED     ARCHI_COLOR_FG(204)
#define YELLOW  ARCHI_COLOR_FG(223)
#define GREEN   ARCHI_COLOR_FG(116)
#define BLUE    ARCHI_COLOR_FG(68)
#define PURPLE  ARCHI_COLOR_FG(140)

#define GRAY1   ARCHI_COLOR_FG(242)
#define GRAY2   ARCHI_COLOR_FG(244)
#define GRAY3   ARCHI_COLOR_FG(246)
#define GRAY4   ARCHI_COLOR_FG(248)

#define EYE0    ARCHI_COLOR_FG(245)
#define EYE1    ARCHI_COLOR_FG(255)

    static const char *colors1[LINES_1][COLUMNS_1] = {
        {RED, YELLOW, GREEN, BLUE, PURPLE},
        {RED, YELLOW, GREEN, BLUE, PURPLE},
        {RED, YELLOW, GREEN, BLUE, PURPLE},
        {RED, EYE0, EYE0, EYE0, PURPLE},
        {RED, EYE0, EYE1, EYE0, PURPLE},
    };
    static const char *colors2[LINES_2][COLUMNS_2] = {
        {GRAY4, EYE0, EYE0, EYE0, EYE0, GRAY4},
        {GRAY3, GRAY3, GRAY3, GRAY3, GRAY3, GRAY3},
        {GRAY2, GRAY2, GRAY2, GRAY2, GRAY2, GRAY2},
        {GRAY1, GRAY1, GRAY1, GRAY1, GRAY1, GRAY1},
    };

    static const char *indent = "   ";

    if (archi_print_lock(0))
    {
        archi_print_color(ARCHI_COLOR_RESET);
        archi_print("\n");

        for (int i = 0; i < LINES_1; i++)
        {
            archi_print("%s%s", indent, padding1);

            for (int j = 0; j < COLUMNS_1; j++)
            {
                archi_print_color(colors1[i][j]);
                archi_print("%s", logo1[i][j]);
            }

            archi_print_color(ARCHI_COLOR_RESET);
            archi_print("\n");
        }

        for (int i = 0; i < LINES_2; i++)
        {
            archi_print("%s%s", indent, padding2);

            for (int j = 0; j < COLUMNS_2; j++)
            {
                archi_print_color(colors2[i][j]);
                archi_print("%s", logo2[i][j]);
            }

            archi_print_color(ARCHI_COLOR_RESET);
            archi_print("\n");
        }

        archi_print("\n");

        archi_print_unlock();
    }

#undef RED
#undef YELLOW
#undef GREEN
#undef BLUE
#undef PURPLE

#undef GRAY1
#undef GRAY2
#undef GRAY3
#undef GRAY4

#undef EYE0
#undef EYE1

#undef LINES_1
#undef COLUMNS_1
#undef LINES_2
#undef COLUMNS_2
}

int
main(
        int argc,
        char *argv[])
{
    /////////////////////////////////////////
    // Command line argument parsing phase //
    /////////////////////////////////////////

    parse_args(argc, argv);

    ///////////////////////////////
    // Early bootstrapping phase //
    ///////////////////////////////

    // Initialize logging
    {
        if (!archi_exe_log_initialize(NULL, archi_process.args.verbosity_level, !archi_process.args.no_color))
        {
            fprintf(stderr, "Error: couldn't initialize logging.\n");
            return EXIT_FAILURE;
        }

        archi_log_global_context_set(archi_exe_log_global_context());
    }

    // Install exit functions
    {
        if (atexit(exit_clean) != 0)
        {
            archi_log_error(__func__, "Error: atexit(exit_clean) failed.");
            archi_exe_log_finalize();
            return EXIT_FAILURE;
        }

        if (at_quick_exit(exit_quick) != 0)
            archi_log_warning(__func__, "at_quick_exit(exit_quick) failed, attempting to continue...");
    }

    // Display the logo
    if (!archi_process.args.no_logo)
        print_logo();

    // Exit if there is nothing to do
    if (archi_process.args.num_inputs == 0)
        return EXIT_SUCCESS;

    //////////////////////////////
    // Late bootstrapping phase //
    //////////////////////////////

    archi_log_info(__func__, "Bootstrapping the application...");

    initialize_global_contexts();
    open_and_map_input_files();
    initialize_signal_management();
    initialize_builtin_contexts();
    add_builtin_registry_contexts();
    add_builtin_registry_operations();

    /////////////////////
    // Execution phase //
    /////////////////////

    execute();

    return EXIT_SUCCESS;
}

static
const archi_kvlist_t*
input_file_operations(
        size_t index)
{
    archi_pointer_t input_file = archi_context_data(archi_process.context.input_file[index]);
    const archi_exe_input_file_header_t *input_file_header = input_file.ptr;

    for (const archi_kvlist_t *contents = input_file_header->contents; contents != NULL; contents = contents->next)
    {
        if (ARCHI_STRING_COMPARE(ARCHI_EXE_INPUT_FILE_CONTENTS__OPERATIONS, ==, contents->key))
        {
            if (!ARCHI_POINTER_TO_DATA_TYPE(contents->value, 1, archi_kvlist_t))
            {
                archi_log_warning(__func__, "Pointer to operations list is NULL or has incorrect attributes, ignoring...");
                return NULL;
            }

            return contents->value.ptr;
        }
    }

    return NULL;
}

static
void
execute_operation(
        const archi_kvlist_t *operation,
        archi_reference_count_t ref_count)
{
    // Extract operation function pointer
    archi_pointer_t operation_ptr = {0};

    INIT_ERROR();
    archi_context_get_slot(archi_process.context.operations, (archi_context_slot_t){.name = operation->key},
            (archi_context_callback_t){.function = archi_context_callback__getter, .data = &operation_ptr},
            &archi_process.error);
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);

    // Check operation function pointer attributes
    if (!ARCHI_POINTER_TO_FUNCTION_TYPE(operation_ptr, false, ARCHI_POINTER_FTAG__CONTEXT_OPERATION))
    {
        archi_log_error(__func__, "Error: registry operation pointer is NULL or has incorrect attributes.");
        exit(EXIT_FAILURE);
    }

    // Prepare operation function arguments
    archi_context_operation_func_t operation_fn = (archi_context_operation_func_t)operation_ptr.fptr;
    archi_pointer_t operation_arg = operation->value;
    operation_arg.ref_count = ref_count;

    // Print the operation function data
    if (archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG))
    {
        INIT_ERROR();
        /******************************************************/
        operation_fn(NULL, operation_arg, &archi_process.error);
        /******************************************************/
        print_error();

        if (archi_process.error.code != 0)
            exit(EXIT_FAILURE);

        archi_print_unlock();
    }

    // Do the operation
    INIT_ERROR();
    /********************************************************************************/
    operation_fn(archi_process.context.registry, operation_arg, &archi_process.error);
    /********************************************************************************/
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);
}

void
execute(void)
{
    archi_log_info(__func__, "Executing the application...");

    size_t operation_number_total = 0;
    size_t operation_number_in_file;

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        // Extract list of operations from the input file
        archi_log_debug(__func__, " * extracting operations from file #%zu...", i);
        const archi_kvlist_t *operations = input_file_operations(i);
        if (operations == NULL)
            continue;

        // Add the input file context to the registry
        archi_log_debug(__func__, " * adding file mapping context #%zu to the registry...", i);
        add_registry_context(archi_process.context.input_file[i], ARCHI_EXE_REGISTRY_CONTEXT__INPUT_FILE);

        archi_reference_count_t ref_count = archi_context_data(archi_process.context.input_file[i]).ref_count;

        // Executing the operations
        archi_log_debug(__func__, " * executing file #%zu ('%s')...", i, archi_process.args.input[i]);

        operation_number_in_file = 0;
        for (; operations != NULL; operations = operations->next)
        {
            operation_number_total++;
            operation_number_in_file++;

            if (operations->key == NULL)
            {
                archi_log_debug(__func__, "   * no operation #%zu (%zu)",
                        operation_number_in_file, operation_number_total);
                continue;
            }

            archi_log_debug(__func__, "   * operation #%zu (%zu): '%s'",
                    operation_number_in_file, operation_number_total, operations->key);

            execute_operation(operations, ref_count);
        }

        // Forget the input file
        archi_reference_count_decrement(ref_count);
        archi_process.context.input_file[i] = NULL;
    }
}

void
exit_clean(void) // is called on exit() or if main() returns
{
    ////////////////////////
    // Finalization phase //
    ////////////////////////

    archi_log_info(__func__, "Shutting down the application...");

    finalize_signal_management();
    finalize_builtin_contexts();
    finalize_global_contexts();

    archi_log_info(__func__, "The application has exited normally.");

    archi_exe_log_finalize();
}

void
exit_quick(void) // is called on quick_exit()
{
    archi_log_info(__func__, "The application has exited without the finalization procedures.");
}

///////////////////////////////////////////////////////////////////////////////

void
print_error(void)
{
#define ERROR_ORIGIN_SIZE 1024

    char error_origin[ERROR_ORIGIN_SIZE];

    if ((archi_process.error.origin.func != NULL) && (archi_process.error.origin.file != NULL))
        snprintf(error_origin, ERROR_ORIGIN_SIZE, "%s()@'%s':%i",
                archi_process.error.origin.func,
                archi_process.error.origin.file,
                archi_process.error.origin.line);
    else if (archi_process.error.origin.func != NULL)
        snprintf(error_origin, ERROR_ORIGIN_SIZE, "%s()",
                archi_process.error.origin.func);
    else if (archi_process.error.origin.file != NULL)
        snprintf(error_origin, ERROR_ORIGIN_SIZE, "'%s':%i",
                archi_process.error.origin.file,
                archi_process.error.origin.line);
    else
        error_origin[0] = '\0';

#undef ERROR_ORIGIN_SIZE

    if (archi_process.error.code != 0)
    {
        const char *code_name = archi_error_code_string(archi_process.error.code);

        if (code_name != NULL)
        {
            if (archi_process.error.message[0] != '\0')
                archi_log_error(error_origin, "Error code %i (%s): %s",
                        archi_process.error.code, code_name, archi_process.error.message);
            else
                archi_log_error(error_origin, "Error code %i (%s)",
                        archi_process.error.code, code_name);
        }
        else
        {
            if (archi_process.error.message[0] != '\0')
                archi_log_error(error_origin, "Error code %i: %s",
                        archi_process.error.code, archi_process.error.message);
            else
                archi_log_error(error_origin, "Error code %i",
                        archi_process.error.code);
        }
    }
    else if (archi_process.error.message[0] != '\0')
        archi_log_warning(error_origin, "%s", archi_process.error.message);
}

archi_context_t
initialize_context(
        archi_context_interface_t context_interface,
        const archi_kvlist_t *params)
{
    archi_pointer_t context_interface_ptr = {
        .ptr = (void*)&context_interface,
        .attr = ARCHI_POINTER_ATTRIBUTES(ARCHI_POINTER_TYPE__STACK_MEM,
                ARCHI_ALIGNOF_LOG2(archi_context_interface_t), 0),
        .length = 1,
        .stride = sizeof(archi_context_interface_t),
    };

    INIT_ERROR();
    archi_context_t context = archi_context_initialize(
            context_interface_ptr, params, &archi_process.error);
    print_error();

    if (context == NULL)
        exit(EXIT_FAILURE);

    return context;
}

void
add_registry_context(
        archi_context_t context,
        const char *key)
{
    archi_pointer_t context_ptr = {
        .ptr = context,
        .attr = ARCHI_POINTER_ATTRIBUTES(ARCHI_POINTER_TYPE__READONLY_MEM, 0, 0),
        .length = 1,
        // opaque object
    };

    INIT_ERROR();
    archi_context_set_slot(archi_process.context.registry, (archi_context_slot_t){.name = key},
            context_ptr, &archi_process.error);
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);
}

void
add_registry_operation(
        archi_context_operation_func_t operation,
        const char *key)
{
    archi_pointer_t operation_ptr = {
        .fptr = (archi_function_t)operation,
        .attr = ARCHI_POINTER_ATTRIBUTES(ARCHI_POINTER_TYPE__FUNCTION,
                ARCHI_POINTER_FTAG__CONTEXT_OPERATION, 0),
    };

    INIT_ERROR();
    archi_context_set_slot(archi_process.context.operations, (archi_context_slot_t){.name = key},
            operation_ptr, &archi_process.error);
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);
}

///////////////////////////////////////////////////////////////////////////////

void
initialize_global_contexts(void)
{
    // Initialize environment context
    archi_log_debug(__func__, "Initializing environment context...");

    if (!archi_exe_env_initialize())
    {
        archi_log_error(__func__, "Error: couldn't initialize environment context.");
        exit(EXIT_FAILURE);
    }

    archi_env_global_context_set(archi_exe_env_global_context());
}

static
archi_file_descriptor_t
open_input_file(
        size_t index)
{
    archi_file_open_params_t file_open_params = {
        .readable = true,
    };

    INIT_ERROR();
    archi_file_descriptor_t fd = archi_file_open(
            archi_process.args.input[index], file_open_params,
            &archi_process.error);
    print_error();

    if (fd < 0)
        exit(EXIT_FAILURE);

    return fd;
}

static
void
map_input_file(
        size_t index,
        archi_file_descriptor_t fd)
{
    // Prepare context initialization parameters
    const char value_true_var = true;

    archi_pointer_t value_true = {
        .ptr = (void*)&value_true_var,
        .attr = ARCHI_POINTER_ATTRIBUTES(ARCHI_POINTER_TYPE__STACK_MEM,
                ARCHI_ALIGNOF_LOG2(char), 0),
        .length = 1,
        .stride = sizeof(char),
    };

    archi_kvlist_t params[] = {
        {
            .key = "fd",
            .value = {
                .ptr = &fd,
                .attr = ARCHI_POINTER_ATTRIBUTES(ARCHI_POINTER_TYPE__STACK_MEM,
                        ARCHI_ALIGNOF_LOG2(archi_file_descriptor_t), 0),
                .length = 1,
                .stride = sizeof(archi_file_descriptor_t),
            },
        },
        {
            .key = "ptr_support",
            .value = value_true,
        },
        {
            .key = "readable",
            .value = value_true,
        },
    };
    LINK_KVLIST_NODES(params);

    // Initialize a file mapping context
    archi_pointer_t context_interface = {
        .ptr = (void*)&archi_context_file_mapping_interface,
        .attr = ARCHI_POINTER_ATTRIBUTES(ARCHI_POINTER_TYPE__READONLY_MEM,
                ARCHI_ALIGNOF_LOG2(archi_context_interface_t), 0),
        .length = 1,
        .stride = sizeof(archi_context_interface_t),
    };

    INIT_ERROR();
    archi_process.context.input_file[index] = archi_context_initialize(
            context_interface, params, &archi_process.error);
    print_error();

    if (archi_process.context.input_file[index] == NULL)
    {
        archi_file_close(fd, NULL); // this line prevents from utilizing initialize_context() here
        exit(EXIT_FAILURE);
    }

    // Close file descriptor
    INIT_ERROR();
    archi_file_close(fd, &archi_process.error);
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);
}

static
void
check_input_file_format(
        archi_pointer_t input_file)
{
    if (input_file.length < sizeof(archi_exe_input_file_header_t))
    {
        archi_log_error(__func__, "Error: input file is too short (file header won't fit).");
        exit(EXIT_FAILURE);
    }

    const archi_exe_input_file_header_t *input_file_header = input_file.ptr;

    if (strncmp(ARCHI_EXE_INPUT_FILE_MAGIC, input_file_header->magic, sizeof(input_file_header->magic)) != 0)
    {
        archi_log_error(__func__, "Error: input file is invalid (magic bytes are incorrect).");
        exit(EXIT_FAILURE);
    }
}

void
open_and_map_input_files(void)
{
    // Allocate and initialize the array of input file contexts
    archi_log_debug(__func__, "Allocating the array of input file contexts...");

    archi_process.context.input_file = malloc(
            sizeof(*archi_process.context.input_file) * archi_process.args.num_inputs);
    if (archi_process.context.input_file == NULL)
    {
        archi_log_error(__func__, "Error: couldn't allocate the array of input file contexts.");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        archi_process.context.input_file[i] = NULL;

    archi_log_debug(__func__, "Opening and mapping input files...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        // Open input file
        archi_log_debug(__func__, " * opening file #%zu ('%s')", i, archi_process.args.input[i]);
        archi_file_descriptor_t fd = open_input_file(i);

        // Map input file to memory, creating a context
        archi_log_debug(__func__, " * mapping file #%zu ('%s')", i, archi_process.args.input[i]);
        map_input_file(i, fd);

        archi_pointer_t input_file = archi_context_data(archi_process.context.input_file[i]);

        // Check input file format
        archi_log_debug(__func__, " * checking format of file #%zu ('%s')", i, archi_process.args.input[i]);
        check_input_file_format(input_file);

        // Log file address & size
        const archi_exe_input_file_header_t *input_file_header = input_file.ptr;
        archi_log_debug(__func__, "\taddress = %p, end = %p, size = %tu B (%.2f KiB, %.2f MiB, %.2f GiB)",
                input_file_header->header.addr, input_file_header->header.end, input_file.length,
                input_file.length / 1024.0, input_file.length / (1024.0 * 1024.0),
                input_file.length / (1024.0 * 1024.0 * 1024.0));
    }
}

static
void
print_signal_watch_set(
        const archi_signal_watch_set_t *signal_watch_set)
{
    if (archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG))
    {
        archi_print("\t");

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

        archi_print_unlock();
    }
}

static
archi_signal_watch_set_t*
extract_input_signal_watch_set(
        size_t index)
{
    archi_pointer_t input_file = archi_context_data(archi_process.context.input_file[index]);
    const archi_exe_input_file_header_t *input_file_header = input_file.ptr;

    archi_signal_watch_set_t *signal_watch_set = NULL;

    for (const archi_kvlist_t *node = input_file_header->contents; node != NULL; node = node->next)
    {
        if (ARCHI_STRING_COMPARE(ARCHI_EXE_INPUT_FILE_CONTENTS__SIGNALS, ==, node->key))
        {
            if (ARCHI_POINTER_TO_DATA(node->value, 1, ARCHI_SIGNAL_WATCH_SET_SIZEOF, alignof(archi_signal_watch_set_t)))
                signal_watch_set = node->value.ptr;
            else
                archi_log_warning(__func__, "Signal watch set pointer has incorrect attributes, ignoring...");

            break;
        }
    }

    return signal_watch_set;
}

static
bool
prepare_signal_watch_set(void)
{
    // Allocate signal watch set
    archi_log_debug(__func__, "Allocating the signal watch set...");

    archi_process.signal.watch_set = archi_signal_watch_set_alloc();
    if (archi_process.signal.watch_set == NULL)
    {
        archi_log_error(__func__, "Error: couldn't allocate the signal watch set.");
        exit(EXIT_FAILURE);
    }

    // Join partial watch sets into one
    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(__func__, " * extracting signal watch set from file #%zu ('%s')", i, archi_process.args.input[i]);
        archi_signal_watch_set_t *signal_watch_set = extract_input_signal_watch_set(i);
        print_signal_watch_set(signal_watch_set);

        archi_signal_watch_set_join(archi_process.signal.watch_set, signal_watch_set);
    }

    // Log the resulting signal watch set
    if (archi_signal_watch_set_not_empty(archi_process.signal.watch_set))
    {
        archi_log_debug(__func__, "Signals to watch:");
        print_signal_watch_set(archi_process.signal.watch_set);
        return true;
    }
    else
    {
        archi_log_debug(__func__, "No signals to watch.");
        return false;
    }
}

void
initialize_signal_management(void)
{
    // Prepare signal watch set
    if (!prepare_signal_watch_set())
        return;

    // Initialize the signal handler data context
    archi_log_debug(__func__, "Initializing the signal handler data context...");
    archi_process.context.signal_handler_data = initialize_context(
            archi_context_signal_handler_hashmap_data_interface, NULL);

    // Start the signal management thread
    archi_log_debug(__func__, "Starting the signal management thread...");

    archi_process.signal.handler.function = archi_signal_handler_hashmap;
    archi_process.signal.handler.data = archi_context_data(archi_process.context.signal_handler_data).ptr;

    INIT_ERROR();
    archi_exe_signal_management_start(archi_process.signal.watch_set, archi_process.signal.handler,
                &archi_process.error);
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);

    archi_signal_management_global_context_set(archi_exe_signal_management_global_context());
}

void
initialize_builtin_contexts(void)
{
    archi_log_debug(__func__, "Initializing the registry context...");
    archi_process.context.registry = initialize_context(archi_context_hashmap_interface, NULL);

    archi_log_debug(__func__, "Initializing the registry operations context...");
    archi_process.context.operations = initialize_context(archi_context_hashmap_interface, NULL);

    archi_log_debug(__func__, "Initializing the library handle context of the executable...");
    archi_process.context.executable = initialize_context(archi_context_library_interface, NULL);
}

void
add_builtin_registry_contexts(void)
{
    archi_log_debug(__func__, "Registering built-in registry contexts...");

#define ADD(context, key)   do {                \
    archi_log_debug(__func__, " * '%s'", key);  \
    add_registry_context(context, key);         \
} while (0)

    {
        // Insert the registry context into itself, which increments reference counter
        ADD(archi_process.context.registry, ARCHI_EXE_REGISTRY_CONTEXT__REGISTRY);

        // Decrement reference counter of the registry to prevent cyclic dependency
        archi_reference_count_decrement(archi_context_data(archi_process.context.registry).ref_count);
    }

    ADD(archi_process.context.operations, ARCHI_EXE_REGISTRY_CONTEXT__OPERATIONS);
    ADD(archi_process.context.executable, ARCHI_EXE_REGISTRY_CONTEXT__EXECUTABLE);

    ADD(archi_process.context.signal_handler_data, ARCHI_EXE_REGISTRY_CONTEXT__SIGNAL_HANDLER);

#undef ADD
}

void
add_builtin_registry_operations(void)
{
    archi_log_debug(__func__, "Registering built-in registry operations...");

#define ADD(func, key) do {                                 \
    archi_log_debug(__func__, " * '%s': " #func "()", key); \
    add_registry_operation(func, key);                      \
} while (0)

    ADD(archi_context_registry_op__delete, ARCHI_EXE_REGISTRY_OPERATION__DELETE);
    ADD(archi_context_registry_op__alias, ARCHI_EXE_REGISTRY_OPERATION__ALIAS);
    ADD(archi_context_registry_op__create_as, ARCHI_EXE_REGISTRY_OPERATION__CREATE_AS);
    ADD(archi_context_registry_op__create_from, ARCHI_EXE_REGISTRY_OPERATION__CREATE_FROM);
    ADD(archi_context_registry_op__create_parameters, ARCHI_EXE_REGISTRY_OPERATION__CREATE_PARAMETERS);
    ADD(archi_context_registry_op__create_pointer_to_value, ARCHI_EXE_REGISTRY_OPERATION__CREATE_POINTER_TO_VALUE);
    ADD(archi_context_registry_op__create_pointer_to_context, ARCHI_EXE_REGISTRY_OPERATION__CREATE_POINTER_TO_CONTEXT);
    ADD(archi_context_registry_op__create_dptr_array, ARCHI_EXE_REGISTRY_OPERATION__CREATE_DPTR_ARRAY);
    ADD(archi_context_registry_op__set, ARCHI_EXE_REGISTRY_OPERATION__SET);
    ADD(archi_context_registry_op__assign, ARCHI_EXE_REGISTRY_OPERATION__ASSIGN);
    ADD(archi_context_registry_op__act, ARCHI_EXE_REGISTRY_OPERATION__ACT);
    ADD(archi_context_registry_op__act_and_assign, ARCHI_EXE_REGISTRY_OPERATION__ACT_AND_ASSIGN);

#undef ADD
}

///////////////////////////////////////////////////////////////////////////////

void
finalize_signal_management(void)
{
    if (!archi_signal_watch_set_not_empty(archi_process.signal.watch_set))
        return;

    archi_log_debug(__func__, "Finalizing signal management...");

    archi_exe_signal_management_stop();
    free(archi_process.signal.watch_set);

    archi_process.signal.watch_set = NULL;
    archi_process.signal.handler = (archi_signal_handler_t){0};
}

void
finalize_builtin_contexts(void)
{
    archi_log_debug(__func__, "Finalizing built-in contexts...");

    if (archi_process.context.registry != NULL)
    {
        archi_log_debug(__func__, " * context registry");
        archi_reference_count_decrement(archi_context_data(archi_process.context.registry).ref_count);
        archi_process.context.registry = NULL;
    }

    if (archi_process.context.operations != NULL)
    {
        archi_log_debug(__func__, " * context operations");
        archi_reference_count_decrement(archi_context_data(archi_process.context.operations).ref_count);
        archi_process.context.operations = NULL;
    }

    if (archi_process.context.executable != NULL)
    {
        archi_log_debug(__func__, " * library handle of the executable");
        archi_reference_count_decrement(archi_context_data(archi_process.context.executable).ref_count);
        archi_process.context.executable = NULL;
    }

    if (archi_process.context.input_file != NULL)
    {
        for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        {
            archi_log_debug(__func__, " * mapping of the input file #%zu", i);
            archi_reference_count_decrement(archi_context_data(archi_process.context.input_file[i]).ref_count);
        }

        free(archi_process.context.input_file);
        archi_process.context.input_file = NULL;
    }

    if (archi_process.context.signal_handler_data != NULL)
    {
        archi_log_debug(__func__, " * signal handler data");
        archi_reference_count_decrement(archi_context_data(archi_process.context.signal_handler_data).ref_count);
        archi_process.context.signal_handler_data = NULL;
    }
}

void
finalize_global_contexts(void)
{
    // Finalize environment context
    archi_log_debug(__func__, "Finalizing environment context...");
    archi_exe_env_finalize();
}

///////////////////////////////////////////////////////////////////////////////

#define ARCHI_LOG_VERBOSITY_DEFAULT_LOWER ARCHI_LOG_VERBOSITY_NOTICE ///< Lower of two default verbosity levels.
#define ARCHI_LOG_VERBOSITY_DEFAULT_HIGHER ARCHI_LOG_VERBOSITY_INFO  ///< Higher of two default verbosity levels.

enum {
    ARGKEY_DRY_RUN = 'n',

    ARGKEY_NO_LOGO = 'L',
    ARGKEY_NO_COLOR = 'm',
    ARGKEY_VERBOSITY = 'v',

    ARGKEY_HELP = 0xFF,
};

static
const struct argp_option args_options[] = {
    {.doc = "Execution options:"},

    {.key = ARGKEY_DRY_RUN,     .name = "dry-run",  .group = 1,
        .doc = "Simulate execution: only log operations that would be executed, don't actually do anything"},

    {.doc = "Logging options:"},

    {.key = ARGKEY_NO_LOGO,     .name = "no-logo",  .group = 2,
                                    .doc = "Don't print the logo"},
    {.key = ARGKEY_NO_COLOR,    .name = "no-color", .group = 2,
                                    .doc = "Monochrome logging, don't use ANSI escape codes"},
    {.key = ARGKEY_VERBOSITY,   .name = "verbose",  .arg = "[LEVEL]", .flags = OPTION_ARG_OPTIONAL, .group = 2,
                                    .doc = "Set log verbosity level (number/name, see below)"},

    {0}
};

static
error_t
args_parser_func(
        int key,
        char *arg,
        struct argp_state *state)
{
    struct archi_exe_args *args = state->input;

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
            else if (ARCHI_STRING_COMPARE(arg, ==, "quiet"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_QUIET;
            else if (ARCHI_STRING_COMPARE(arg, ==, "error"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_ERROR;
            else if (ARCHI_STRING_COMPARE(arg, ==, "warning"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_WARNING;
            else if (ARCHI_STRING_COMPARE(arg, ==, "notice"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_NOTICE;
            else if (ARCHI_STRING_COMPARE(arg, ==, "info"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_INFO;
            else if (ARCHI_STRING_COMPARE(arg, ==, "debug"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_DEBUG;
            else if (ARCHI_STRING_COMPARE(arg, ==, "max"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY_MAX;
            else if ((strlen(arg) == 1) && (arg[0] >= '0') && (arg[0] <= '9'))
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

void
parse_args(
        int argc,
        char *argv[])
{
    archi_process.args = (struct archi_exe_args){
        .verbosity_level = ARCHI_LOG_VERBOSITY_DEFAULT_LOWER,
    };

    // Parse command line arguments
    struct argp args_parser = {
        .options = args_options,
        .parser = args_parser_func,
        .args_doc = "[FILE...]",
        .doc = "\n\
A versatile, modular application using builtin and plugin modules configured\n\
by memory-mapped initialization files containing data and instructions\n\
that define its specific behavior.\n\
\v\
Initialization files are mapped all at once, thus their mapping regions in memory must not overlap. \
Initialization files with overlapping memory regions cannot be used together within a single process.\n\
\n\
Logging verbosity levels:\n\
 0 ('quiet') - disable all log messages\n\
 1 ('error')\n\
 2 ('warning')\n\
 3 ('notice')\t[default level]\n\
 4 ('info')\t[level used if --verbose is specified without an argument]\n\
 5 ('debug' or 'max') - enable all log messages\n\
"
    };

    error_t err = argp_parse(&args_parser, argc, argv, 0, NULL, &archi_process.args);
    switch (err)
    {
        case 0: // success
            return;

        case EINVAL: // incorrect arguments
            fprintf(stderr, "Error: incorrect command line arguments.\n");
            exit(2);

        case ENOMEM: // memory allocation error
            fprintf(stderr, "Error: memory allocation failure while parsing command line arguments.\n");
            exit(1);

        default: // unknown error
            fprintf(stderr, "Error: unknown failure while parsing command line arguments.\n");
            exit(1);
    }
}

