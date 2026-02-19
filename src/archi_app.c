/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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
 * @brief Implementation of the Archipelago application.
 */

// Contexts & registry
#include "archi_app/registry.def.h"
#include "archi_app/registry.var.h"
#include "archi/context/api/interface.fun.h"
#include "archi/context/api/registry.fun.h"
#include "archi/context/api/tag.def.h"

// Hashmaps
#include "archi/hashmap/ctx/hashmap.var.h"
#include "archi/hashmap/api/hashmap.fun.h"

// Files
#include "archi_app/input_file.typ.h"
#include "archi_app/input_file.def.h"
#include "archi/file/api/file.fun.h"
#include "archi/file/ctx/mapping.var.h"

// Shared libraries
#include "archi/library/ctx/library.var.h"

// Signal management
#include "archi_app/signal_management.fun.h"
#include "archi/signal/api/management.fun.h"
#include "archi/signal/api/signal.fun.h"
#include "archi/signal/api/tag.def.h"
#include "archi/signal/sig/hashmap.fun.h"
#include "archi/signal/sig-ctx/hashmap_data.var.h"

// Environment
#include "archi_app/environment.fun.h"
#include "archi/env/api/context.fun.h"

// Error handling
#include "archi_base/error.fun.h"

// Logging
#include "archi_app/logging.fun.h"
#include "archi_log/context.fun.h"
#include "archi_log/print.fun.h"
#include "archi_log/verbosity.def.h"
#include "archi_log/color.def.h"

// Utilities
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/kvlist.fun.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/string.fun.h"

// System utilities
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Command line argument parsing
#include <argp.h>


///////////////////////////////////////////////////////////////////////////////
// Process state data
///////////////////////////////////////////////////////////////////////////////

struct archi_app_args {
    // Functionality options
    char **input;      ///< Array of pathnames of input memory-mapped initialization files.
    size_t num_inputs; ///< Number of input initialization files.

    bool dry_run; ///< Do a dry run: initialization instructions not executed, logged only.

    // Logging options
    bool no_logo;  ///< True if the application logo is not displayed.
    bool no_color; ///< True if color codes are not used when printing log messages.
    int verbosity_level; ///< Logging verbosity level.
};

static
struct {
    struct archi_app_args args; ///< Command line arguments.

    archi_error_t error; ///< The current execution error.

    struct {
        archi_context_t registry;    ///< The context registry.
        archi_context_t *input_file; ///< Array of the input file contexts.
    } context; ///< Special contexts.

    struct {
        archi_signal_set_t watch_set;   ///< Set of signals to watch.
        archi_signal_handler_t handler; ///< The signal handler.
        archi_context_t handler_data;   ///< The signal handler data context.
    } signal; ///< Signal management subsystem.

    size_t num_operations; ///< Total number of executed operations.
} archi_process; ///< The application process state.

///////////////////////////////////////////////////////////////////////////////
// Auxiliary macros
///////////////////////////////////////////////////////////////////////////////

#define INIT_ERROR()    do {                                                                        \
    ARCHI_ERROR_VAR_SET(&archi_process.error, ARCHI__EUNSPECIFIED, "error code hasn't been set");   \
} while (0)

#define INTERFACE_PTR(interface_var)    {                                       \
    .cptr = &interface_var,                                                     \
    .attr = ARCHI_POINTER_TYPE__DATA_READONLY |                                 \
        archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE),   \
}

///////////////////////////////////////////////////////////////////////////////
// Auxiliary functions
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
void
print_logo(void);

///////////////////////////////////////////////////////////////////////////////
// Bootstrapping & cleanup steps
///////////////////////////////////////////////////////////////////////////////

static
void
create_context_registry(void);

static
void
create_builtin_contexts(void);

static
void
finalize_contexts(void);

static
void
open_and_map_input_files(void);

static
void
initialize_signal_management(void);

static
void
finalize_signal_management(void);

