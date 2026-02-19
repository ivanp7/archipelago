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
 * @brief Data for operation functions for SDL2 rendering operations.
 */

#pragma once
#ifndef _ARCHI_SDL2_EXE_RENDER_TYP_H_
#define _ARCHI_SDL2_EXE_RENDER_TYP_H_

#include "SDL_render.h"

#include <stdbool.h>


/**
 * @brief Operation function data: parameters for updating texture area.
 *
 * If `entire` is true, field `area` is not used.
 */
typedef struct archi_dexgraph_op_data__sdl2_texture_copy {
    SDL_Texture *texture; ///< Updated texture.

    bool entire; ///< Whether to update entire texture.
    SDL_Rect area; ///< Texture area rectangle.

    const void *pixels; ///< New pixel data.
    int pitch; ///< Pixel buffer pitch (in pixels).
} archi_dexgraph_op_data__sdl2_texture_copy_t;

/**
 * @brief Operation function data: SDL_RenderCopy() parameters.
 *
 * If `entire_target` is true, field `target_area` is not used.
 * If `entire_source` is true, field `source_area` is not used.
 */
typedef struct archi_dexgraph_op_data__sdl2_render_copy {
    SDL_Renderer *renderer; ///< Target renderer.
    SDL_Texture *texture; ///< Copied texture.

    bool entire_target; ///< Whether the entire target is used.
    bool entire_source; ///< Whether the entire source is used.

    SDL_Rect target_area; ///< Target area rectangle.
    SDL_Rect source_area; ///< Source area rectangle.
} archi_dexgraph_op_data__sdl2_render_copy_t;

#endif // _ARCHI_SDL2_EXE_RENDER_TYP_H_

