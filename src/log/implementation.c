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
 * @brief Implementation of the logging module.
 */

#include "archi/log/context.fun.h"
#include "archi/log/print.fun.h"
#include "archi/log/print.def.h"
#include "archi/log/color.def.h"
#include "context.typ.h" // for struct archi_log_context

#include <time.h> // for struct timespec, timespec_get()
#include <stdio.h> // for fprintf(), vfprintf(), stderr
#include <stdarg.h>

static
struct archi_log_context *archi_logger;

void
archi_log_initialize(
        struct archi_log_context *context)
{
    if (archi_logger != NULL)
        return;

    archi_logger = context;
}

struct archi_log_context*
archi_log_get_context(void)
{
    return archi_logger;
}

void
archi_log_elapsed_time(
        struct timespec *ts)
{
    if ((archi_logger == NULL) || (ts == NULL))
        return;

    struct timespec start_time = archi_logger->start_time;

    struct timespec current_time;
    timespec_get(&current_time, TIME_UTC);

    struct timespec elapsed_time = {
        .tv_sec = current_time.tv_sec - start_time.tv_sec,
        .tv_nsec = current_time.tv_nsec - start_time.tv_nsec,
    };

    if (elapsed_time.tv_nsec < 0)
    {
        elapsed_time.tv_nsec += 1000000000; // 1,000,000,000 ns == 1 s
        elapsed_time.tv_sec--;
    }

    *ts = elapsed_time;
}

int
archi_log_verbosity(void)
{
    return (archi_logger != NULL) ? archi_logger->verbosity_level : ARCHI_LOG_VERBOSITY_QUIET;
}

bool
archi_log_colorful_output(void)
{
    return (archi_logger != NULL) ? archi_logger->colorful_output : false;
}

/*****************************************************************************/

void
archi_print(
        const char *format,
        ...)
{
    if ((archi_logger == NULL) || (format == NULL))
        return;

    va_list args;
    va_start(args, format);

#ifndef __STDC_NO_ATOMICS__
    while (atomic_flag_test_and_set_explicit(&archi_logger->spinlock, memory_order_acquire))
        ; // spin until the lock is acquired
#endif

    vfprintf(stderr, format, args);

#ifndef __STDC_NO_ATOMICS__
    atomic_flag_clear_explicit(&archi_logger->spinlock, memory_order_release); // release the lock
#endif

    va_end(args);
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
    while (atomic_flag_test_and_set_explicit(&archi_logger->spinlock, memory_order_acquire))
        ; // spin until the lock is acquired
#endif

    if (archi_logger->colorful_output)
        fprintf(stderr, ARCHI_COLOR_RESET);

    fprintf(stderr, "\r");

    if (archi_logger->colorful_output)
        fprintf(stderr, message_color);

    // Set the color, print date/time and message type character
    fprintf(stderr, " %li:%02li:%02li.%03li,%03li [%s] ",
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
    if (archi_logger->colorful_output)
        fprintf(stderr, ARCHI_COLOR_RESET);

    fprintf(stderr, "\n");

#ifndef __STDC_NO_ATOMICS__
    atomic_flag_clear_explicit(&archi_logger->spinlock, memory_order_release); // release the lock
#endif
}

/*****************************************************************************/

/**
 * @defgroup ArchiLogColorConstants  Message Color Constants
 * @brief ANSI color codes used to colorize log output per level.
 * @{
 */

#define ARCHI_LOG_COLOR_ERROR       ARCHI_COLOR_FG_BRI_RED      ///< Color for error messages.
#define ARCHI_LOG_COLOR_WARNING     ARCHI_COLOR_FG_BRI_YELLOW   ///< Color for warning messages.
#define ARCHI_LOG_COLOR_NOTICE      ARCHI_COLOR_FG_BRI_BLUE     ///< Color for notice messages.
#define ARCHI_LOG_COLOR_INFO        "" /* default color */      ///< Color for info messages.
#define ARCHI_LOG_COLOR_DEBUG       ARCHI_COLOR_FG_BRI_BLACK    ///< Color for debug messages.

/** @} end of ArchiLogColorConstants */

/**
 * @defgroup ArchiLogLevelNameConstants  Message Level Name Constants
 * @brief Short identifiers prepended to each log line to denote severity.
 * @{
 */

#define ARCHI_LOG_LEVEL_NAME_ERROR      "ERR"   ///< Severity level name for error messages.
#define ARCHI_LOG_LEVEL_NAME_WARNING    "WRN"   ///< Severity level name for warning messages.
#define ARCHI_LOG_LEVEL_NAME_NOTICE     "NTC"   ///< Severity level name for notice messages.
#define ARCHI_LOG_LEVEL_NAME_INFO       "INF"   ///< Severity level name for info messages.
#define ARCHI_LOG_LEVEL_NAME_DEBUG      "DBG"   ///< Severity level name for debug messages.

/** @} end of ArchiLogLevelNameConstants */

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

    archi_log(ARCHI_LOG_LEVEL_NAME_ERROR, ARCHI_LOG_COLOR_ERROR, module, format, args);

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

    archi_log(ARCHI_LOG_LEVEL_NAME_WARNING, ARCHI_LOG_COLOR_WARNING, module, format, args);

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

    archi_log(ARCHI_LOG_LEVEL_NAME_NOTICE, ARCHI_LOG_COLOR_NOTICE, module, format, args);

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

    archi_log(ARCHI_LOG_LEVEL_NAME_INFO, ARCHI_LOG_COLOR_INFO, module, format, args);

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

    archi_log(ARCHI_LOG_LEVEL_NAME_DEBUG, ARCHI_LOG_COLOR_DEBUG, module, format, args);

    va_end(args);
}