///////////////////////////////////////////////////////////////////////////////
// main() and exit_*() functions
///////////////////////////////////////////////////////////////////////////////

static
void
exit_clean(void);

static
void
exit_quick(void);

static
void
exec_file(
        archi_context_t file_context,
        size_t file_index
);

static
void
exec_list(
        const archi_kvlist_t *operation_list,
        archi_reference_count_t file_ref_count,
        size_t list_index,
        size_t file_index
);

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
        if (!archi_app_log_initialize(NULL, archi_process.args.verbosity_level, !archi_process.args.no_color))
        {
            fprintf(stderr, "Error: couldn't initialize logging.\n");
            return EXIT_FAILURE;
        }

        archi_global_context_set__log(archi_app_log_global_context());
    }

    // Install exit functions
    {
        if (atexit(exit_clean) != 0)
        {
            archi_log_error(__func__, "Error: atexit(exit_clean) failed.");
            archi_app_log_finalize();
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

    // Initialize environment global state
    {
        archi_log_debug(__func__, "Initializing environment global state...");

        if (!archi_app_env_initialize())
        {
            archi_log_error(__func__, "Error: couldn't initialize environment global state.");
            exit(EXIT_FAILURE);
        }

        archi_global_context_set__env(archi_app_env_global_context());
    }

    /////////////////////////
    // Bootstrapping phase //
    /////////////////////////

    archi_log_info(__func__, "Bootstrapping the application...");

    open_and_map_input_files();
    create_context_registry();
    create_builtin_contexts();
    initialize_signal_management();

    /////////////////////
    // Execution phase //
    /////////////////////

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        // Insert the current input file context to the registry
        archi_log_debug(__func__, " * inserting file #%zu mapping context to the registry...", i);

        INIT_ERROR();
        archi_context_registry_insert(archi_process.context.registry,
                ARCHI_APP_REGISTRY_CONTEXT__INPUT_FILE, archi_process.context.input_file[i],
                &archi_process.error);
        print_error();

        if (archi_process.error.code != 0)
            exit(EXIT_FAILURE);

        // Execute the current input file
        exec_file(archi_process.context.input_file[i], i);

        // Disown the current input file
        archi_log_debug(__func__, " * disowning file #%zu mapping context...", i);

        archi_context_finalize(archi_process.context.input_file[i]);
        archi_process.context.input_file[i] = NULL;
    }

    return EXIT_SUCCESS;
}

void
exec_file(
        archi_context_t file_context,
        size_t file_index)
{
    archi_log_debug(__func__, " * executing file #%zu ('%s')...", file_index,
            archi_process.args.input[file_index]);

    size_t num_operation_lists = 0; // number of traversed operation lists in the current file

    // Obtain pointer to the file contents list
    const archi_kvlist_t *contents;
    {
        archi_rcpointer_t input_file = archi_context_data(file_context);
        const archi_app_input_file_header_t *input_file_header = input_file.cptr;
        contents = input_file_header->contents;
    }

    for (; contents != NULL; contents = contents->next)
    {
        // Skip everything that is not an operation list
        if ((contents->key == NULL) || ARCHI_STRING_COMPARE(
                    ARCHI_APP_INPUT_FILE_CONTENTS__OPERATIONS, !=, contents->key))
            continue;

        num_operation_lists++;

        if (!archi_pointer_attr_compatible(contents->value.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KVLIST)))
        {
            archi_log_warning(__func__, "Pointer to operation list #%zu has incorrect attributes, ignoring...",
                    num_operation_lists - 1);
            continue;
        }

        archi_log_debug(__func__, "   * [file #%zu] executing operation list #%zu...",
                file_index, num_operation_lists - 1);

        // Execute the current operation list
        exec_list(contents->value.cptr, archi_context_data(file_context).ref_count,
                num_operation_lists - 1, file_index);
    }
}

