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

#pragma once
#ifndef _ARCHI_SDL2_CTX_RENDERER_VAR_H_
#define _ARCHI_SDL2_CTX_RENDERER_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: SDL renderer.
 *
 * Initialization parameters:
 * - "window"           : (SDL_Window) owner window
 * - "driver_index"     : (int) index of the rendering driver to initialize
 * - "software"         : (char) renderer is a software fallback
 * - "accelerated"      : (char) renderer uses hardware acceleration
 * - "present_vsync"    : (char) present is synchronized with the refresh rate
 * - "target_texture"   : (char) renderer supports rendering to texture
 * - "flags"            : (uint32_t) renderer creation flags
 *
 * Getter slots:
 * - "window"   : (SDL_Window) owner window
 */
extern
const archi_context_interface_t
archi_context_interface__sdl2_renderer;

#endif // _ARCHI_SDL2_CTX_RENDERER_VAR_H_

