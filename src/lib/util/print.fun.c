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
 * @brief Functions for printing formatted messages and logging.
 */

#include "archi/util/print.fun.h"
#include "archi/util/print.def.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#ifndef __STDC_NO_ATOMICS__
#  include <stdatomic.h>

static
atomic_flag archi_print_spinlock_flag = ATOMIC_FLAG_INIT;
#endif

/*****************************************************************************/

void
archi_print(
        const char *format,
        ...)
{
    if (format == NULL)
        return;

    va_list args;
    va_start(args, format);

#ifndef __STDC_NO_ATOMICS__
    while (atomic_flag_test_and_set_explicit(&archi_print_spinlock_flag, memory_order_acquire))
        ; // spin until the lock is acquired
#endif

    vfprintf(stderr, format, args);

#ifndef __STDC_NO_ATOMICS__
    atomic_flag_clear_explicit(&archi_print_spinlock_flag, memory_order_release); // release the lock
#endif

    va_end(args);
}

/*****************************************************************************/

static
int archi_log_verbosity_level = ARCHI_LOG_VERBOSITY_MAX;

int
archi_log_verbosity(void)
{
    return archi_log_verbosity_level;
}

void
archi_log_set_verbosity(
        int verbosity_level)
{
    static bool verbosity_set = false;

    if (!verbosity_set)
    {
        if (verbosity_level < 0)
            verbosity_level = 0;
        else if (verbosity_level > ARCHI_LOG_VERBOSITY_MAX)
            verbosity_level = ARCHI_LOG_VERBOSITY_MAX;

        archi_log_verbosity_level = verbosity_level;
        verbosity_set = true;
    }
}

/*****************************************************************************/

static
struct timespec archi_log_start_timespec;

void
archi_log_start_time(
        struct timespec *ts)
{
    if (ts != NULL)
        *ts = archi_log_start_timespec;
}

void
archi_log_set_start_time(void)
{
    static bool start_time_set = false;

    if (!start_time_set)
    {
        timespec_get(&archi_log_start_timespec, TIME_UTC);
        start_time_set = true;
    }
}

void
archi_log_elapsed_time(
        struct timespec *ts)
{
    if (ts == NULL)
        return;

    struct timespec current_time;
    timespec_get(&current_time, TIME_UTC);

    struct timespec elapsed_time = {
        .tv_sec = current_time.tv_sec - archi_log_start_timespec.tv_sec,
        .tv_nsec = current_time.tv_nsec - archi_log_start_timespec.tv_nsec,
    };

    if (elapsed_time.tv_nsec < 0)
    {
        elapsed_time.tv_nsec += 1000000000; /// 1.000.000.000 ns == 1 s
        elapsed_time.tv_sec--;
    }

    *ts = elapsed_time;
}

/*****************************************************************************/

static
void
archi_log(
        const char *message_char,
        const char *message_color,
        const char *module,
        const char *format,
        va_list args)
{
    // Get current date/time
    struct timespec ts;
    archi_log_elapsed_time(&ts);

#ifndef __STDC_NO_ATOMICS__
    while (atomic_flag_test_and_set_explicit(&archi_print_spinlock_flag, memory_order_acquire))
        ; // spin until the lock is acquired
#endif

    // Set the color, print date/time and message type character
    fprintf(stderr, ARCHI_COLOR_RESET "\r%s %li:%02li:%02li.%03li,%03li [%s] ",
            message_color,
            (long)ts.tv_sec / 60 / 60,      // hours
            (long)ts.tv_sec / 60 % 60,      // minutes
            (long)ts.tv_sec % 60,           // seconds
            (long)ts.tv_nsec / 1000 / 1000, // milliseconds
            (long)ts.tv_nsec / 1000 % 1000, // microseconds
            message_char);

    // Print module name
    if (module != NULL)
        fprintf(stderr, "%s: ", module);

    // Print the message
    if (format != NULL)
        vfprintf(stderr, format, args);

    // Finally, reset the color
    fprintf(stderr, ARCHI_COLOR_RESET "\n");

#ifndef __STDC_NO_ATOMICS__
    atomic_flag_clear_explicit(&archi_print_spinlock_flag, memory_order_release); // release the lock
#endif
}

/*****************************************************************************/

void
archi_log_error(
        const char *module,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY_ERROR)
        return;

    va_list args;
    va_start(args, format);

    archi_log(ARCHI_LOG_TYPESTR_ERROR, ARCHI_LOG_COLOR_ERROR, module, format, args);

    va_end(args);
}

void
archi_log_warning(
        const char *module,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY_WARNING)
        return;

    va_list args;
    va_start(args, format);

    archi_log(ARCHI_LOG_TYPESTR_WARNING, ARCHI_LOG_COLOR_WARNING, module, format, args);

    va_end(args);
}

void
archi_log_notice(
        const char *module,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY_NOTICE)
        return;

    va_list args;
    va_start(args, format);

    archi_log(ARCHI_LOG_TYPESTR_NOTICE, ARCHI_LOG_COLOR_NOTICE, module, format, args);

    va_end(args);
}

void
archi_log_info(
        const char *module,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY_INFO)
        return;

    va_list args;
    va_start(args, format);

    archi_log(ARCHI_LOG_TYPESTR_INFO, ARCHI_LOG_COLOR_INFO, module, format, args);

    va_end(args);
}

void
archi_log_debug(
        const char *module,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY_DEBUG)
        return;

    va_list args;
    va_start(args, format);

    archi_log(ARCHI_LOG_TYPESTR_DEBUG, ARCHI_LOG_COLOR_DEBUG, module, format, args);

    va_end(args);
}

