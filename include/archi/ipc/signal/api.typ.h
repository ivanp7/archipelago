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
#ifndef _ARCHI_IPC_SIGNAL_API_TYP_H_
#define _ARCHI_IPC_SIGNAL_API_TYP_H_

#ifdef __STDC_NO_ATOMICS__
#  error Atomics are required, but not supported by the compiler.
#endif

#include <stdatomic.h>
#include <stdbool.h>

/**
 * @struct archi_signal_watch_set_t
 * @brief Mask of POSIX signals to watch and handle.
 *
 * This structure represents a set of signals the application
 * wishes to monitor. Each boolean flag corresponds to a signal;
 * setting a flag to true requests that the signal be monitored.
 * Flags set to false leave the signal ignored by the application
 * (enabling default handling behavior).
 *
 * Full size of the structure is ARCHI_SIGNAL_WATCH_SET_SIZEOF bytes.
 *
 * Note on real‑time signals:
 *   - POSIX defines real‑time signals in the range [SIGRTMIN .. SIGRTMAX].
 *   - The flexible array member f_SIGRTMIN[] holds one bool per real‑time signal.
 *     f_SIGRTMIN[0] corresponds to SIGRTMIN, f_SIGRTMIN[1] to SIGRTMIN+1,
 *     and so on, up to index (SIGRTMAX − SIGRTMIN).
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

    // Limit exceeding events
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
 * @typedef archi_signal_flag_t
 * @brief Signal state flag.
 *
 * A flags is:
 *   - Cleared (false) on initialization.
 *   - Set to true when a signal arrives.
 *   - Manually reset by user code to detect subsequent occurrences.
 */
typedef atomic_bool archi_signal_flag_t;

/**
 * @struct archi_signal_flags_t
 * @brief Atomic flags reflecting states of POSIX signals.
 *
 * This structure represents a set of states of POSIX signals.
 * Each atomic boolean flag corresponds to a signal; flags are:
 *   - Cleared (false) on initialization.
 *   - Set to true when a signal arrives (only if it was added to the watch set on initialization).
 *   - Manually reset by user code to detect subsequent occurrences.
 *
 * These atomic flags allow safe, lock‑free notification of signal
 * delivery in multithreaded environment.
 *
 * Full size of the structure is ARCHI_SIGNAL_FLAGS_SIZEOF bytes.
 *
 * Note on real‑time signals:
 *   - POSIX defines real‑time signals in the range [SIGRTMIN .. SIGRTMAX].
 *   - The flexible array member f_SIGRTMIN[] holds one atomic_bool per real‑time signal.
 *     f_SIGRTMIN[0] corresponds to SIGRTMIN, f_SIGRTMIN[1] to SIGRTMIN+1,
 *     and so on, up to index (SIGRTMAX − SIGRTMIN).
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

    // Limit exceeding events
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
 * @def ARCHI_SIGNAL_HANDLER_FUNC(func_name)
 * @brief Declare or define a signal handler function.
 *
 * This macro expands to the exact signature required for a signal
 * handler that the signal management subsystem will invoke whenever
 * a watched signal is caught.
 *
 * @note The handler runs in the context of the dedicated signal management thread
 * (not the async signal handler), so it may perform non-reentrant operations.
 * Signal handlers should avoid blocking operations to prevent delays in signal processing.
 *
 * @warning Never call exit(), quick_exit(), _Exit(), or similar
 *          process-terminating routines from inside a signal handler function,
 *          as it invokes undefined behavior.
 *
 * @param[in]       signo
 *                      Signal number (SIGINT, SIGTERM, etc).
 * @param[in]       siginfo
 *                      Pointer to the OS-provided siginfo_t.
 * @param[in,out]   flags
 *                      Pointer to the signal flags structure (archi_signal_flags_t).
 *                      The handler function is allowed to read and write states of any signals.
 * @param[in,out]   data
 *                      Signal handler function data.
 *
 * @return Whether the corresponding flag of the current signal should be set automatically.
 *   - true: the flag should be set;
 *   - false: the flag should not be set.
 *
 * @see archi_signal_handler_function_t
 */
#define ARCHI_SIGNAL_HANDLER_FUNC(func_name)    bool func_name( \
        int signo, \
        void *const restrict siginfo, \
        archi_signal_flags_t *const restrict flags, \
        void *const restrict data)

/**
 * @brief Function pointer type for a signal handler.
 *
 * Points to a function matching the ARCHI_SIGNAL_HANDLER_FUNC() signature.
 *
 * @see ARCHI_SIGNAL_HANDLER_FUNC
 */
typedef ARCHI_SIGNAL_HANDLER_FUNC((*archi_signal_handler_function_t));

/**
 * @struct archi_signal_handler_t
 * @brief Descriptor for a signal handler.
 *
 * Bundles a signal handler function pointer with an opaque user data pointer.
 * When a watched signal arrives, the signal manager invokes
 * handler->function(signo, siginfo, flags, handler->data).
 */
typedef struct archi_signal_handler {
    archi_signal_handler_function_t function; ///< Signal hander function.
    void *data; ///< Signal hander function data.
} archi_signal_handler_t;

/*****************************************************************************/

/**
 * @brief Parameters for starting the signal management subsystem (archi_signal_management_start()).
 *
 * This structure bundles the configuration needed to launch the
 * dedicated signal management thread. Once started:
 *   - The set of watched signals is fixed (signals->f_* flags).
 *   - The signal handler may be changed later, via archi_signal_management_set_handler().
 */
typedef struct archi_signal_management_start_params {
    archi_signal_watch_set_t *signals;     ///< Signals to watch.
    archi_signal_handler_t signal_handler; ///< Signal handler.
} archi_signal_management_start_params_t;

#endif // _ARCHI_IPC_SIGNAL_API_TYP_H_

