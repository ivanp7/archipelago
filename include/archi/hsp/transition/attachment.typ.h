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
 * @brief Types for attached hierarchical state processor transitions.
 */

#pragma once
#ifndef _ARCHI_HSP_TRANSITION_ATTACHMENT_TYP_H_
#define _ARCHI_HSP_TRANSITION_ATTACHMENT_TYP_H_

#include "archi/hsp/transition.typ.h"

/**
 * @brief Transition attached to a state of a hierarchical state processor.
 */
typedef struct archi_hsp_transition_attachment {
    archi_hsp_transition_t pre;  ///< Transition used before the state execution.
    archi_hsp_transition_t post; ///< Transition used after the state execution.
} archi_hsp_transition_attachment_t;

#endif // _ARCHI_HSP_TRANSITION_ATTACHMENT_TYP_H_

