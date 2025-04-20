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
 * @brief Application configuration instructions.
 */

#pragma once
#ifndef _ARCHI_APP_INSTRUCTION_FUN_H_
#define _ARCHI_APP_INSTRUCTION_FUN_H_

#include "archi/util/status.typ.h"

#include <stddef.h> // for size_t

struct archi_app_instruction_base;
struct archi_context;

/**
 * @brief Get instruction size in bytes.
 *
 * @return Instruction size in bytes.
 */
size_t
archi_app_instruction_sizeof(
        const struct archi_app_instruction_base *instruction ///< [in] Instruction.
);

/**
 * @brief Execute an application initialization instruction for the specified registry.
 *
 * @return Status code:
 * <0 - error;
 *  0 - success;
 *  1 - failure: the key does not exist in the registry;
 *  2 - failure: the key already exists in the registry.
 */
archi_status_t
archi_app_instruction_execute(
        struct archi_context *registry, ///< [in,out] Application registry.
        const struct archi_app_instruction_base *instruction ///< [in] Instruction to execute.
);

#endif // _ARCHI_APP_INSTRUCTION_FUN_H_

