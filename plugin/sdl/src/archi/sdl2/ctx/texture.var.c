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
 * @brief Context interface for SDL textures.
 */

#include "archi/sdl2/ctx/texture.var.h"
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


struct archi_context_data__sdl2_texture {
    archi_rcpointer_t texture;

    // References
    archi_rcpointer_t renderer;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__sdl2_texture)
{
    // Parse parameters
    archi_rcpointer_t renderer = {0};
    int width = 0, height = 0;
    const char *pixel_format_name = NULL;
    bool access_static = false, access_streaming = false, access_target = false;
    bool width_set = false, height_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "renderer",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__SDL2_RENDERER)}},
                .assign = {archi_plist_assign__rcpointer, &renderer, sizeof(renderer), NULL}},
            {.name = "width",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &width, sizeof(width), &width_set}},
            {.name = "height",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &height, sizeof(height), &height_set}},
            {.name = "pixel_format",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}},
                .assign = {archi_plist_assign__dptr, &pixel_format_name, sizeof(pixel_format_name), NULL}},
            {.name = "static",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &access_static, sizeof(access_static), NULL}},
            {.name = "streaming",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &access_streaming, sizeof(access_streaming), NULL}},
            {.name = "target",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &access_target, sizeof(access_target), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    if (renderer.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "renderer is not set");
        return NULL;
    }
    else if (!width_set)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture width is not set");
        return NULL;
    }
    else if (width <= 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture width is non-positive");
        return NULL;
    }
    else if (!height_set)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture height is not set");
        return NULL;
    }
    else if (height <= 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture height is non-positive");
        return NULL;
    }
    else if (pixel_format_name == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture pixel format is not set");
        return NULL;
    }
    else if ((int)access_static + (int)access_streaming + (int)access_target == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture access mode is not set");
        return NULL;
    }
    else if ((int)access_static + (int)access_streaming + (int)access_target != 1)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture access modes are exclusive");
        return NULL;
    }

    int access = 0;
    if (access_static)
        access = SDL_TEXTUREACCESS_STATIC;
    else if (access_streaming)
        access = SDL_TEXTUREACCESS_STREAMING;
    else if (access_target)
        access = SDL_TEXTUREACCESS_TARGET;

    uint32_t pixel_format;
    {
#define PIXEL_FORMAT(id)                                            \
        else if (ARCHI_STRING_COMPARE(pixel_format_name, ==, #id))  \
            pixel_format = SDL_PIXELFORMAT_##id;

        if (0);

        // the list was taken from SDL_pixels.h
        PIXEL_FORMAT(INDEX1LSB)
        PIXEL_FORMAT(INDEX1MSB)
        PIXEL_FORMAT(INDEX2LSB)
        PIXEL_FORMAT(INDEX2MSB)
        PIXEL_FORMAT(INDEX4LSB)
        PIXEL_FORMAT(INDEX4MSB)
        PIXEL_FORMAT(INDEX8)
        PIXEL_FORMAT(RGB332)
        PIXEL_FORMAT(XRGB4444)
        PIXEL_FORMAT(RGB444)
        PIXEL_FORMAT(XBGR4444)
        PIXEL_FORMAT(BGR444)
        PIXEL_FORMAT(RGB555)
        PIXEL_FORMAT(XBGR1555)
        PIXEL_FORMAT(BGR555)
        PIXEL_FORMAT(ARGB4444)
        PIXEL_FORMAT(RGBA4444)
        PIXEL_FORMAT(ABGR4444)
        PIXEL_FORMAT(BGRA4444)
        PIXEL_FORMAT(ARGB1555)
        PIXEL_FORMAT(RGBA5551)
        PIXEL_FORMAT(ABGR1555)
        PIXEL_FORMAT(BGRA5551)
        PIXEL_FORMAT(RGB565)
        PIXEL_FORMAT(BGR565)
        PIXEL_FORMAT(RGB24)
        PIXEL_FORMAT(BGR24)
        PIXEL_FORMAT(XRGB8888)
        PIXEL_FORMAT(RGB888)
        PIXEL_FORMAT(RGBX8888)
        PIXEL_FORMAT(XBGR8888)
        PIXEL_FORMAT(BGR888)
        PIXEL_FORMAT(BGRX8888)
        PIXEL_FORMAT(ARGB8888)
        PIXEL_FORMAT(RGBA8888)
        PIXEL_FORMAT(ABGR8888)
        PIXEL_FORMAT(BGRA8888)
        PIXEL_FORMAT(ARGB2101010)

        PIXEL_FORMAT(RGBA32)
        PIXEL_FORMAT(ARGB32)
        PIXEL_FORMAT(BGRA32)
        PIXEL_FORMAT(ABGR32)
        PIXEL_FORMAT(RGBX32)
        PIXEL_FORMAT(XRGB32)
        PIXEL_FORMAT(BGRX32)
        PIXEL_FORMAT(XBGR32)

        PIXEL_FORMAT(YV12)
        PIXEL_FORMAT(IYUV)
        PIXEL_FORMAT(YUY2)
        PIXEL_FORMAT(UYVY)
        PIXEL_FORMAT(YVYU)
        PIXEL_FORMAT(NV12)
        PIXEL_FORMAT(NV21)
        PIXEL_FORMAT(EXTERNAL_OES)

        else
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unrecognized pixel format '%s'", pixel_format_name);
            return NULL;
        }

#undef PIXEL_FORMAT
    }

    // Construct the context
    struct archi_context_data__sdl2_texture *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__sdl2_texture){
        .texture = {
            .ptr = SDL_CreateTexture(renderer.ptr, pixel_format, access, width, height),
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__SDL2_TEXTURE),
        },
    };

    if (context_data->texture.ptr == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create an SDL texture: %s", SDL_GetError());
        return NULL;
    }

    // Initialize references
    context_data->renderer = archi_rcpointer_own(renderer, ARCHI_ERROR_PARAM);
    if (!context_data->renderer.attr)
        goto failure;

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    SDL_DestroyTexture(context_data->texture.ptr);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__sdl2_texture)
{
    struct archi_context_data__sdl2_texture *context_data =
        (struct archi_context_data__sdl2_texture*)context;

    SDL_DestroyTexture(context_data->texture.ptr);
    archi_rcpointer_disown(context_data->renderer);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__sdl2_texture)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__sdl2_texture *context_data =
        (struct archi_context_data__sdl2_texture*)context;

    if (ARCHI_STRING_COMPARE("renderer", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->renderer);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__sdl2_texture = {
    .init_fn = archi_context_init__sdl2_texture,
    .final_fn = archi_context_final__sdl2_texture,
    .eval_fn = archi_context_eval__sdl2_texture,
};

