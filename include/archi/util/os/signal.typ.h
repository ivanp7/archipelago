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
 * @brief Types for signal management.
 */

#pragma once
#ifndef _ARCHI_UTIL_OS_SIGNAL_TYP_H_
#define _ARCHI_UTIL_OS_SIGNAL_TYP_H_

#include <stdatomic.h>
#include <stdbool.h>

/**
 * @brief Flags designating which signals need to be watched.
 */
typedef struct archi_signal_watch_set {
    // Interruption events
    bool f_SIGINT;      ///< Interruption request.
    bool f_SIGQUIT;     ///< Quit request.
    bool f_SIGTERM;     ///< Termination request.

    // Process events
    bool f_SIGCHLD;     ///< Child stopped or terminated.
    bool f_SIGCONT;     ///< Continue if stopped.
    bool f_SIGTSTP;     ///< Stop request.
    bool f_SIGXCPU;     ///< CPU time limit exceeded.
    bool f_SIGXFSZ;     ///< File size limit exceeded.

    // Input/output events
    bool f_SIGPIPE;     ///< Broken pipe.
    bool f_SIGPOLL;     ///< Pollable event.
    bool f_SIGURG;      ///< Urgent condition on socket.

    // Timer events
    bool f_SIGALRM;     ///< Timer signal from alarm.
    bool f_SIGVTALRM;   ///< Virtual alarm clock.
    bool f_SIGPROF;     ///< Profiling timer expired.

    // Terminal events
    bool f_SIGHUP;      ///< Terminal hangup.
    bool f_SIGTTIN;     ///< Terminal input for background process.
    bool f_SIGTTOU;     ///< Terminal output for background process.
    bool f_SIGWINCH;    ///< Terminal resized.

    // User-defined events
    bool f_SIGUSR1;     ///< User-defined signal 1.
    bool f_SIGUSR2;     ///< User-defined signal 2.

    // User-defined real-time events
    bool f_SIGRTMIN[];  ///< Real-time signals SIGRTMIN+index, where index <= SIGRTMAX - SIGRTMIN.
} archi_signal_watch_set_t;

/*****************************************************************************/

/**
 * @brief Signal status flag.
 */
typedef atomic_bool archi_signal_flag_t;

/**
 * @brief Flags designating states of signals.
 *
 * All flags are initialized clear.
 * Incoming signals set corresponding flags.
 * User code can reset flags to catch signals more than once.
 */
typedef struct archi_signal_flags {
    // Interruption events
    archi_signal_flag_t f_SIGINT;       ///< Interruption request.
    archi_signal_flag_t f_SIGQUIT;      ///< Quit request.
    archi_signal_flag_t f_SIGTERM;      ///< Termination request.

    // Process events
    archi_signal_flag_t f_SIGCHLD;      ///< Child stopped or terminated.
    archi_signal_flag_t f_SIGCONT;      ///< Continue if stopped.
    archi_signal_flag_t f_SIGTSTP;      ///< Stop request.
    archi_signal_flag_t f_SIGXCPU;      ///< CPU time limit exceeded.
    archi_signal_flag_t f_SIGXFSZ;      ///< File size limit exceeded.

    // Input/output events
    archi_signal_flag_t f_SIGPIPE;      ///< Broken pipe.
    archi_signal_flag_t f_SIGPOLL;      ///< Pollable event.
    archi_signal_flag_t f_SIGURG;       ///< Urgent condition on socket.

    // Timer events
    archi_signal_flag_t f_SIGALRM;      ///< Timer signal from alarm.
    archi_signal_flag_t f_SIGVTALRM;    ///< Virtual alarm clock.
    archi_signal_flag_t f_SIGPROF;      ///< Profiling timer expired.

    // Terminal events
    archi_signal_flag_t f_SIGHUP;       ///< Terminal hangup.
    archi_signal_flag_t f_SIGTTIN;      ///< Terminal input for background process.
    archi_signal_flag_t f_SIGTTOU;      ///< Terminal output for background process.
    archi_signal_flag_t f_SIGWINCH;     ///< Terminal resized.

    // User-defined events
    archi_signal_flag_t f_SIGUSR1;      ///< User-defined signal 1.
    archi_signal_flag_t f_SIGUSR2;      ///< User-defined signal 2.

    // User-defined real-time events
    archi_signal_flag_t f_SIGRTMIN[];   ///< Real-time signals SIGRTMIN+index, where index <= SIGRTMAX - SIGRTMIN.
} archi_signal_flags_t;

/*****************************************************************************/

/**
 * @brief Declare/define signal handler function.
 *
 * This function is called from the signal management thread every time a signal is caught.
 *
 * @warning Calling exit() or quick_exit from this function invokes undefined behavior.
 *
 * @return True if the signal manager should set the signal flag, false if it should ignore the signal.
 */
#define ARCHI_SIGNAL_HANDLER_FUNC(name) bool name( \
        int signo, /* Signal number. */ \
        void *const restrict siginfo, /* Pointer to siginfo_t data structure. */ \
        archi_signal_flags_t *const restrict signals, /* Signal flags. */ \
        void *const restrict data) /* Signal handler data. */

/**
 * @brief Signal handler function.
 */
typedef ARCHI_SIGNAL_HANDLER_FUNC((*archi_signal_handler_function_t));

/**
 * @brief Signal handler.
 */
typedef struct archi_signal_handler {
    archi_signal_handler_function_t function; ///< Signal hander function.
    void *data; ///< Signal hander function data.
} archi_signal_handler_t;

/*****************************************************************************/

/**
 * @brief Signal management configuration.
 */
typedef struct archi_signal_management_config {
    archi_signal_watch_set_t *signals;    ///< Signals to watch.
    archi_signal_handler_t signal_handler; ///< Signal handler.
} archi_signal_management_config_t;

#endif // _ARCHI_UTIL_OS_SIGNAL_TYP_H_