void
exec_list(
        const archi_kvlist_t *operation_list,
        archi_reference_count_t file_ref_count,
        size_t list_index,
        size_t file_index)
{
    size_t num_operations = 0; // number of executed operations in the current list

    for (; operation_list != NULL; operation_list = operation_list->next)
    {
        if (operation_list->key != NULL)
        {
            archi_log_debug(__func__, "     * [file #%zu, list #%zu, operation #%zu (%zu)] :: %s",
                    file_index, list_index, num_operations, archi_process.num_operations, operation_list->key);

            // Find the current operation function
            archi_app_registry_operation_func_t operation_fn = NULL;
            {
                size_t op_index = 0;
                while (archi_app_registry_operations[op_index].name != NULL)
                {
                    if (ARCHI_STRING_COMPARE(operation_list->key, ==,
                                archi_app_registry_operations[op_index].name))
                    {
                        operation_fn = archi_app_registry_operations[op_index].function;
                        break;
                    }
                    op_index++;
                }
            }

            if (operation_fn != NULL) // operation is supported
            {
                // Log the current operation data
                {
                    INIT_ERROR();
                    if (archi_print_lock(ARCHI_LOG_VERBOSITY__DEBUG))
                    {
                        /*********************************************/
                        operation_fn(NULL, operation_list->value, NULL,
                                &archi_process.error);
                        /*********************************************/
                        print_error();

                        archi_print_unlock();
                    }

                    if (archi_process.error.code != 0)
                        exit(EXIT_FAILURE);
                }

                // Execute the current operation
                if (!archi_process.args.dry_run)
                {
                    INIT_ERROR();
                    /********************************************/
                    operation_fn(archi_process.context.registry,
                            operation_list->value, file_ref_count,
                            &archi_process.error);
                    /********************************************/
                    print_error();

                    if (archi_process.error.code != 0)
                        exit(EXIT_FAILURE);
                }
            }
            else
                archi_log_warning(__func__, "Operation of type \"%s\" is not supported, ignoring...",
                        operation_list->key);
        }
        else // no-op
            archi_log_debug(__func__, "     * [file #%zu, list #%zu, operation #%zu (%zu)]",
                    file_index, list_index, num_operations, archi_process.num_operations);

        // Increment operation counters
        num_operations++;
        archi_process.num_operations++;
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
    finalize_contexts();

    if (archi_global_context__env() != NULL)
    {
        archi_log_debug(__func__, "Finalizing environment global state...");

        archi_app_env_finalize();
    }

    archi_log_info(__func__, "The application has exited normally.");

    archi_app_log_finalize();
}

void
exit_quick(void) // is called on quick_exit()
{
    archi_log_info(__func__, "The application has exited without the finalization procedures.");
}

///////////////////////////////////////////////////////////////////////////////
// Bootstrapping & cleanup steps
///////////////////////////////////////////////////////////////////////////////

void
create_context_registry(void)
{
    if (archi_process.args.dry_run)
        return;

    archi_log_debug(__func__, "Creating the context registry...");

    // Prepare context initialization parameters
    archi_hashmap_alloc_params_t hashmap_params = {
        .capacity = ARCHI_HASHMAP_DEFAULT_CAPACITY,
    };

    archi_krcvlist_t params[] = {
        {
            .key = "params",
            .value = {
                .ptr = &hashmap_params,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, archi_hashmap_alloc_params_t),
            },
        },
    };

    // Initialize a context registry hashmap context
    archi_rcpointer_t interface =
        INTERFACE_PTR(archi_context_interface__hashmap);

    INIT_ERROR();
    archi_process.context.registry = archi_context_initialize(
            interface, params, &archi_process.error);
    print_error();

    if (archi_process.context.registry == NULL)
        exit(EXIT_FAILURE);

    archi_log_debug(__func__, "Inserting the context registry into itself...");

    INIT_ERROR();
    archi_context_registry_insert(archi_process.context.registry,
            ARCHI_APP_REGISTRY_CONTEXT__REGISTRY, archi_process.context.registry,
            &archi_process.error);
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);
}

