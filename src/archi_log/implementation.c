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
 * @brief Implementation of the logging module.
 */

#include "archi_log/context.fun.h"
#include "archi_log/print.fun.h"
#include "archi_log/verbosity.def.h"
#include "archi_log/color.def.h"
#include "context.typ.h" // for struct archi_log_context

#include <time.h> // for struct timespec, timespec_get()
#include <stdio.h> // for fprintf(), vfprintf(), stderr
#include <stdarg.h>
#include <stdint.h> // for uintptr_t


static
struct archi_log_context *archi_logger;

ARCHI_GLOBAL_SET_FUNC(archi_global_context_set__log)
{
    if (archi_logger != NULL)
        return;

    archi_logger = context;
}

ARCHI_GLOBAL_GET_FUNC(archi_global_context__log)
{
    return archi_logger;
}

/*****************************************************************************/

void
archi_log_elapsed_time(
        struct timespec *ts)
{
    if (archi_logger == NULL)
        return;
    else if (archi_logger->stream == NULL)
        return;
    else if (ts == NULL)
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
    if (archi_logger == NULL)
        return ARCHI_LOG_VERBOSITY__QUIET;

    return archi_logger->verbosity_level;
}

bool
archi_log_colorful(void)
{
    if (archi_logger == NULL)
        return false;

    return archi_logger->colorful;
}

/*****************************************************************************/

bool
archi_print_lock(
        int verbosity)
{
    if (archi_logger == NULL)
        return false;
    else if (archi_logger->stream == NULL)
        return false;

    if (archi_log_verbosity() < verbosity)
        return false;

#ifndef __STDC_NO_THREADS__
    if (archi_logger->lock != NULL)
    {
        int ret = mtx_lock(archi_logger->lock);
        if (ret != thrd_success)
            return false;
    }
#endif

    return true;
}

void
archi_print_unlock(void)
{
    if (archi_logger == NULL)
        return;
    else if (archi_logger->stream == NULL)
        return;

#ifndef __STDC_NO_THREADS__
    if (archi_logger->lock != NULL)
        mtx_unlock(archi_logger->lock);
#endif
}

/*****************************************************************************/

int
archi_print(
        const char *format,
        ...)
{
    if (archi_logger == NULL)
        return 0;
    else if (archi_logger->stream == NULL)
        return 0;
    else if (format == NULL)
        return 0;

    int printed;
    {
        va_list args;
        va_start(args, format);

        printed = vfprintf(archi_logger->stream, format, args);

        va_end(args);
    }

    return printed;
}

int
archi_print_string(
        const char *string)
{
    if (string == NULL)
        return 0;

    return archi_print("%s", string);
}

int
archi_print_color(
        const char *string)
{
    if (!archi_log_colorful())
        return 0;

    return archi_print_string(string);
}

void
archi_print_hex_dump(
        const void *memory,
        size_t length,
        bool absolute,
        unsigned indentation)
{
    if (archi_logger == NULL)
        return;
    else if (archi_logger->stream == NULL)
        return;
    else if ((memory == NULL) || (length == 0))
        return;

#define BYTES_PER_LINE  (1 << 4) // 16

    uintptr_t start_addr = (uintptr_t)memory; // address of the first byte
    uintptr_t end_addr = start_addr + length; // address of the byte after the last

    uintptr_t addr; // current line address/offset
    int width; // field width for line address/offset

    if (absolute)
    {
        addr = start_addr & ~((uintptr_t)BYTES_PER_LINE - 1); // line address must be aligned to BYTES_PER_LINE
        width = sizeof(addr) * 2; // two hex characters per byte
    }
    else
    {
        addr = start_addr;
        width = 2;
        while (((size_t)1 << (width*4)) < length) // 4 bits per hex character
            width += 2; // two hex characters per byte
    }

    for (; addr < end_addr; addr += BYTES_PER_LINE)
    {
        // Print left padding with spaces
        fprintf(archi_logger->stream, "%*s", (int)indentation, "");

        if (absolute) // Print address
            fprintf(archi_logger->stream, "%0*zx:", width, (size_t)addr);
        else // Print offset
            fprintf(archi_logger->stream, "%0*tx:", width, (ptrdiff_t)(addr - start_addr));

        // Print line bytes in hexadecimal format
        for (uintptr_t i = 0; i < BYTES_PER_LINE; i++)
        {
            if (i % 2 == 0)
                fprintf(archi_logger->stream, " "); // separator between pairs of bytes

            if ((addr + i >= start_addr) && (addr + i < end_addr))
                fprintf(archi_logger->stream, "%02hhx", *(unsigned char*)(addr + i));
            else
                fprintf(archi_logger->stream, "%2s", "");
        }

        // Print separator whitespace
        fprintf(archi_logger->stream, "  ");

        // Print line bytes as characters
        for (uintptr_t i = 0; i < BYTES_PER_LINE; i++)
        {
            if ((addr + i >= start_addr) && (addr + i < end_addr))
            {
                int c = *(unsigned char*)(addr + i);

                if ((c >= 0x20) && (c < 0x7F)) // printable ASCII character
                    fprintf(archi_logger->stream, "%c", c);
                else
                    fprintf(archi_logger->stream, ".");
            }
            else
                fprintf(archi_logger->stream, " ");
        }

        // Print the newline
        fprintf(archi_logger->stream, "\n");
    }

#undef BYTES_PER_LINE
}

