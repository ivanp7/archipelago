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
 * @brief Operation functions for SDL2 rendering operations.
 */

#include "archi/sdl2/exe/render.fun.h"
#include "archi/sdl2/exe/render.typ.h"

#include "SDL_render.h"

#include <string.h> // for memcpy()


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__sdl2_texture_copy)
{
    const archi_dexgraph_op_data__sdl2_texture_copy_t *copy_params = data;

    if (copy_params == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "renderer copy parameters is NULL");
        return;
    }
    else if (copy_params->texture == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture handle is NULL");
        return;
    }
    else if (copy_params->pixels == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to pixel buffer is NULL");
        return;
    }

    // Query texture properties
    int texture_width, texture_height;

    Uint32 pixel_format;
    int texture_access;

    if (SDL_QueryTexture(copy_params->texture,
                &pixel_format, &texture_access, &texture_width, &texture_height) != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't query SDL texture properties: %s", SDL_GetError());
        return;
    }

    int area_width = copy_params->entire ? texture_width : copy_params->area.w;
    int area_height = copy_params->entire ? texture_height : copy_params->area.h;

    if (copy_params->pitch < area_width)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pixel buffer pitch is less than texture area width");
        return;
    }

    int buffer_pitch = copy_params->pitch * SDL_BYTESPERPIXEL(pixel_format); // pixels to bytes

    // Update texture area
    if (texture_access == SDL_TEXTUREACCESS_STREAMING)
    {
        // Lock the area
        void *texture_pixels;
        int texture_pitch; // in bytes

        if (SDL_LockTexture(copy_params->texture, copy_params->entire ? NULL : &copy_params->area,
                    &texture_pixels, &texture_pitch) != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't lock SDL texture area: %s", SDL_GetError());
            return;
        }

        int row_size = texture_width * SDL_BYTESPERPIXEL(pixel_format); // texture row size in bytes

        if ((texture_pitch == row_size) && (buffer_pitch == row_size))
        {
            // Copy the whole area
            int area_size = area_width * area_height * SDL_BYTESPERPIXEL(pixel_format);

            memcpy(texture_pixels, copy_params->pixels, area_size);
        }
        else
        {
            // Copy area row by row
            row_size = area_width * SDL_BYTESPERPIXEL(pixel_format); // area row size in bytes

            for (int row_index = 0; row_index < area_height; row_index++)
                memcpy((char*)texture_pixels + texture_pitch * row_index,
                        (const char*)copy_params->pixels + buffer_pitch * row_index,
                        row_size);
        }

        // Unlock the area
        SDL_UnlockTexture(copy_params->texture);
    }
    else // slow, but generally applicable method
    {
        if (SDL_UpdateTexture(copy_params->texture, copy_params->entire ? NULL : &copy_params->area,
                    copy_params->pixels, buffer_pitch) != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't update SDL texture area: %s", SDL_GetError());
            return;
        }
    }

    ARCHI_ERROR_RESET();
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__sdl2_render_clear)
{
    if (data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "renderer handle is NULL");
        return;
    }

    SDL_RenderClear(data);

    ARCHI_ERROR_RESET();
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__sdl2_render_copy)
{
    archi_dexgraph_op_data__sdl2_render_copy_t *copy_params = data;

    if (copy_params == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "renderer copy parameters is NULL");
        return;
    }
    else if (copy_params->renderer == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "renderer handle is NULL");
        return;
    }
    else if (copy_params->texture == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "texture handle is NULL");
        return;
    }

    if (SDL_RenderCopy(copy_params->renderer, copy_params->texture,
                copy_params->entire_source ? NULL : &copy_params->source_area,
                copy_params->entire_target ? NULL : &copy_params->target_area) != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't copy an SDL texture: %s", SDL_GetError());
        return;
    }

    ARCHI_ERROR_RESET();
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__sdl2_render_present)
{
    if (data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "renderer handle is NULL");
        return;
    }

    SDL_RenderPresent(data);

    ARCHI_ERROR_RESET();
}

