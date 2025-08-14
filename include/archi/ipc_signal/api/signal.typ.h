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
 * @brief Types for signal sets.
 */

#pragma once
#ifndef _ARCHI_IPC_SIGNAL_API_SIGNAL_TYP_H_
#define _ARCHI_IPC_SIGNAL_API_SIGNAL_TYP_H_

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

#endif // _ARCHI_IPC_SIGNAL_API_SIGNAL_TYP_H_

