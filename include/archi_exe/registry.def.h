/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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
 * @brief Keys of the built-in contexts in the context registry.
 */

#pragma once
#ifndef _ARCHI_EXE_REGISTRY_DEF_H_
#define _ARCHI_EXE_REGISTRY_DEF_H_

// After modifying this list, don't forget to update the help message in `exe/args.c`

#define ARCHI_EXE_REGISTRY_KEY_REGISTRY         "archi.registry"    ///< Registry key: the context registry itself.
#define ARCHI_EXE_REGISTRY_KEY_EXECUTABLE       "archi.executable"  ///< Registry key: the library handle of the executable itself.
#define ARCHI_EXE_REGISTRY_KEY_INPUT_FILE       "archi.input_file"  ///< Registry key: the current input file context.

#define ARCHI_EXE_REGISTRY_KEY_SIGNAL           "archi.signal"      ///< Registry key: the signal management context.

#endif // _ARCHI_EXE_REGISTRY_DEF_H_