void
create_builtin_contexts(void)
{
    if (archi_process.args.dry_run)
        return;

    archi_log_debug(__func__, "Creating the library handle context of the executable...");

    archi_rcpointer_t interface =
        INTERFACE_PTR(archi_context_interface__library);

    INIT_ERROR();
    archi_context_registry_create(archi_process.context.registry,
            ARCHI_APP_REGISTRY_CONTEXT__EXECUTABLE, interface,
            (archi_context_registry_params_t){0},
            &archi_process.error);
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);
}

void
finalize_contexts(void)
{
    if (archi_process.context.registry != NULL)
    {
        archi_log_debug(__func__, "Finalizing context registry...");

        // Remove all contexts from the registry
        archi_hashmap_traverse(archi_context_data(archi_process.context.registry).ptr,
                false, archi_hashmap_trav_kv__unset_all, NULL, NULL);

        archi_context_finalize(archi_process.context.registry);
        archi_process.context.registry = NULL;
    }

    if (archi_process.context.input_file != NULL)
    {
        for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        {
            archi_log_debug(__func__, "Finalizing file #%zu mapping context...", i);

            archi_context_finalize(archi_process.context.input_file[i]);
        }

        free(archi_process.context.input_file);
        archi_process.context.input_file = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

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
    archi_file_map_params_t map_params = {
        .ptr_support = true,
        .readable = true,
    };

    archi_krcvlist_t params[] = {
        {
            .key = "fd",
            .value = {
                .ptr = &fd,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, archi_file_descriptor_t),
            },
        },
        {
            .key = "params",
            .value = {
                .ptr = &map_params,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, archi_file_map_params_t),
            },
        },
    };
    params[0].next = &params[1];

    // Initialize a file mapping context
    archi_rcpointer_t interface =
        INTERFACE_PTR(archi_context_interface__file_mapping);

    INIT_ERROR();
    archi_process.context.input_file[index] = archi_context_initialize(
            interface, params, &archi_process.error);
    print_error();

    // Close file descriptor
    INIT_ERROR();
    archi_file_close(fd, &archi_process.error); // this line prevents from initialize_context() being used here
    print_error();

    if ((archi_process.context.input_file[index] == NULL) ||
            (archi_process.error.code != 0))
        exit(EXIT_FAILURE);
}

static
void
check_input_file_format(
        archi_rcpointer_t input_file)
{
    size_t length, stride;
    archi_pointer_attr_unpk__pdata(input_file.attr, &length, &stride, NULL, &archi_process.error);
    size_t size = length * stride;

    if (archi_process.error.code != 0)
    {
        archi_log_error(__func__, "Error: input file mapping attributes are incorrect.");
        exit(EXIT_FAILURE);
    }

    if (size < sizeof(archi_app_input_file_header_t))
    {
        archi_log_error(__func__, "Error: input file mapping is too short (file header won't fit).");
        exit(EXIT_FAILURE);
    }

    const archi_app_input_file_header_t *input_file_header = input_file.cptr;

    if (strncmp(ARCHI_APP_INPUT_FILE_MAGIC, input_file_header->magic, sizeof(input_file_header->magic)) != 0)
    {
        archi_log_error(__func__, "Error: input file format is invalid (magic bytes are incorrect).");
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
        archi_log_debug(__func__, " * mapping file #%zu ('%s') into memory", i, archi_process.args.input[i]);
        map_input_file(i, fd);

        // Check input file format
        archi_rcpointer_t input_file = archi_context_data(archi_process.context.input_file[i]);

        archi_log_debug(__func__, " * checking format of file #%zu ('%s')", i, archi_process.args.input[i]);
        check_input_file_format(input_file);

        // Log file address & size
        const archi_app_input_file_header_t *input_file_header = input_file.cptr;
        size_t size = sizeof(input_file_header->header) + input_file_header->header.size;

        archi_log_debug(__func__, "\taddress = %p, size = %zu B (%.2f KiB, %.2f MiB, %.2f GiB)",
                input_file_header->header.addr, size,
                size / 1024.0, size / (1024.0 * 1024.0), size / (1024.0 * 1024.0 * 1024.0));
    }
}

