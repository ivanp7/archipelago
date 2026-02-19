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

#pragma once
#ifndef _ARCHI_SDL2_CTX_LIBRARY_VAR_H_
#define _ARCHI_SDL2_CTX_LIBRARY_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: SDL library initialization.
 *
 * @warning In an application using SDL2, there should be only one context of this type.
 *
 * Initialization parameters:
 * - "everything"       : (char) initialize all subsystems
 * - "timer"            : (char) initialize timer subsystem
 * - "audio"            : (char) initialize audio subsystem
 * - "video"            : (char) initialize video subsystem
 * - "joystick"         : (char) initialize joystick subsystem
 * - "haptic"           : (char) initialize haptic subsystem
 * - "gamecontroller"   : (char) initialize gamecontroller subsystem
 * - "events"           : (char) initialize events subsystem
 * - "sensor"           : (char) initialize sensor subsystem
 */
extern
const archi_context_interface_t
archi_context_interface__sdl2_library;

#endif // _ARCHI_SDL2_CTX_LIBRARY_VAR_H_

