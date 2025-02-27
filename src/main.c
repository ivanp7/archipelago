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
 * @brief Standard main() implementation.
 */

#include "archi/exe/args.fun.h"
#include "archi/exe/args.typ.h"
#include "archi/exe/builtin.var.h"
#include "archi/exe/config.typ.h"
#include "archi/exe/interface.fun.h"
#include "archi/app/config.fun.h"
#include "archi/app/loader.fun.h"
#include "archi/app/instance.typ.h"
#include "archi/fsm/algorithm.fun.h"
#include "archi/util/container.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"
#include "archi/util/print.def.h"
#include "archi/util/os/shm.fun.h"
#include "archi/util/os/signal.fun.h"

#include <stdlib.h> // for atexit()
#include <string.h> // for strerror()
#include <stdint.h> // for fixed-width integer types
#include <errno.h> // for error codes and errno


#define SAFE(str) ((str) != NULL ? (str) : "[NULL]")

#define SPACE "        "

///////////////////////////////////////////////////////////////////////////////

static
struct {
    archi_args_t args; ///< Command line arguments.

    const archi_process_config_shm_t **config; ///< Configurations of the process in shared memory.

    archi_signal_watch_set_t *signal_watch_set; ///< Signal watch set.
    struct archi_signal_management_context *signal_management; ///< Signal management context.
    archi_fsm_t fsm; ///< Finite state machine.
    archi_application_t app; ///< Application instance.

    struct {
        archi_list_container_data_t libraries;  ///< Data for the libraries container.
        archi_list_container_data_t interfaces; ///< Data for the context interfaces container.
        archi_list_container_data_t contexts;   ///< Data for the contexts container.
    } app_containers; ///< Application containers.

    struct {
#define NUM_BUILTIN_CONTEXTS 2 // (Signal management context) + (finite state machine).

        archi_context_t contexts[NUM_BUILTIN_CONTEXTS];  ///< Array of built-in contexts.
        const char *contexts_keys[NUM_BUILTIN_CONTEXTS]; ///< Array of keys of built-in contexts.
    } builtin;

    struct {
        size_t inputs; ///< Number of processed configurations
        size_t steps;  ///< Number of processed steps in the processed configuration
    } count;
} archi_process;

///////////////////////////////////////////////////////////////////////////////

static
void
init_app_containers(void);

static
void
alloc_config_array(void);

static
void
free_config_array(void);

static
void
map_shared_memory(void);

static
void
unmap_shared_memory(void);

static
void
start_signal_management(void);

static
void
stop_signal_management(void);

static
void
init_builtin_contexts(void);

static
void
add_builtin_context_interfaces(void);

static
void
remove_builtin_context_interfaces(void);

static
void
add_builtin_contexts(void);

static
void
remove_builtin_contexts(void);

static
void
load_shared_libraries(
        size_t index
);

static
void
unload_shared_libraries(void);

static
void
add_context_interfaces(
        size_t index
);

static
void
remove_context_interfaces(void);

static
void
configure_app(
        size_t index
);

static
void
reset_app(void);

static
void
exit_cleanup(void);

static
void
exit_quick(void);

///////////////////////////////////////////////////////////////////////////////

#define M "main()"

