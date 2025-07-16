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
 * @brief Operations on the global log context.
 */

#include "archi/exe/logging.fun.h"
#include "archi/log/verbosity.def.h"
#include "../log/context.typ.h" // for struct archi_log_context

static
struct archi_log_context archi_logger = {
#ifndef __STDC_NO_ATOMICS__
    .spinlock = ATOMIC_FLAG_INIT,
#else
    0
#endif
};

archi_log_context_t
archi_exe_log_context(void)
{
    return &archi_logger;
}

void
archi_exe_log_init_stream(
        FILE *stream)
{
    static bool stream_set = false;
    if (stream_set)
        return;
    stream_set = true;

    if (stream == NULL)
        stream = stderr;

    archi_logger.stream = stream;
}

void
archi_exe_log_init_start_time(void)
{
    static bool start_time_set = false;
    if (start_time_set)
        return;
    start_time_set = true;

    timespec_get(&archi_logger.start_time, TIME_UTC);
}

void
archi_exe_log_init_verbosity(
        int level)
{
    static bool verbosity_set = false;
    if (verbosity_set)
        return;
    verbosity_set = true;

    if (level < 0)
        level = 0;
    else if (level > ARCHI_LOG_VERBOSITY_MAX)
        level = ARCHI_LOG_VERBOSITY_MAX;

    archi_logger.verbosity_level = level;
}

void
archi_exe_log_init_color(
        bool colorful)
{
    static bool colorful_set = false;
    if (colorful_set)
        return;
    colorful_set = true;

    archi_logger.colorful = colorful;
}

