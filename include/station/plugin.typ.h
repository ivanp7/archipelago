/*****************************************************************************
 * Copyright (C) 2020-2024 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Station.                                             *
 *                                                                           *
 *   Station is free software: you can redistribute it and/or modify it      *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Station is distributed in the hope that it will be useful,              *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Station. If not, see <http://www.gnu.org/licenses/>. *
 *****************************************************************************/

/**
 * @file
 * @brief Types for application plugins.
 */

#pragma once
#ifndef _STATION_PLUGIN_TYP_H_
#define _STATION_PLUGIN_TYP_H_

#include <station/fsm.typ.h>
#include <station/parallel.typ.h>

#include <stdint.h>
#include <stdbool.h>

struct station_signal_set;
struct station_opencl_context;

/**
 * @brief Arguments for plugin configuration function.
 */
typedef struct station_plugin_conf_func_args {
    void *cmdline; ///< Pointer to store parsed command line arguments.

    struct station_signal_set *signals; ///< Signals to watch.

    station_threads_number_t num_threads; ///< Number of parallel processing threads to create.

    bool opencl_is_used; ///< Whether OpenCL is used and should be initialized.

    bool sdl_is_used; ///< Whether SDL is used and should be initialized.
    uint32_t sdl_init_flags; ///< Flags to pass to SDL_Init() call.
} station_plugin_conf_func_args_t;

/**
 * @brief Input arguments for plugin initialization function.
 */
typedef struct station_plugin_init_func_inputs {
    void *cmdline; ///< Parsed command line arguments.

    struct station_signal_set *signals; ///< States of signals.

    station_parallel_processing_context_t *parallel_processing_context; ///< Parallel processing context.
    struct station_opencl_context *opencl_context; ///< OpenCL context.

    bool sdl_is_available; ///< Whether SDL is available and its subsystems was initialized.
} station_plugin_init_func_inputs_t;

/**
 * @brief Output arguments for plugin initialization function.
 */
typedef struct station_plugin_init_func_outputs {
    void *plugin_resources; ///< Plugin resources.

    station_state_t fsm_initial_state; ///< Initial state of finite state machine.
    void *fsm_data; ///< Finite state machine data (argument passed to all state functions).
} station_plugin_init_func_outputs_t;

/**
 * @brief Plugin help function.
 *
 * This function is intended to do nothing besides displaying plugin usage help.
 *
 * It is allowed to terminate application in this function
 * by using exit() or quick_exit().
 */
typedef void (*station_plugin_help_func_t)(
        int argc,    ///< [in] Number of command line arguments.
        char *argv[] ///< [in] Command line arguments.
);

/**
 * @brief Plugin configuration function.
 *
 * @warning Do not create any threads in this function,
 * or initialize any resources that can create threads,
 * otherwise signal management will break!
 *
 * This function is intended for parsing command line arguments and
 * configuring features offered by the application.
 *
 * It is allowed to terminate application in this function
 * by using exit() or quick_exit().
 */
typedef void (*station_plugin_conf_func_t)(
        station_plugin_conf_func_args_t *args, ///< [in,out] Structure with function arguments.
        int argc,    ///< [in] Number of command line arguments.
        char *argv[] ///< [in] Command line arguments.
);

/**
 * @brief Plugin initialization function.
 *
 * This function is intended to allocate and initialize plugin resources,
 * copy context pointers for the later use, configure finite state machine.
 *
 * It is allowed to terminate application in this function
 * by using exit() or quick_exit().
 */
typedef void (*station_plugin_init_func_t)(
        const station_plugin_init_func_inputs_t *inputs, ///< [in] Structure with function inputs.
        station_plugin_init_func_outputs_t *outputs      ///< [out] Structure with function outputs.
);

/**
 * @brief Plugin finalization function.
 *
 * This function is intended to release plugin resources.
 *
 * @warning Do not use exit() or quick_exit() in this function,
 * otherwise the behavior is undefined!
 *
 * @return Application exit code (ignored if application is terminated by exit() or quick_exit()).
 */
typedef int (*station_plugin_final_func_t)(
        void *plugin_resources, ///< [in] Plugin resources created by the plugin initialization function.
        bool quick ///< [in] Whether the application is terminated by quick_exit() call.
);

/**
 * @brief Plugin format.
 */
typedef struct station_plugin_format {
    uint32_t signature; ///< Value uniquely identifying plugin format.
    uint32_t version;   ///< Value determining application-plugin compatibility.
} station_plugin_format_t;

/**
 * @brief Plugin vtable.
 */
typedef struct station_plugin_vtable {
    const char *name; ///< Plugin name.

    station_plugin_help_func_t help_fn;   ///< Pointer to plugin help function.

    station_plugin_conf_func_t conf_fn;   ///< Pointer to plugin configuration function.
    station_plugin_init_func_t init_fn;   ///< Pointer to plugin initialization function.
    station_plugin_final_func_t final_fn; ///< Pointer to plugin finalization function.
} station_plugin_vtable_t;

#endif // _STATION_PLUGIN_TYP_H_

