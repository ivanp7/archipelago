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
#include <errno.h> // for error codes and errno

/*****************************************************************************/

#define M "main()" // module name for logging

#define SAFE(str) ((str) != NULL ? (str) : "[NULL]")

/*****************************************************************************/

static
struct {
    const archi_process_config_shm_t *config; ///< Configuration of the process in shared memory.

    struct archi_signal_management_context *signal_management; ///< Signal management context.

    archi_fsm_t fsm; ///< Finite state machine.

    archi_application_t app; ///< Application instance.
    archi_list_container_data_t app_libraries_data;  ///< Data for the libraries container.
    archi_list_container_data_t app_interfaces_data; ///< Data for the context interfaces container.
    archi_list_container_data_t app_contexts_data;   ///< Data for the contexts container.
    archi_context_t app_context_signal; ///< Wrapper of application signal management context.
    archi_context_t app_context_fsm;    ///< Wrapper of application finite state machine.

    struct {
        size_t builtin_interfaces;
        size_t builtin_contexts;
        size_t libraries;
        size_t interfaces;
        size_t steps;
    } counter;
} archi_process;

static
void
archi_exit_cleanup(void);

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
    archi_args_t args;
    switch (archi_args_parse(&args, argc, argv))
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

    // Display application logo
    archi_print(ARCHI_COLOR_RESET "\n");
    if (!args.no_logo)
        archi_print(ARCHI_COLOR_FG_BRI_WHITE "%s" ARCHI_COLOR_RESET "\n\n", ARCHI_PELAGO_LOGO);

    // Exit if there is nothing to do
    if (args.file == NULL)
        return 0;

    // Set cleanup function
    atexit(archi_exit_cleanup);

    ///////////////////////
    // Map shared memory //
    ///////////////////////

    {
        archi_log_debug(M, "(ini) Opening memory-mapped configuration file '%s'...", args.file);

        int fd = archi_shm_open_file(args.file, true, false);
        if (fd == -1)
        {
            archi_log_error(M, "Couldn't open memory-mapped configuration file '%s': %s.", args.file, strerror(errno));
            return ARCHI_EXIT_CODE(ARCHI_ERROR_FILE);
        }

        archi_log_debug(M, "(ini) Mapping memory-mapped configuration file '%s'...", args.file);

        archi_process.config = (const archi_process_config_shm_t*)archi_shm_map(fd, true, false, false, 0);
        if (archi_process.config == NULL)
        {
            archi_log_error(M, "Couldn't map memory-mapped configuration file '%s': %s.", args.file, strerror(errno));

            archi_shm_close(fd);
            return ARCHI_EXIT_CODE(ARCHI_ERROR_MAP);
        }

        archi_log_debug(M, "(ini) Closing memory-mapped configuration file...");

        if (!archi_shm_close(fd))
            archi_log_warning(M, "Couldn't close memory-mapped configuration file '%s': %s.", args.file, strerror(errno));
    }

    /////////////////////////////
    // Start signal management //
    /////////////////////////////

    if (archi_process.config->signal_watch_set != NULL)
    {
        archi_log_debug(M, "(ini) Starting signal management...");

#define LOG_SIGNAL(signal) do { \
        if (archi_process.config->signal_watch_set->f_##signal) \
            archi_log_debug(M, " - %s", #signal); \
} while (0)

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
            if (archi_process.config->signal_watch_set->f_SIGRTMIN[i])
                archi_log_debug(M, " - SIGRTMIN+%u", (unsigned)i);
        }

        archi_process.signal_management = archi_signal_management_start(
                archi_process.config->signal_watch_set, (archi_signal_handler_t){0});

        if (archi_process.signal_management == NULL)
        {
            archi_log_error(M, "Couldn't start signal management: %s.", args.file, strerror(errno));
            return ARCHI_EXIT_CODE(ARCHI_ERROR_SIGNAL);
        }
    }

    ////////////////////////////
    // Initialize application //
    ////////////////////////////

    archi_log_debug(M, "(ini) Initializing the application...");

    archi_process.app = (archi_application_t){
        .libraries = {
            .data = &archi_process.app_libraries_data,
            .interface = &archi_list_container_interface,
        },
        .interfaces = {
            .data = &archi_process.app_interfaces_data,
            .interface = &archi_list_container_interface,
        },
        .contexts = {
            .data = &archi_process.app_contexts_data,
            .interface = &archi_list_container_interface,
        },
    };

    archi_process.app_libraries_data =
        archi_process.app_interfaces_data =
        archi_process.app_contexts_data =
        (archi_list_container_data_t){
            .insert_to_head = false,
            .traverse_from_head = false,
        };

    archi_process.app_context_signal = (archi_context_t){
        .handle = archi_process.signal_management,
        .interface = &archi_app_signal_interface,
    };
    archi_process.app_context_fsm = (archi_context_t){
        .handle = &archi_process.fsm,
        .interface = &archi_app_fsm_interface,
    };

    // Register built-in interfaces
    for (size_t i = 0; i < archi_builtin_interfaces_num_of; i++)
    {
        archi_log_debug(M, " - adding built-in context interface '%s'...",
                archi_builtin_interfaces_aliases[i]);

        code = archi_container_insert(archi_process.app.interfaces,
                archi_builtin_interfaces_aliases[i], (void*)archi_builtin_interfaces[i]);

        if (code != 0)
        {
            archi_log_error(M, "Couldn't add built-in context interface '%s' (error %i).",
                    (unsigned)i, archi_builtin_interfaces_aliases[i], code);
            return ARCHI_EXIT_CODE(code);
        }

        archi_process.counter.builtin_interfaces++;
    }

    // Register built-in contexts
    if (archi_process.signal_management != NULL)
    {
        archi_log_debug(M, " - adding built-in context '%s'...", ARCHI_APP_SIGNAL_ALIAS);

        code = archi_container_insert(archi_process.app.contexts,
                ARCHI_APP_SIGNAL_ALIAS, &archi_process.app_context_signal);

        if (code != 0)
        {
            archi_log_error(M, "Couldn't add built-in context '%s' (error %i).",
                    ARCHI_APP_SIGNAL_ALIAS, code);
            return ARCHI_EXIT_CODE(code);
        }

        archi_process.counter.builtin_contexts++;
    }

    {
        archi_log_debug(M, " - adding built-in context '%s'...", ARCHI_APP_FSM_ALIAS);

        code = archi_container_insert(archi_process.app.contexts,
                ARCHI_APP_FSM_ALIAS, &archi_process.app_context_fsm);

        if (code != 0)
        {
            archi_log_error(M, "Couldn't add built-in context '%s' (error %i).",
                    ARCHI_APP_FSM_ALIAS, code);
            return ARCHI_EXIT_CODE(code);
        }

        archi_process.counter.builtin_contexts++;
    }

    // Load libraries
    if (archi_process.config->app_config.libraries != NULL)
    {
        for (size_t i = 0; i < archi_process.config->app_config.num_libraries; i++)
        {
            archi_log_debug(M, " - loading shared library '%s'...",
                    SAFE(archi_process.config->app_config.libraries[i].key));

            code = archi_app_add_library(&archi_process.app, archi_process.config->app_config.libraries[i]);
            if (code != 0)
            {
                archi_log_error(M, "Couldn't load shared library '%s' (error %i).",
                        SAFE(archi_process.config->app_config.libraries[i].key), code);
                return ARCHI_EXIT_CODE(code);
            }
        }

        archi_process.counter.libraries++;
    }

    // Get context interfaces
    if (archi_process.config->app_config.interfaces != NULL)
    {
        for (size_t i = 0; i < archi_process.config->app_config.num_interfaces; i++)
        {
            archi_log_debug(M, " - adding context interface '%s'...",
                    SAFE(archi_process.config->app_config.interfaces[i].key));

            code = archi_app_add_interface(&archi_process.app, archi_process.config->app_config.interfaces[i]);
            if (code != 0)
            {
                archi_log_error(M, "Couldn't extract context interface '%s' (error %i).",
                        SAFE(archi_process.config->app_config.interfaces[i].key), code);
                return ARCHI_EXIT_CODE(code);
            }
        }

        archi_process.counter.interfaces++;
    }

    // Configure the application
    if (archi_process.config->app_config.steps != NULL)
    {
        for (size_t i = 0; i < archi_process.config->app_config.num_steps; i++)
        {
            archi_app_config_step_t step = archi_process.config->app_config.steps[i];

            switch (step.type)
            {
                case ARCHI_APP_CONFIG_STEP_INIT:
                    archi_log_debug(M, " - [%u] initializing context '%s' of type '%s'...",
                            (unsigned)i, SAFE(step.key), SAFE(step.as_init.interface_key));
                    break;

                case ARCHI_APP_CONFIG_STEP_FINAL:
                    archi_log_debug(M, " - [%u] finalizing context '%s'...", (unsigned)i, SAFE(step.key));
                    break;

                case ARCHI_APP_CONFIG_STEP_SET:
                    archi_log_debug(M, " - [%u] setting slot '%s' of context '%s' to value of type %u...",
                            (unsigned)i, SAFE(step.as_set.slot), SAFE(step.key), step.as_set.value->type);
                    break;

                case ARCHI_APP_CONFIG_STEP_ASSIGN:
                    if (step.as_assign.source_slot != NULL)
                        archi_log_debug(M, " - [%u] assigning slot '%s' of context '%s' to slot '%s' of context '%s'...",
                                (unsigned)i, SAFE(step.as_assign.source_slot), SAFE(step.as_assign.source_key),
                                SAFE(step.as_assign.slot), SAFE(step.key));
                    else
                        archi_log_debug(M, " - [%u] assigning context '%s' to slot '%s' of context '%s'...",
                                (unsigned)i, SAFE(step.as_assign.source_key),
                                SAFE(step.as_assign.slot), SAFE(step.key));
                    break;

                case ARCHI_APP_CONFIG_STEP_ACT:
                    archi_log_debug(M, " - [%u] invoking action '%s' of context '%s'...",
                            (unsigned)i, SAFE(step.as_act.action), SAFE(step.key));
                    break;
            }

            code = archi_app_do_config_step(&archi_process.app, step);
            if (code != 0)
            {
                archi_log_error(M, "Couldn't do configuration step #%u (error %i).", (unsigned)i, code);
                return ARCHI_EXIT_CODE(code);
            }

            archi_process.counter.steps++;
        }
    }

    /////////////////////////////
    // Execute the application //
    /////////////////////////////

    archi_log_debug(M, "(exe) Executing the finite state machine...");
    code = archi_fsm_execute(archi_process.fsm);

    return ARCHI_EXIT_CODE(code);
}