/*****************************************************************************/

static
void
archi_log(
        const char *message_char,
        const char *message_color,
        const char *origin,
        const char *format,
        va_list args)
{
    // Get current date/time
    struct timespec ts;
    archi_log_elapsed_time(&ts);

#ifndef __STDC_NO_THREADS__
    if (archi_logger->lock != NULL)
    {
        int ret = mtx_lock(archi_logger->lock);
        if (ret != thrd_success)
            return;
    }
#endif

    if (archi_logger->colorful)
        fprintf(archi_logger->stream, "%s", ARCHI_COLOR_RESET);

    fprintf(archi_logger->stream, "\r");

    if (archi_logger->colorful)
        fprintf(archi_logger->stream, "%s", message_color);

    // Set the color, print date/time and message type character
    fprintf(archi_logger->stream, " %li:%02li:%02li.%03li,%03li [%s] ",
            (long)ts.tv_sec / 60 / 60,      // hours
            (long)ts.tv_sec / 60 % 60,      // minutes
            (long)ts.tv_sec % 60,           // seconds
            (long)ts.tv_nsec / 1000 / 1000, // milliseconds
            (long)ts.tv_nsec / 1000 % 1000, // microseconds
            message_char);

    // Print message origin name
    if (origin != NULL)
        fprintf(archi_logger->stream, "(%s) ", origin);

    // Print the message
    if (format != NULL)
        vfprintf(archi_logger->stream, format, args);

    // Finally, reset the color
    if (archi_logger->colorful)
        fprintf(archi_logger->stream, "%s", ARCHI_COLOR_RESET);

    fprintf(archi_logger->stream, "\n");

#ifndef __STDC_NO_THREADS__
    if (archi_logger->lock != NULL)
        mtx_unlock(archi_logger->lock);
#endif
}

/*****************************************************************************/

void
archi_log_error(
        const char *origin,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY__ERROR)
        return;

    va_list args;
    va_start(args, format);

    archi_log("ERR", ARCHI_LOG_COLOR_ERROR, origin, format, args);

    va_end(args);
}

void
archi_log_warning(
        const char *origin,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY__WARNING)
        return;

    va_list args;
    va_start(args, format);

    archi_log("WRN", ARCHI_LOG_COLOR_WARNING, origin, format, args);

    va_end(args);
}

void
archi_log_notice(
        const char *origin,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY__NOTICE)
        return;

    va_list args;
    va_start(args, format);

    archi_log("NTC", ARCHI_LOG_COLOR_NOTICE, origin, format, args);

    va_end(args);
}

void
archi_log_info(
        const char *origin,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY__INFO)
        return;

    va_list args;
    va_start(args, format);

    archi_log("INF", ARCHI_LOG_COLOR_INFO, origin, format, args);

    va_end(args);
}

void
archi_log_debug(
        const char *origin,
        const char *format,
        ...)
{
    if (archi_log_verbosity() < ARCHI_LOG_VERBOSITY__DEBUG)
        return;

    va_list args;
    va_start(args, format);

    archi_log("DBG", ARCHI_LOG_COLOR_DEBUG, origin, format, args);

    va_end(args);
}

