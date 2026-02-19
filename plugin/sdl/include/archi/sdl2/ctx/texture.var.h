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

#pragma once
#ifndef _ARCHI_SDL2_CTX_TEXTURE_VAR_H_
#define _ARCHI_SDL2_CTX_TEXTURE_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: SDL texture.
 *
 * Initialization parameters:
 * - "renderer"     : (SDL_Renderer) owner renderer
 * - "width"        : (int) texture width in pixels
 * - "height"       : (int) texture height in pixels
 * - "pixel_format" : (char[]) pixel format name
 * - "static"       : (char) texture changes rarely, not lockable
 * - "streaming"    : (char) texture changes frequently, lockable
 * - "target"       : (char) texture can be used as a render target
 *
 * Getter slots:
 * - "renderer"     : (SDL_Renderer) owner renderer
 */
extern
const archi_context_interface_t
archi_context_interface__sdl2_texture;

#endif // _ARCHI_SDL2_CTX_TEXTURE_VAR_H_