int
main(
        int argc,
        char *argv[])
{
    //////////////////
    // Preparations //
    //////////////////

    // Record the application start time
    archi_log_set_start_time(NULL);

    // Parse command line arguments
    switch (archi_args_parse(&archi_process.args, argc, argv))
    {
        case 0: // success
            break;

        case EINVAL: // incorrect arguments
            archi_print("Error: incorrect command line arguments.\n");
            return ARCHI_EXIT_CODE(ARCHI_ERROR_MISUSE);
        case ENOMEM: // memory allocation error
            archi_print("Error: memory allocation fail while parsing command line arguments.\n");
            return ARCHI_EXIT_CODE(ARCHI_ERROR_ALLOC);
        default: // unknown error
            archi_print("Error: unknown fail while parsing command line arguments.\n");
            return ARCHI_EXIT_CODE(ARCHI_ERROR_UNKNOWN);
    }

    // Set logging verbosity
    archi_log_set_verbosity(archi_process.args.verbosity_level);

    // Display application logo
    if (!archi_process.args.no_logo)
        archi_print(ARCHI_COLOR_RESET "\n" ARCHI_COLOR_FG_BRI_WHITE "%s"
                ARCHI_COLOR_RESET "\n\n", ARCHI_PELAGO_LOGO);

    // Exit if there is nothing to do
    if (archi_process.args.num_inputs == 0)
        return 0;

    // Set exit functions
    atexit(exit_cleanup);
    at_quick_exit(exit_quick);

    //////////////////////////
    // Initialization phase //
    //////////////////////////

    archi_log_info(M, "Initializing the application...");

    // Initialize application containers
    init_app_containers();

    // Allocate array of configurations
    alloc_config_array();

    // Map shared memory
    map_shared_memory();

    // Start signal management
    start_signal_management();

    // Initialize built-in contexts
    init_builtin_contexts();

    // Add built-in context interfaces
    add_builtin_context_interfaces();

    // Add built-in contexts
    add_builtin_contexts();

    // Process the inputs sequentially
    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(M, "[%u] Processing input configuration '%s'...",
                (unsigned)i, archi_process.args.inputs[i]);

        // Load shared libraries
        load_shared_libraries(i);

        // Add context interfaces from libraries
        add_context_interfaces(i);

        // Configure the application
        configure_app(i);
    }

    /////////////////////
    // Execution phase //
    /////////////////////

    if (!archi_process.args.no_fsm &&
            ((archi_process.fsm.entry_state.function != NULL) ||
             (archi_process.fsm.transition.function != NULL)))
    {
        archi_log_info(M, "Executing the finite state machine...");

        archi_status_t code = archi_fsm_execute(archi_process.fsm);
        return ARCHI_EXIT_CODE(code);
    }

    return 0;
}

#undef M
#define M "exit_cleanup()"

void
exit_cleanup(void) // is called on exit() or if main() returns
{
    ////////////////////////
    // Finalization phase //
    ////////////////////////

    archi_log_info(M, "Finalizing the application...");

    if (archi_process.config != NULL)
    {
        // Undo the configuration
        reset_app();

        // Remove context interfaces
        remove_context_interfaces();

        // Unload libraries
        unload_shared_libraries();
    }

    // Remove built-in contexts
    remove_builtin_contexts();

    // Remove built-in context interfaces
    remove_builtin_context_interfaces();

    // Stop signal management
    stop_signal_management();

    // Unmap shared memory
    unmap_shared_memory();

    // Free array of configurations
    free_config_array();

    // Finalization is done
    archi_log_info(M, "The application has exited successfully.");
}

#undef M
#define M "exit_quick()"