void
archi_exit_cleanup(void) // will be called on exit(), or if main() returns
{
    archi_status_t code;

    //////////////////////////////
    // Finalize the application //
    //////////////////////////////

    if (archi_process.config != NULL)
    {
        archi_log_debug(M, "(fin) Finalizing the application...");

        // Reset the application
        for (size_t i = 0; i < archi_process.counter.steps; i++)
        {
            size_t j = (archi_process.counter.steps - 1) - i;

            archi_log_debug(M, " - undoing configuration step #%u...", (unsigned)j);

            code = archi_app_undo_config_step(&archi_process.app, archi_process.config->app_config.steps[j]);
            if (code != 0)
                archi_log_error(M, "Couldn't undo configuration step #%u (error %i).", (unsigned)i, code);
        }

        // Remove context interfaces
        for (size_t i = 0; i < archi_process.counter.interfaces; i++)
        {
            size_t j = (archi_process.counter.interfaces - 1) - i;

            archi_log_debug(M, " - removing context interface '%s'...",
                    SAFE(archi_process.config->app_config.interfaces[j].key));

            code = archi_app_remove_interface(&archi_process.app, archi_process.config->app_config.interfaces[j].key);
            if (code != 0)
                archi_log_error(M, "Couldn't remove context interface '%s' (error %i).",
                        SAFE(archi_process.config->app_config.interfaces[j].key), code);
        }

        // Unload libraries
        for (size_t i = 0; i < archi_process.counter.libraries; i++)
        {
            size_t j = (archi_process.counter.libraries - 1) - i;

            archi_log_debug(M, " - unloading shared library '%s'...",
                    SAFE(archi_process.config->app_config.libraries[j].key));

            code = archi_app_remove_library(&archi_process.app, archi_process.config->app_config.libraries[j].key);
            if (code != 0)
                archi_log_error(M, "Couldn't unload shared library '%s' (error %i).",
                        SAFE(archi_process.config->app_config.libraries[j].key), code);
        }

        // Remove built-in contexts
        {
            archi_log_debug(M, " - removing built-in context '%s'...", ARCHI_APP_FSM_ALIAS);

            code = archi_container_remove(archi_process.app.contexts, ARCHI_APP_FSM_ALIAS, NULL);
            if (code != 0)
                archi_log_error(M, "Couldn't remove built-in context '%s' (error %i).",
                        ARCHI_APP_FSM_ALIAS, code);
        }
        {
            archi_log_debug(M, " - removing built-in context '%s'...", ARCHI_APP_SIGNAL_ALIAS);

            code = archi_container_remove(archi_process.app.contexts, ARCHI_APP_SIGNAL_ALIAS, NULL);
            if (code != 0)
                archi_log_error(M, "Couldn't remove built-in context '%s' (error %i).",
                        ARCHI_APP_SIGNAL_ALIAS, code);
        }

        // Remove built-in context interfaces
        for (size_t i = 0; i < archi_process.counter.builtin_interfaces; i++)
        {
            size_t j = (archi_process.counter.builtin_interfaces - 1) - i;

            archi_log_debug(M, " - removing built-in context interface '%s'...",
                    archi_builtin_interfaces_aliases[j]);

            code = archi_container_remove(archi_process.app.interfaces,
                    archi_builtin_interfaces_aliases[j], NULL);
            if (code != 0)
                archi_log_error(M, "Couldn't remove built-in context interface '%s' (error %i).",
                        archi_builtin_interfaces_aliases[j], code);
        }
    }

    ////////////////////////////
    // Stop signal management //
    ////////////////////////////

    if (archi_process.signal_management != NULL)
    {
        archi_log_debug(M, "(fin) Stopping signal management...");
        archi_signal_management_stop(archi_process.signal_management);
    }

    /////////////////////////
    // Unmap shared memory //
    /////////////////////////

    if (archi_process.config != NULL)
    {
        archi_log_debug(M, "(ini) Unmapping memory-mapped configuration file...");
        if (!archi_shm_unmap((archi_shm_header_t*)archi_process.config))
            archi_log_error(M, "Couldn't unmap memory-mapped configuration file: %s.", strerror(errno));
    }
}

