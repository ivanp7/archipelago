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
 * @brief Data and function type tags.
 */

#pragma once
#ifndef _ARCHI_SIGNAL_API_TAG_DEF_H_
#define _ARCHI_SIGNAL_API_TAG_DEF_H_

#define ARCHI_POINTER_DATA_TAG__SIGNAL_SET          0x50 ///< Data type tag for archi_signal_set_t.
#define ARCHI_POINTER_DATA_TAG__SIGNAL_FLAGS        0x51 ///< Data type tag for archi_signal_flags_t.

#define ARCHI_POINTER_FUNC_TAG__SIGNAL_HANDLER      0x50 ///< Function type tag for archi_signal_handler_func_t.

#endif // _ARCHI_SIGNAL_API_TAG_DEF_H_

