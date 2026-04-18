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
 * @brief Context interface for SDL library initialization.
 */

#include "archi/sdl2/ctx/library.var.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"

#include "SDL.h"

#include <stdbool.h>


static
bool
archi_sdl2_initialized;

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__sdl2_library)
{
    if (archi_sdl2_initialized)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "SDL2 library is initialized already");
        return NULL;
    }

    // Parse parameters
    bool everything = false;
    bool timer, audio, video, joystick, haptic, gamecontroller, events, sensor;
    {
        archi_plist_param_t parsed[] = {
            {.name = "everything",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &everything, sizeof(everything), NULL}},
            // subsystems
            {.name = "timer", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}}},
            {.name = "audio", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}}},
            {.name = "video", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}}},
            {.name = "joystick", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}}},
            {.name = "haptic", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}}},
            {.name = "gamecontroller", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}}},
            {.name = "events", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}}},
            {.name = "sensor", .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;

        size_t index = 0;

#define SUBSYSTEM(var)  do {                            \
            index++;                                    \
            if (parsed[index].value_set)                \
                var = *(char*)parsed[index].value.ptr;  \
            else                                        \
                var = everything;                       \
        } while (0)

        SUBSYSTEM(timer);
        SUBSYSTEM(audio);
        SUBSYSTEM(video);
        SUBSYSTEM(joystick);
        SUBSYSTEM(haptic);
        SUBSYSTEM(gamecontroller);
        SUBSYSTEM(events);
        SUBSYSTEM(sensor);

#undef SUBSYSTEM
    }

    Uint32 flags = everything ? SDL_INIT_EVERYTHING : 0;
    {
#define SUBSYSTEM(var, flag)    \
        if (var) flags |= flag; else flags &= ~flag

        SUBSYSTEM(timer, SDL_INIT_TIMER);
        SUBSYSTEM(audio, SDL_INIT_AUDIO);
        SUBSYSTEM(video, SDL_INIT_VIDEO);
        SUBSYSTEM(joystick, SDL_INIT_JOYSTICK);
        SUBSYSTEM(haptic, SDL_INIT_HAPTIC);
        SUBSYSTEM(gamecontroller, SDL_INIT_GAMECONTROLLER);
        SUBSYSTEM(events, SDL_INIT_EVENTS);
        SUBSYSTEM(sensor, SDL_INIT_SENSOR);

#undef SUBSYSTEM
    }

    // Construct the context
    int ret = SDL_Init(flags);
    if (ret < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't initialize SDL2 library (error code %i)", ret);
        return NULL;
    }

    archi_sdl2_initialized = true;

    static archi_rcpointer_t context_data; // dummy
    context_data = (archi_rcpointer_t){0};

    ARCHI_ERROR_RESET();
    return &context_data; // must return something non-NULL
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__sdl2_library)
{
    (void) context;

    SDL_Quit();
    archi_sdl2_initialized = false;
}

const archi_context_interface_t
archi_context_interface__sdl2_library = {
    .init_fn = archi_context_init__sdl2_library,
    .final_fn = archi_context_final__sdl2_library,
};

