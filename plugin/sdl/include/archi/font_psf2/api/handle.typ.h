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
 * @brief PSFv2 font handle type.
 */

#pragma once
#ifndef _ARCHI_FONT_PSF2_API_HANDLE_TYP_H_
#define _ARCHI_FONT_PSF2_API_HANDLE_TYP_H_

#include <stdint.h> // for uint32_t


struct archi_font_psf2;

/**
 * @brief Pointer to PC Screen Font version 2.
 */
typedef struct archi_font_psf2 *archi_font_psf2_t;

#endif // _ARCHI_FONT_PSF2_API_HANDLE_TYP_H_

