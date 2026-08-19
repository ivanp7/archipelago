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
 * @brief Types for signal sets.
 */

#pragma once
#ifndef _ARCHI_SIGNAL_API_SIGNAL_TYP_H_
#define _ARCHI_SIGNAL_API_SIGNAL_TYP_H_

#ifdef __STDC_NO_ATOMICS__
#  error Atomics are required, but not supported by the compiler.
#endif

#include <stdatomic.h> // for atomic_bool
#include <stddef.h> // for size_t
#include <stdint.h> // for uint32_t
#include <limits.h> // for CHAR_BIT


/**
 * @brief ID numbers of signals.
 */
enum {
    // Interruption events
    ARCHI__SIGINT,      ///< Interruption request.
    ARCHI__SIGQUIT,     ///< Quit request.
    ARCHI__SIGTERM,     ///< Termination request.

    // Process events
    ARCHI__SIGCHLD,     ///< Child stopped or terminated.
    ARCHI__SIGCONT,     ///< Continue if stopped.
    ARCHI__SIGTSTP,     ///< Stop request.

    // Limit exceeding events
    ARCHI__SIGXCPU,     ///< CPU time limit exceeded.
    ARCHI__SIGXFSZ,     ///< File size limit exceeded.

    // Input/output events
    ARCHI__SIGPIPE,     ///< Broken pipe.
    ARCHI__SIGPOLL,     ///< Pollable event.
    ARCHI__SIGURG,      ///< Urgent condition on socket.

    // Timer events
    ARCHI__SIGALRM,     ///< Timer signal from alarm.
    ARCHI__SIGVTALRM,   ///< Virtual alarm clock.
    ARCHI__SIGPROF,     ///< Profiling timer expired.

    // Terminal events
    ARCHI__SIGHUP,      ///< Terminal hangup.
    ARCHI__SIGTTIN,     ///< Terminal input for background process.
    ARCHI__SIGTTOU,     ///< Terminal output for background process.
    ARCHI__SIGWINCH,    ///< Terminal resized.

    // User-defined events
    ARCHI__SIGUSR1,     ///< User-defined signal 1.
    ARCHI__SIGUSR2,     ///< User-defined signal 2.

    // Number of standard signals
    ARCHI_SIGNAL_NUMBER,    ///< Number of standard (non real-time) signals.

    // User-defined real-time events
    ARCHI__SIGRTMIN = ARCHI_SIGNAL_NUMBER,  ///< Minimum real-time signal.
};

/**
 * @brief Get number of POSIX real‑time signals supported by the system.
 *
 * POSIX real‑time signals occupy the continuous range
 * [SIGRTMIN ... SIGRTMAX]. This function computes the total number
 * of distinct real‑time signals available on the current platform.
 *
 * @return Number of POSIX real‑time signals supported by the system.
 */
int
archi_signal_number_realtime(void);

/**
 * @brief Number of POSIX real‑time signals supported by the system.
 */
#define ARCHI_SIGNAL_NUMBER_REALTIME    (archi_signal_number_realtime())

/**
 * @brief Maximum real-time signal.
 */
#define ARCHI__SIGRTMAX     (ARCHI__SIGRTMIN + ARCHI_SIGNAL_NUMBER_REALTIME - 1)

/**
 * @brief Names of signals.
 *
 * This array stores names of the standard signals plus the name of minimum real-time signal (SIGRTMIN).
 */
extern
const char*
archi_signal_name[ARCHI_SIGNAL_NUMBER + 1];

/**
 * @brief Name of the maximum real-time signal (SIGRTMAX).
 */
extern
const char*
archi_signal_name__SIGRTMAX;

/*****************************************************************************/

/**
 * @typedef archi_signal_set_t
 * @brief Component mask of a POSIX signal set.
 *
 * This mask represents a part of a set of signals.
 * Each bit flag corresponds to a signal (encoded by position);
 * setting a flag to 1 means that the signal is in the set.
 */
typedef uint32_t archi_signal_set_mask_t;

/**
 * @def ARCHI_SIGNAL_SET_MASK_NUM_BITS
 * @brief Number of bits in a component mask in a POSIX signal set.
 */
#define ARCHI_SIGNAL_SET_MASK_NUM_BITS  (sizeof(archi_signal_set_mask_t) * CHAR_BIT)

/**
 * @def ARCHI_SIGNAL_SET_NUM_MASKS
 * @brief Number of components masks in a POSIX signal set.
 */
#define ARCHI_SIGNAL_SET_NUM_MASKS                          \
    ((ARCHI_SIGNAL_NUMBER + ARCHI_SIGNAL_NUMBER_REALTIME    \
      + (ARCHI_SIGNAL_SET_MASK_NUM_BITS - 1)) / ARCHI_SIGNAL_SET_MASK_NUM_BITS)

/**
 * @struct archi_signal_set_t
 * @brief Set of POSIX signals.
 *
 * This is a pointer to array of exactly ARCHI_SIGNAL_SET_NUM_MASKS masks.
 */
typedef archi_signal_set_mask_t *archi_signal_set_t;

/**
 * @struct archi_signal_set_const_t
 * @brief Set of POSIX signals (read-only).
 *
 * This is a constant version of archi_signal_set_t.
 */
typedef const archi_signal_set_mask_t *archi_signal_set_const_t;

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
 *   - Set to true when a signal arrives (only if it was added to the signal set on initialization).
 *   - Manually reset by user code to detect subsequent occurrences.
 *
 * These atomic flags allow safe, lock‑free notification of signal
 * delivery in multithreaded environment.
 *
 * Note on real‑time signals:
 *   - POSIX defines real‑time signals in the range [SIGRTMIN .. SIGRTMAX].
 *   - The flexible array member rt_signal[] holds one atomic_bool per real‑time signal.
 *     rt_signal[0] corresponds to SIGRTMIN, rt_signal[1] to SIGRTMIN+1,
 *     and so on, up to index (SIGRTMAX − SIGRTMIN).
 */
typedef struct archi_signal_flags {
    archi_signal_flag_t signal[ARCHI_SIGNAL_NUMBER]; ///< Standard signals.
    archi_signal_flag_t rt_signal[]; ///< Real-time signals (SIGRTMIN+index).
} archi_signal_flags_t;

#endif // _ARCHI_SIGNAL_API_SIGNAL_TYP_H_