///////////////////////////////////////////////////////////////////////////////

static
void
print_signal_set(
        archi_signal_set_const_t signal_set)
{
    for (int i = 0; i < ARCHI_SIGNAL_NUMBER; i++)
        if (archi_signal_set_contains(signal_set, i))
            archi_print(" %s", archi_signal_name[i]);

    for (int i = 0; i < ARCHI_SIGNAL_NUMBER_REALTIME; i++)
        if (archi_signal_set_contains(signal_set, ARCHI__SIGRTMIN+i))
            archi_print(" %s+%i", archi_signal_name[ARCHI__SIGRTMIN], i);
}

static
bool
prepare_signal_watch_set(void)
{
    // Allocate signal watch set
    archi_log_debug(__func__, "Allocating the signal watch set...");

    archi_process.signal.watch_set = archi_signal_set_alloc();
    if (archi_process.signal.watch_set == NULL)
    {
        archi_log_error(__func__, "Error: couldn't allocate the signal watch set.");
        exit(EXIT_FAILURE);
    }

    // Join signal watch sets from all files into one
    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(__func__, " * joining signal watch sets from file #%zu ('%s')",
                i, archi_process.args.input[i]);

        size_t num_signal_sets = 0;

        const archi_kvlist_t *contents;
        {
            archi_rcpointer_t input_file = archi_context_data(archi_process.context.input_file[i]);
            const archi_app_input_file_header_t *input_file_header = input_file.cptr;
            contents = input_file_header->contents;
        }

        for (; contents != NULL; contents = contents->next)
        {
            // Skip everything that is not a signal set
            if ((contents->key == NULL) || ARCHI_STRING_COMPARE(
                        ARCHI_APP_INPUT_FILE_CONTENTS__SIGNALS, !=, contents->key))
                continue;

            num_signal_sets++;

            if (!archi_pointer_attr_compatible(contents->value.attr,
                        archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__SIGNAL_SET)))
            {
                archi_log_warning(__func__, "Pointer to signal watch set #%zu has incorrect attributes, ignoring...",
                        num_signal_sets - 1);
                continue;
            }

            archi_signal_set_const_t signal_watch_set = contents->value.cptr;

            // Log the signal set
            if (archi_print_lock(ARCHI_LOG_VERBOSITY__DEBUG))
            {
                archi_print("    signal watch set #%zu: {", num_signal_sets - 1);
                print_signal_set(signal_watch_set);
                archi_print(" }\n");

                archi_print_unlock();
            }

            // Join the signal set
            archi_signal_set_join(archi_process.signal.watch_set, signal_watch_set);
        }
    }

    // Decide if signal management thread is needed
    bool enable_signal_management_thread = !archi_signal_set_is_empty(archi_process.signal.watch_set);

    if (enable_signal_management_thread)
    {
        archi_log_debug(__func__, "Signal management thread: ENABLED");

        // Log the signal watch set
        if (archi_print_lock(ARCHI_LOG_VERBOSITY__DEBUG))
        {
            archi_print("    signal watch set: {");
            print_signal_set(archi_process.signal.watch_set);
            archi_print(" }\n");

            archi_print_unlock();
        }
    }
    else
    {
        archi_log_debug(__func__, "Signal management thread: DISABLED");

        // Free the signal watch set
        free(archi_process.signal.watch_set);
        archi_process.signal.watch_set = NULL;
    }

    return enable_signal_management_thread;
}

