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
 * @brief Context registry instructions.
 */

#pragma once
#ifndef _ARCHI_EXE_INSTRUCTION_FUN_H_
#define _ARCHI_EXE_INSTRUCTION_FUN_H_

#include "archipelago/context/interface.fun.h"

#include <stddef.h> // for size_t

struct archi_exe_registry_instr_base;

/**
 * @brief Get instruction size in bytes.
 *
 * @return Instruction size in bytes.
 */
size_t
archi_exe_registry_instr_sizeof(
        const struct archi_exe_registry_instr_base *instruction ///< [in] Instruction.
);

/**
 * @brief Execute an application context registry instruction.
 *
 * @return Status code:
 * <0 - error;
 *  0 - success;
 *  1 - failure: the key does not exist in the registry;
 *  2 - failure: the key already exists in the registry.
 */
archi_status_t
archi_exe_registry_instr_execute(
        archi_context_t registry, ///< [in,out] Application registry.
        const struct archi_exe_registry_instr_base *instruction, ///< [in] Instruction to execute.
        archi_reference_count_t ref_count ///< [in] Reference count of memory where static objects reside.
);

/*****************************************************************************/

/**
 * @brief Print details of an application context registry instruction.
 *
 * Instruction fields are printed using archi_print().
 *
 * If instruction number is 0, the number is not printed.
 */
void
archi_exe_registry_instr_print(
        const struct archi_exe_registry_instr_base *instruction, ///< [in] Instruction to execute.
        size_t instruction_number ///< [in] Number of the instruction.
);

#endif // _ARCHI_EXE_INSTRUCTION_FUN_H_