void
exit_quick(void) // is called on quick_exit()
{
    archi_log_info(M, "Finalizing the application without cleaning the resources...");

    if (archi_process.config != NULL)
    {
        // Undo the configuration
        reset_app();
    }

    // Finalization is done
    archi_log_info(M, "The application has exited successfully without cleaning the resources.");
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "init_app_containers()"

void
init_app_containers(void)
{
    archi_process.app = (archi_application_t){
        .libraries = {
            .data = &archi_process.app_containers.libraries,
            .interface = &archi_list_container_interface,
        },
        .interfaces = {
            .data = &archi_process.app_containers.interfaces,
            .interface = &archi_list_container_interface,
        },
        .contexts = {
            .data = &archi_process.app_containers.contexts,
            .interface = &archi_list_container_interface,
        },
    };

    archi_process.app_containers.libraries =
        archi_process.app_containers.interfaces =
        archi_process.app_containers.contexts =
        (archi_list_container_data_t){
            .insert_to_head = false,
            .traverse_from_head = false,
        };
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "alloc_config_array()"

void
alloc_config_array(void)
{
    archi_log_debug(M, "Allocating the array of %u configuration pointers...",
            (unsigned)archi_process.args.num_inputs);

    archi_process.config = malloc(sizeof(*archi_process.config) * archi_process.args.num_inputs);
    if (archi_process.config == NULL)
    {
        archi_log_error(M, "Couldn't allocate the array of configuration pointers.");
        exit(ARCHI_EXIT_CODE(ARCHI_ERROR_ALLOC));
    }

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        archi_process.config[i] = NULL;
}

#undef M
#define M "free_config_array()"

void
free_config_array(void)
{
    archi_log_debug(M, "Freeing the array of configuration pointers...");

    free(archi_process.config);
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "map_shared_memory()"

void
map_shared_memory(void)
{
    archi_log_debug(M, "Mapping %u configuration files...", (unsigned)archi_process.args.num_inputs);

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        archi_log_debug(M, "> open('%s')", archi_process.args.inputs[i]);

        errno = 0;
        int fd = archi_shm_open_file(archi_process.args.inputs[i], true, false); // open for read only
        if (fd == -1)
        {
            archi_log_error(M, "Couldn't open memory-mapped configuration file '%s': %s.",
                    archi_process.args.inputs[i], strerror(errno));
            exit(ARCHI_EXIT_CODE(ARCHI_ERROR_RESOURCE));
        }

        archi_log_debug(M, "> mmap('%s')", archi_process.args.inputs[i]);

        errno = 0;
        archi_process.config[i] = (const archi_process_config_shm_t*)
            archi_shm_map(fd, true, false, false, 0); // map private read-only copy
        if (archi_process.config[i] == NULL)
        {
            archi_log_error(M, "Couldn't map memory-mapped configuration file '%s': %s.",
                    archi_process.args.inputs[i], (errno != 0) ? strerror(errno) : "file is mapped at a wrong address");

            archi_log_debug(M, "> close('%s')", archi_process.args.inputs[i]);
            archi_shm_close(fd);

            exit(ARCHI_EXIT_CODE(ARCHI_ERROR_MAP));
        }

        archi_log_debug(M, "> close('%s')", archi_process.args.inputs[i]);

        errno = 0;
        if (!archi_shm_close(fd))
            archi_log_warning(M, "Couldn't close memory-mapped configuration file '%s': %s.",
                    archi_process.args.inputs[i], strerror(errno));
    }
}

#undef M
#define M "unmap_shared_memory()"

void
unmap_shared_memory(void)
{
    archi_log_debug(M, "Unmapping memory-mapped configuration files...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        size_t index = (archi_process.args.num_inputs - 1) - i; // reverse order

        if (archi_process.config[index] == NULL)
            continue;

        archi_log_debug(M, "> munmap('%s')", archi_process.args.inputs[index]);

        errno = 0;
        if (!archi_shm_unmap((archi_shm_header_t*)archi_process.config[index]))
            archi_log_error(M, "Couldn't unmap memory-mapped configuration file '%s': %s.",
                    archi_process.args.inputs[index], strerror(errno));
    }
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "start_signal_management()"

void
start_signal_management(void)
{
    archi_log_debug(M, "Joining signal watch sets...");

    archi_process.signal_watch_set = archi_signal_watch_set_alloc();
    if (archi_process.signal_watch_set == NULL)
    {
        archi_log_error(M, "Couldn't allocate the application signal watch set.");
        exit(ARCHI_EXIT_CODE(ARCHI_ERROR_ALLOC));
    }

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
        archi_signal_watch_set_join(archi_process.signal_watch_set,
                archi_process.config[i]->signal_watch_set);

    bool signal_management_needed = false;

#define LOG_SIGNAL(signal) do { \
    if (archi_process.signal_watch_set->f_##signal) { \
        archi_log_debug(M, "> watch_signal(%s)", #signal); \
        signal_management_needed = true; \
    } } while (0)

    LOG_SIGNAL(SIGINT);
    LOG_SIGNAL(SIGQUIT);
    LOG_SIGNAL(SIGTERM);

    LOG_SIGNAL(SIGCHLD);
    LOG_SIGNAL(SIGCONT);
    LOG_SIGNAL(SIGTSTP);
    LOG_SIGNAL(SIGXCPU);
    LOG_SIGNAL(SIGXFSZ);

    LOG_SIGNAL(SIGPIPE);
    LOG_SIGNAL(SIGPOLL);
    LOG_SIGNAL(SIGURG);

    LOG_SIGNAL(SIGALRM);
    LOG_SIGNAL(SIGVTALRM);
    LOG_SIGNAL(SIGPROF);

    LOG_SIGNAL(SIGHUP);
    LOG_SIGNAL(SIGTTIN);
    LOG_SIGNAL(SIGTTOU);
    LOG_SIGNAL(SIGWINCH);

    LOG_SIGNAL(SIGUSR1);
    LOG_SIGNAL(SIGUSR2);

#undef LOG_SIGNAL

    for (size_t i = 0; i < archi_signal_number_of_rt_signals(); i++)
    {
        if (archi_process.signal_watch_set->f_SIGRTMIN[i])
        {
            archi_log_debug(M, "> watch_signal(SIGRTMIN+%u)", (unsigned)i);
            signal_management_needed = true;
        }
    }

    if (signal_management_needed)
    {
        archi_log_debug(M, "Starting signal management...");

        errno = 0;
        archi_process.signal_management = archi_signal_management_start(
                archi_process.signal_watch_set, (archi_signal_handler_t){0});

        if (archi_process.signal_management == NULL)
        {
            archi_log_error(M, "Couldn't start signal management: %s.", strerror(errno));
            exit(ARCHI_EXIT_CODE(ARCHI_ERROR_INIT));
        }
    }
}

#undef M
#define M "stop_signal_management()"

void
stop_signal_management(void)
{
    if (archi_process.signal_management != NULL)
    {
        archi_log_debug(M, "Stopping signal management...");

        archi_signal_management_stop(archi_process.signal_management);
    }

    archi_log_debug(M, "Freeing the application signal watch set...");

    free(archi_process.signal_watch_set);
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "init_builtin_contexts()"

void
init_builtin_contexts(void)
{
    // Signal management context
    archi_process.builtin.contexts[0] = (archi_context_t){
        .handle = archi_process.signal_management,
        .interface = &archi_app_signal_interface,
    };
    archi_process.builtin.contexts_keys[0] = ARCHI_APP_SIGNAL_CONTEXT_KEY;

    // Finite state machine
    archi_process.builtin.contexts[1] = (archi_context_t){
        .handle = &archi_process.fsm,
        .interface = &archi_app_fsm_interface,
    };
    archi_process.builtin.contexts_keys[1] = ARCHI_APP_FSM_CONTEXT_KEY;
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "add_builtin_context_interfaces()"

void
add_builtin_context_interfaces(void)
{
    archi_log_debug(M, "Registering built-in context interfaces...");

    for (size_t i = 0; i < archi_builtin_interfaces_num_of; i++)
    {
        archi_log_debug(M, "> register_builtin_interface('%s')",
                archi_builtin_interfaces_keys[i]);

        archi_status_t code = archi_container_insert(archi_process.app.interfaces,
                archi_builtin_interfaces_keys[i], (void*)archi_builtin_interfaces[i]);

        if (code != 0)
        {
            archi_log_error(M, "Couldn't register built-in context interface '%s' (error %i).",
                    (unsigned)i, archi_builtin_interfaces_keys[i], code);
            exit(ARCHI_EXIT_CODE(code));
        }
    }
}

#undef M
#define M "remove_builtin_context_interfaces()"

void
remove_builtin_context_interfaces(void)
{
    archi_log_debug(M, "Unregistering built-in context interfaces...");

    for (size_t i = 0; i < archi_builtin_interfaces_num_of; i++)
    {
        size_t index = (archi_builtin_interfaces_num_of - 1) - i; // reverse order

        archi_log_debug(M, "> unregister_builtin_interface('%s')",
                archi_builtin_interfaces_keys[index]);

        archi_status_t code = archi_container_remove(archi_process.app.interfaces,
                archi_builtin_interfaces_keys[index], NULL);

        if (code < 0) // ignore 'not found' error
            archi_log_error(M, "Couldn't unregister built-in context interface '%s' (error %i).",
                    archi_builtin_interfaces_keys[index], code);
    }
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "add_builtin_contexts()"

void
add_builtin_contexts(void)
{
    archi_log_debug(M, "Registering built-in contexts...");

    for (size_t i = 0; i < NUM_BUILTIN_CONTEXTS; i++)
    {
        archi_log_debug(M, "> register_builtin_context('%s')",
                archi_process.builtin.contexts_keys[i]);

        archi_status_t code = archi_container_insert(archi_process.app.contexts,
                archi_process.builtin.contexts_keys[i], &archi_process.builtin.contexts[i]);

        if (code != 0)
        {
            archi_log_error(M, "Couldn't register built-in context '%s' (error %i).",
                    (unsigned)i, archi_process.builtin.contexts_keys[i], code);
            exit(ARCHI_EXIT_CODE(code));
        }
    }
}

#undef M
#define M "remove_builtin_contexts()"

void
remove_builtin_contexts(void)
{
    archi_log_debug(M, "Unregistering built-in contexts...");

    for (size_t i = 0; i < NUM_BUILTIN_CONTEXTS; i++)
    {
        size_t index = (NUM_BUILTIN_CONTEXTS - 1) - i; // reverse order

        archi_log_debug(M, "> unregister_builtin_context('%s')",
                archi_process.builtin.contexts_keys[index]);

        archi_status_t code = archi_container_remove(archi_process.app.contexts,
                archi_process.builtin.contexts_keys[index], NULL);

        if (code < 0) // ignore 'not found' error
            archi_log_error(M, "Couldn't unregister built-in context '%s' (error %i).",
                    archi_process.builtin.contexts_keys[index], code);
    }
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "load_shared_libraries()"

void
load_shared_libraries(
        size_t index)
{
    if (archi_process.config[index]->app_config.libraries == NULL)
        return;

    archi_log_debug(M, "Loading %u shared libraries...",
            (unsigned)archi_process.config[index]->app_config.num_libraries);

    for (size_t i = 0; i < archi_process.config[index]->app_config.num_libraries; i++)
    {
        archi_log_debug(M, "> load_library('%s', '%s')",
                SAFE(archi_process.config[index]->app_config.libraries[i].key),
                SAFE(archi_process.config[index]->app_config.libraries[i].pathname));

        archi_status_t code = archi_app_add_library(&archi_process.app,
                archi_process.config[index]->app_config.libraries[i]);

        if (code != 0)
        {
            archi_log_error(M, "Couldn't load shared library '%s' (error %i).",
                    SAFE(archi_process.config[index]->app_config.libraries[i].key), code);
            exit(ARCHI_EXIT_CODE(code));
        }
    }
}

#undef M
#define M "unload_shared_libraries()"

void
unload_shared_libraries(void)
{
    archi_log_debug(M, "Unloading shared libraries...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        size_t input_index = (archi_process.args.num_inputs - 1) - i; // reverse order

        if (archi_process.config[input_index] == NULL)
            continue;

        for (size_t j = 0; j < archi_process.config[input_index]->app_config.num_libraries; j++)
        {
            size_t index = (archi_process.config[input_index]->app_config.num_libraries - 1) - j; // reverse order

            const char *key = archi_process.config[input_index]->app_config.libraries[index].key;
            if (archi_container_extract(archi_process.app.libraries, key, NULL) == 0)
            {
                archi_log_debug(M, "> unload_library('%s')", SAFE(key));

                archi_status_t code = archi_app_remove_library(&archi_process.app, key);

                if (code != 0)
                    archi_log_error(M, "Couldn't unload shared library '%s' (error %i).", SAFE(key), code);
            }
        }
    }
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "add_context_interfaces()"

void
add_context_interfaces(
        size_t index)
{
    if (archi_process.config[index]->app_config.interfaces == NULL)
        return;

    archi_log_debug(M, "Registering %u context interfaces...",
            (unsigned)archi_process.config[index]->app_config.num_interfaces);

    for (size_t i = 0; i < archi_process.config[index]->app_config.num_interfaces; i++)
    {
        archi_log_debug(M, "> register_interface('%s', '%s', '%s')",
                SAFE(archi_process.config[index]->app_config.interfaces[i].key),
                SAFE(archi_process.config[index]->app_config.interfaces[i].symbol_name),
                SAFE(archi_process.config[index]->app_config.interfaces[i].library_key));

        archi_status_t code = archi_app_add_interface(&archi_process.app,
                archi_process.config[index]->app_config.interfaces[i]);

        if (code != 0)
        {
            archi_log_error(M, "Couldn't register context interface '%s' (error %i).",
                    SAFE(archi_process.config[index]->app_config.interfaces[i].key), code);
            exit(ARCHI_EXIT_CODE(code));
        }
    }
}

#undef M
#define M "remove_context_interfaces()"

void
remove_context_interfaces(void)
{
    archi_log_debug(M, "Unregistering context interfaces...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        size_t input_index = (archi_process.args.num_inputs - 1) - i; // reverse order

        if (archi_process.config[input_index] == NULL)
            continue;

        for (size_t j = 0; j < archi_process.config[input_index]->app_config.num_interfaces; j++)
        {
            size_t index = (archi_process.config[input_index]->app_config.num_interfaces - 1) - j; // reverse order

            const char *key = archi_process.config[input_index]->app_config.interfaces[index].key;
            if (archi_container_extract(archi_process.app.interfaces, key, NULL) == 0)
            {
                archi_log_debug(M, "> unregister_interface('%s')", SAFE(key));

                archi_status_t code = archi_app_remove_interface(&archi_process.app, key);

                if (code != 0)
                    archi_log_error(M, "Couldn't unregister context interface '%s' (error %i).", SAFE(key), code);
            }
        }
    }
}

#undef M

///////////////////////////////////////////////////////////////////////////////

#define M "configure_app()"

static
void
configure_app_log_value(
        const archi_value_t *value)
{
    switch (value->type)
    {
        case ARCHI_VALUE_NULL:
            break;

        case ARCHI_VALUE_FALSE:
            archi_log_debug(M, SPACE SPACE "false");
            break;

        case ARCHI_VALUE_TRUE:
            archi_log_debug(M, SPACE SPACE "true");
            break;

        case ARCHI_VALUE_UINT:
            switch (value->size)
            {
                case sizeof(uint8_t): archi_log_debug(M, SPACE SPACE "%u", *(uint8_t*)value->ptr); break;
                case sizeof(uint16_t): archi_log_debug(M, SPACE SPACE "%u", *(uint16_t*)value->ptr); break;
                case sizeof(uint32_t): archi_log_debug(M, SPACE SPACE "%lu", *(uint32_t*)value->ptr); break;
                case sizeof(uint64_t): archi_log_debug(M, SPACE SPACE "%llu", *(uint64_t*)value->ptr); break;
            }
            break;

        case ARCHI_VALUE_SINT:
            switch (value->size)
            {
                case sizeof(int8_t): archi_log_debug(M, SPACE SPACE "%i", *(int8_t*)value->ptr); break;
                case sizeof(int16_t): archi_log_debug(M, SPACE SPACE "%i", *(int16_t*)value->ptr); break;
                case sizeof(int32_t): archi_log_debug(M, SPACE SPACE "%li", *(int32_t*)value->ptr); break;
                case sizeof(int64_t): archi_log_debug(M, SPACE SPACE "%lli", *(int64_t*)value->ptr); break;
            }
            break;

        case ARCHI_VALUE_FLOAT:
            switch (value->size)
            {
                case sizeof(float): archi_log_debug(M, SPACE SPACE "%g", *(float*)value->ptr); break;
                case sizeof(double): archi_log_debug(M, SPACE SPACE "%g", *(double*)value->ptr); break;
                case sizeof(long double): archi_log_debug(M, SPACE SPACE "%lg", *(long double*)value->ptr); break;
            }
            break;

        case ARCHI_VALUE_STRING:
            archi_log_debug(M, SPACE SPACE "%s", value->ptr);
            break;

        case ARCHI_VALUE_DATA:
            {
                unsigned char *mem = value->ptr;

                switch (value->size)
                {
                    case 0: break;
                    case 1: archi_log_debug(M, SPACE SPACE "%02x",
                                    mem[0]); break;
                    case 2: archi_log_debug(M, SPACE SPACE "%02x%02x",
                                    mem[0], mem[1]); break;
                    case 3: archi_log_debug(M, SPACE SPACE "%02x%02x %02x",
                                    mem[0], mem[1], mem[2]); break;
                    case 4: archi_log_debug(M, SPACE SPACE "%02x%02x %02x%02x",
                                    mem[0], mem[1], mem[2], mem[3]); break;
                    case 5: archi_log_debug(M, SPACE SPACE "%02x%02x %02x%02x %02x",
                                    mem[0], mem[1], mem[2], mem[3], mem[4]); break;
                    case 6: archi_log_debug(M, SPACE SPACE "%02x%02x %02x%02x %02x%02x",
                                    mem[0], mem[1], mem[2], mem[3], mem[4], mem[5]); break;
                    case 7: archi_log_debug(M, SPACE SPACE "%02x%02x %02x%02x %02x%02x %02x",
                                    mem[0], mem[1], mem[2], mem[3], mem[4], mem[5], mem[6]); break;
                    default: archi_log_debug(M, SPACE SPACE "%02x%02x %02x%02x %02x%02x %02x%02x",
                                     mem[0], mem[1], mem[2], mem[3], mem[4], mem[5], mem[6], mem[7]); break;
                }
            }

        default:
            break;
    }
}

static
ARCHI_LIST_ACT_FUNC(configure_app_log_node)
{
    (void) position;
    (void) data;

    const archi_list_node_named_value_t *vnode = (const archi_list_node_named_value_t*)node;

    const char *value_type;
    switch (vnode->value.type)
    {
#define VALUE_TYPE(v) case ARCHI_VALUE_##v: value_type = #v; break

        VALUE_TYPE(NULL);

        VALUE_TYPE(FALSE);
        VALUE_TYPE(TRUE);

        VALUE_TYPE(UINT);
        VALUE_TYPE(SINT);
        VALUE_TYPE(FLOAT);

        VALUE_TYPE(STRING);
        VALUE_TYPE(DATA);

        VALUE_TYPE(NODE);
        VALUE_TYPE(LIST);

        VALUE_TYPE(FUNCTION);

#undef VALUE_TYPE

        default: value_type = "unknown";
    }

    archi_log_debug(M, SPACE "%s = <ptr = %p, size = %u, num_of = %u, type = %s>",
            vnode->base.name, vnode->value.ptr, (unsigned)vnode->value.size, (unsigned)vnode->value.num_of, value_type);
    if (vnode->value.ptr != NULL)
        configure_app_log_value(&vnode->value);

    return 0;
}

void
configure_app(
        size_t index)
{
    if (archi_process.config[index]->app_config.steps == NULL)
        return;

    archi_log_debug(M, "Performing %u configuration steps...",
            (unsigned)archi_process.config[index]->app_config.num_steps);

    for (size_t i = 0; i < archi_process.config[index]->app_config.num_steps; i++)
    {
        archi_app_config_step_t step = archi_process.config[index]->app_config.steps[i];

        switch (step.type)
        {
            case ARCHI_APP_CONFIG_STEP_INIT:
                {
                    archi_log_debug(M, "> [%u] %s = %s()", (unsigned)i,
                            SAFE(step.key), SAFE(step.as_init.interface_key));

                    archi_list_t list = {.head = (archi_list_node_t*)step.as_init.config};
                    archi_list_traverse(&list, NULL, NULL, configure_app_log_node, NULL, true, 0, NULL);
                }
                break;

            case ARCHI_APP_CONFIG_STEP_FINAL:
                archi_log_debug(M, "> [%u] free(%s)", (unsigned)i, SAFE(step.key));
                break;

            case ARCHI_APP_CONFIG_STEP_SET:
                {
                    const char *value_type;
                    switch (step.as_set.value->type)
                    {
#define VALUE_TYPE(v) case ARCHI_VALUE_##v: value_type = #v; break

                        VALUE_TYPE(NULL);

                        VALUE_TYPE(FALSE);
                        VALUE_TYPE(TRUE);

                        VALUE_TYPE(UINT);
                        VALUE_TYPE(SINT);
                        VALUE_TYPE(FLOAT);

                        VALUE_TYPE(STRING);
                        VALUE_TYPE(DATA);

                        VALUE_TYPE(NODE);
                        VALUE_TYPE(LIST);

                        VALUE_TYPE(FUNCTION);

#undef VALUE_TYPE

                        default: value_type = "unknown";
                    }

                    archi_log_debug(M, "> [%u] %s.%s = <ptr = %p, size = %u, num_of = %u, type = %s>",
                            (unsigned)i, SAFE(step.key), SAFE(step.as_set.slot),
                            step.as_set.value->ptr, step.as_set.value->size, step.as_set.value->num_of, value_type);

                    configure_app_log_value(step.as_set.value);
                }
                break;

            case ARCHI_APP_CONFIG_STEP_ASSIGN:
                if (step.as_assign.source_slot != NULL)
                    archi_log_debug(M, "> [%u] %s.%s = %s.%s", (unsigned)i,
                            SAFE(step.key), SAFE(step.as_assign.slot),
                            SAFE(step.as_assign.source_key), SAFE(step.as_assign.source_slot));
                else
                    archi_log_debug(M, "> [%u] %s.%s = %s", (unsigned)i,
                            SAFE(step.key), SAFE(step.as_assign.slot),
                            SAFE(step.as_assign.source_key));
                break;

            case ARCHI_APP_CONFIG_STEP_ACT:
                {
                    archi_log_debug(M, "> [%u] %s.%s()", (unsigned)i,
                            SAFE(step.key), SAFE(step.as_act.action));

                    archi_list_t list = {.head = (archi_list_node_t*)step.as_act.params};
                    archi_list_traverse(&list, NULL, NULL, configure_app_log_node, NULL, true, 0, NULL);
                }
                break;

            default:
                archi_log_debug(M, "> [%u] unknown_step(%s)", (unsigned)i, SAFE(step.key));
        }

        archi_status_t code = archi_app_do_config_step(&archi_process.app, step);

        if (code != 0)
        {
            archi_log_error(M, "Couldn't do configuration step #%u (error %i).", (unsigned)i, code);
            exit(ARCHI_EXIT_CODE(code));
        }

        archi_process.count.steps++;
    }

    archi_process.count.inputs = index + 1;
    archi_process.count.steps = 0;
}

#undef M
#define M "reset_app()"

void
reset_app(void)
{
    archi_log_debug(M, "Undoing the configuration...");

    for (size_t i = 0; i < archi_process.args.num_inputs; i++)
    {
        size_t input_index = (archi_process.args.num_inputs - 1) - i; // reverse order

        if (input_index > archi_process.count.inputs)
            continue; // this configuration file was never used

        for (size_t j = 0; j < archi_process.config[input_index]->app_config.num_steps; j++)
        {
            size_t index = (archi_process.config[input_index]->app_config.num_steps - 1) - j; // reverse order

            if ((input_index == archi_process.count.inputs) && (index >= archi_process.count.steps))
                continue; // this configuration step was never done

            archi_app_config_step_t step = archi_process.config[input_index]->app_config.steps[index];
            if (archi_app_config_step_undoable(step.type))
            {
                archi_log_debug(M, "> undo_configuration_step(%u, %u, '%s')",
                        (unsigned)input_index, (unsigned)index, step.key);

                archi_status_t code = archi_app_undo_config_step(&archi_process.app, step);

                if (code != 0)
                    archi_log_error(M, "Couldn't undo configuration step #%u (error %i).", (unsigned)index, code);
            }
        }
    }
}

#undef M