void
initialize_signal_management(void)
{
    if (archi_process.args.dry_run)
        return;

    // Prepare signal watch set
    if (!prepare_signal_watch_set())
        return; // signal management subsystem is not required

    archi_log_debug(__func__, "Initializing the signal handler data context...");

    {
        // Prepare context initialization parameters
        archi_hashmap_alloc_params_t hashmap_params = {
            .capacity = ARCHI_HASHMAP_DEFAULT_CAPACITY,
        };

        archi_krcvlist_t params[] = {
            {
                .key = "params",
                .value = {
                    .ptr = &hashmap_params,
                    .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                        ARCHI_POINTER_ATTR__PDATA(1, archi_hashmap_alloc_params_t),
                },
            },
        };

        archi_rcpointer_t interface =
            INTERFACE_PTR(archi_context_interface__signal_handler_data__hashmap);

        INIT_ERROR();
        archi_process.signal.handler_data = archi_context_registry_create(archi_process.context.registry,
                ARCHI_APP_REGISTRY_CONTEXT__SIGNAL_HANDLER, interface,
                (archi_context_registry_params_t){.list = params},
                &archi_process.error);
        print_error();

        if (archi_process.error.code != 0)
            exit(EXIT_FAILURE);
    }

    archi_log_debug(__func__, "Starting the signal management thread...");

    archi_process.signal.handler.function = archi_signal_handler__hashmap;
    archi_process.signal.handler.data = archi_context_data(archi_process.signal.handler_data).ptr;

    INIT_ERROR();
    archi_app_signal_management_start(archi_process.signal.watch_set, archi_process.signal.handler,
                &archi_process.error);
    print_error();

    if (archi_process.error.code != 0)
        exit(EXIT_FAILURE);

    archi_global_context_set__signal_management(archi_app_signal_management_global_context());
}

void
finalize_signal_management(void)
{
    if (archi_process.signal.watch_set == NULL)
        return; // signal management subsystem hasn't been initialized

    archi_log_debug(__func__, "Stopping the signal management thread...");

    archi_app_signal_management_stop();

    if (archi_process.signal.handler_data != NULL)
    {
        archi_log_debug(__func__, "Finalizing the signal handler data context...");

        archi_context_finalize(archi_process.signal.handler_data);
        archi_process.signal.handler_data = NULL;
    }

    free(archi_process.signal.watch_set);
    archi_process.signal.watch_set = NULL;

    archi_process.signal.handler = (archi_signal_handler_t){0};
}

///////////////////////////////////////////////////////////////////////////////
// Auxiliary functions
///////////////////////////////////////////////////////////////////////////////

