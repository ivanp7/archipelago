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

#pragma once
#ifndef _ARCHI_SDL2_EXE_RENDER_FUN_H_
#define _ARCHI_SDL2_EXE_RENDER_FUN_H_

#include "archi/exec/api/operation.typ.h"


/**
 * @brief Operation function: update texture area with new pixel data.
 *
 * Function data type: archi_dexgraph_op_data__sdl2_texture_copy_t.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__sdl2_texture_copy);

/**
 * @brief Operation function: call SDL_RenderClear().
 *
 * Function data type: struct SDL_Renderer.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__sdl2_render_clear);

/**
 * @brief Operation function: call SDL_RenderCopy().
 *
 * Function data type: archi_dexgraph_op_data__sdl2_render_copy_t.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__sdl2_render_copy);

/**
 * @brief Operation function: call SDL_RenderPresent().
 *
 * Function data type: struct SDL_Renderer.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__sdl2_render_present);

#endif // _ARCHI_SDL2_EXE_RENDER_FUN_H_

