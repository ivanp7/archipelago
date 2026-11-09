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
 * @brief Functions of directed execution graph operations.
 */

#pragma once
#ifndef _ARCHI_EXEC_I_OPERATION_FUN_H_
#define _ARCHI_EXEC_I_OPERATION_FUN_H_

#include "archi/exec/i/operation.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Call an operation.
 *
 * If the operation function is NULL, it is a no-op.
 *
 * @return True on success, false on failure.
 */
void
archi_dexgraph_operation_call(
        archi_dexgraph_operation_t operation, ///< [in] Operation.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/*****************************************************************************/

/**
 * @brief Allocate an operation sequence.
 *
 * @return Newly allocated operation sequence, or NULL in case of failure.
 */
archi_dexgraph_operation_seq_t*
archi_dexgraph_operation_seq_alloc(
        size_t length ///< [in] Length of the operation sequence.
);

/**
 * @brief Deallocate a node.
 *
 * @note Node name is freed.
 */
void
archi_dexgraph_operation_seq_free(
        archi_dexgraph_operation_seq_t *sequence ///< [in] Operation sequence.
);

/**
 * @brief Get sequence operation with the specified index.
 *
 * The function checks for the index range,
 * the empty operation is returned for indices out of range.
 *
 * @return An operation with the specified index.
 */
archi_dexgraph_operation_t
archi_dexgraph_operation_seq_elt(
        const archi_dexgraph_operation_seq_t *sequence, ///< [in] Operation sequence.
        size_t op_index ///< [in] Index of an operation.
);

/**
 * @brief Execute an operation sequence, starting at the specified index.
 *
 * If the operation function is NULL, it is a no-op.
 * Null sequences are treated like empty sequences.
 *
 * @return Index of the failed operation, or ARCHI_DEXGRAPH_OPERATION_SEQ_END
 * if all operations were executed successfully.
 */
size_t
archi_dexgraph_operation_seq_exec(
        const archi_dexgraph_operation_seq_t *sequence, ///< [in] Operation sequence.
        size_t op_index, ///< [in] Starting index.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_EXEC_I_OPERATION_FUN_H_