void
print_logo(void)
{
#define LINES_1      5
#define COLUMNS_1    5

#define LINES_2      4
#define COLUMNS_2    6

    static const char *logo1[LINES_1][COLUMNS_1] = {
        {"    ", "    ", " ⡔⠢ ", "    ", "   "},
        {"    ", " ⣏⡱ ", " ⢇⡠ ", " ⣇⣸ ", "   "},
        {" ⡔⢢ ", " ⠇⠱ ", "    ", " ⠇⠸ ", " ⢰ "},
        {" ⡏⢹ ", "   ⣠", "⡾⠛⠛⢷", "⣄   ", " ⢸ "},
        {"    ", "  ⢾⣏", " ⢾⡷ ", "⣹⡷  ", "   "},
    };
    static const char *logo2[LINES_2][COLUMNS_2] = {
        {" ⢸⣉⠆", "   ", "⠙⢷⣤", "⣤⡾⠋", "   ", "⢰⠉⡆"},
        {" ⠸  ", "⢸⣉⠁", "   ", "   ", "⢰⠉⠂", "⠘⠤⠃"},
        {"    ", "⠸⠤⠄", "⢸  ", "⢰⣉⡆", "⠘⠬⠃", "   "},
        {"    ", "   ", "⠸⠤⠄", "⠸ ⠇", "   ", "   "},
    };

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

    const int indentation = 6;

    if (archi_print_lock(0))
    {
        archi_print_color(ARCHI_COLOR_RESET);
        archi_print("\n");

        for (int i = 0; i < LINES_1; i++)
        {
            archi_print("%*s", indentation, "");

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
            archi_print("%*s", indentation, "");

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

void
print_error(void)
{
#define ERROR_ORIGIN_LENGTH     1024

    char error_origin[ERROR_ORIGIN_LENGTH];

    if ((archi_process.error.origin.func != NULL) && (archi_process.error.origin.file != NULL))
        snprintf(error_origin, ERROR_ORIGIN_LENGTH, "%s()@'%s':%i",
                archi_process.error.origin.func,
                archi_process.error.origin.file,
                archi_process.error.origin.line);
    else if (archi_process.error.origin.func != NULL)
        snprintf(error_origin, ERROR_ORIGIN_LENGTH, "%s()",
                archi_process.error.origin.func);
    else if (archi_process.error.origin.file != NULL)
        snprintf(error_origin, ERROR_ORIGIN_LENGTH, "'%s':%i",
                archi_process.error.origin.file,
                archi_process.error.origin.line);
    else
        error_origin[0] = '\0';

#undef ERROR_ORIGIN_LENGTH

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

///////////////////////////////////////////////////////////////////////////////

#define ARCHI_LOG_VERBOSITY__DEFAULT_LOWER ARCHI_LOG_VERBOSITY__NOTICE ///< Lower of two default verbosity levels.
#define ARCHI_LOG_VERBOSITY__DEFAULT_HIGHER ARCHI_LOG_VERBOSITY__INFO  ///< Higher of two default verbosity levels.

enum {
    ARGKEY_DRY_RUN = 'n',

    ARGKEY_NO_LOGO = 'L',
    ARGKEY_NO_COLOR = 'm',
    ARGKEY_VERBOSITY = 'v',

    ARGKEY_VERSION = 0xFE,
    ARGKEY_HELP = 0xFF,
};

static
const struct argp_option
args_options[] = {
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

    {.doc = "Other options:"},

    {.key = ARGKEY_VERSION,     .name = "version",  .group = 3,
                                    .doc = "Give version and the list of built-in modules"},

    {0}
};

static
error_t
args_parser_func(
        int key,
        char *arg,
        struct argp_state *state)
{
    struct archi_app_args *args = state->input;

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
                args->verbosity_level = ARCHI_LOG_VERBOSITY__DEFAULT_HIGHER;
            else if (ARCHI_STRING_COMPARE(arg, ==, "quiet"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY__QUIET;
            else if (ARCHI_STRING_COMPARE(arg, ==, "error"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY__ERROR;
            else if (ARCHI_STRING_COMPARE(arg, ==, "warning"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY__WARNING;
            else if (ARCHI_STRING_COMPARE(arg, ==, "notice"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY__NOTICE;
            else if (ARCHI_STRING_COMPARE(arg, ==, "info"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY__INFO;
            else if (ARCHI_STRING_COMPARE(arg, ==, "debug"))
                args->verbosity_level = ARCHI_LOG_VERBOSITY__DEBUG;
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

        case ARGKEY_VERSION:
            printf("version: %s\n", ARCHI_APP_VERSION);
            printf("built-in modules:\n");
            for (const char *module = ARCHI_APP_BUILTIN_MODULES; module != NULL;)
            {
                const char *sep = strchr(module, ':');
                int len = (sep != NULL) ? sep - module : (int)strlen(module);
                if (len == 0)
                    break;

                printf("  %.*s\n", len, module);

                module = (sep != NULL) ? sep + 1 : NULL;
            }
            printf("features:\n");
            for (const char *feature = ARCHI_APP_FEATURES; feature != NULL;)
            {
                const char *sep = strchr(feature, '\a');
                int len = (sep != NULL) ? sep - feature : (int)strlen(feature);
                if (len == 0)
                    break;

                printf("  %.*s\n", len, feature);

                feature = (sep != NULL) ? sep + 1 : NULL;
            }
            exit(EXIT_SUCCESS);

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
    archi_process.args = (struct archi_app_args){
        .verbosity_level = ARCHI_LOG_VERBOSITY__DEFAULT_LOWER,
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

