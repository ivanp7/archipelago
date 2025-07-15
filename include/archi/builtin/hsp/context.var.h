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
 * @brief Application context interface for hierarchical state processor entities.
 */

#pragma once
#ifndef _ARCHI_BUILTIN_HSP_CONTEXT_VAR_H_
#define _ARCHI_BUILTIN_HSP_CONTEXT_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_frame_init);   ///< HSP frame initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_frame_final); ///< HSP frame finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_frame_get);     ///< HSP frame field getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_frame_set);     ///< HSP frame field setter function.
ARCHI_CONTEXT_ACT_FUNC(archi_context_hsp_frame_act);     ///< HSP frame action function.

extern
const archi_context_interface_t archi_context_hsp_frame_interface; ///< HSP frame interface.

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_branch_state_data_init);   ///< HSP branch state data initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_branch_state_data_final); ///< HSP branch state data finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_branch_state_data_get);     ///< HSP branch state data field getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_branch_state_data_set);     ///< HSP branch state data field setter function.

extern
const archi_context_interface_t archi_context_hsp_branch_state_data_interface; ///< HSP branch state data interface.

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_transition_init);   ///< HSP transition initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_transition_final); ///< HSP transition finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_transition_get);     ///< HSP transition field getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_transition_set);     ///< HSP transition field setter function.

extern
const archi_context_interface_t archi_context_hsp_transition_interface; ///< HSP transition interface.

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_transition_attachment_init);   ///< HSP transition attachment initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_transition_attachment_final); ///< HSP transition attachment finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_transition_attachment_get);     ///< HSP transition attachment field getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_transition_attachment_set);     ///< HSP transition attachment field setter function.

extern
const archi_context_interface_t archi_context_hsp_transition_attachment_interface; ///< HSP transition attachment interface.

#endif // _ARCHI_BUILTIN_HSP_CONTEXT_VAR_H_

