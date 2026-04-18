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
 * @brief Context interface for SDL renderers.
 */

#include "archi/sdl2/ctx/renderer.var.h"
#include "archi/sdl2/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include "SDL_render.h"

#include <stdint.h> // for uint32_t
#include <stdlib.h> // for malloc(), free()


struct archi_context_data__sdl2_renderer {
    archi_rcpointer_t renderer;

    // References
    archi_rcpointer_t window;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__sdl2_renderer)
{
    // Parse parameters
    archi_rcpointer_t window = {0};
    int driver_index = -1;
    bool software = false, accelerated = false, present_vsync = false, target_texture = false;
    uint32_t flags = 0;
    bool driver_index_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "window",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__SDL2_WINDOW)}},
                .assign = {archi_plist_assign__rcpointer, &window, sizeof(window), NULL}},
            {.name = "driver_index",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &driver_index, sizeof(driver_index), &driver_index_set}},
            {.name = "software",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &software, sizeof(software), NULL}},
            {.name = "accelerated",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &accelerated, sizeof(accelerated), NULL}},
            {.name = "present_vsync",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &present_vsync, sizeof(present_vsync), NULL}},
            {.name = "target_texture",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &target_texture, sizeof(target_texture), NULL}},
            {.name = "flags",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, uint32_t)}},
                .assign = {archi_plist_assign__value, &flags, sizeof(flags), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    if (window.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "window is not set");
        return NULL;
    }
    else if (driver_index_set && (driver_index < 0))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "rendering driver index is negative");
        return NULL;
    }

    if (software)
        flags |= SDL_RENDERER_SOFTWARE;
    if (accelerated)
        flags |= SDL_RENDERER_ACCELERATED;
    if (present_vsync)
        flags |= SDL_RENDERER_PRESENTVSYNC;
    if (target_texture)
        flags |= SDL_RENDERER_TARGETTEXTURE;

    // Construct the context
    struct archi_context_data__sdl2_renderer *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__sdl2_renderer){
        .renderer = {
            .ptr = SDL_CreateRenderer(window.ptr, driver_index, flags),
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__SDL2_RENDERER),
        },
    };

    if (context_data->renderer.ptr == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create an SDL renderer: %s", SDL_GetError());
        return NULL;
    }

    // Initialize references
    context_data->window = archi_rcpointer_own(window, ARCHI_ERROR_PARAM);
    if (!context_data->window.attr)
        goto failure;

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    SDL_DestroyRenderer(context_data->renderer.ptr);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__sdl2_renderer)
{
    struct archi_context_data__sdl2_renderer *context_data =
        (struct archi_context_data__sdl2_renderer*)context;

    SDL_DestroyRenderer(context_data->renderer.ptr);
    archi_rcpointer_disown(context_data->window);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__sdl2_renderer)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__sdl2_renderer *context_data =
        (struct archi_context_data__sdl2_renderer*)context;

    if (ARCHI_STRING_COMPARE("window", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->window);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__sdl2_renderer = {
    .init_fn = archi_context_init__sdl2_renderer,
    .final_fn = archi_context_final__sdl2_renderer,
    .eval_fn = archi_context_eval__sdl2_renderer,
};

