/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Application plugin API version and magic number.
 */

#pragma once
#ifndef _ARCHI_APP_VERSION_DEF_H_
#define _ARCHI_APP_VERSION_DEF_H_

/**
 * @brief API magic number - confirms the format.
 */
#define ARCHI_API_MAGIC 0x67756C50 // "Plug" in little endian
/**
 * @brief API version - specifies application-plugin interface compatibility.
 */
#define ARCHI_API_VERSION 241202 // 2024-Dec-02

#endif // _ARCHI_APP_VERSION_DEF_H_

