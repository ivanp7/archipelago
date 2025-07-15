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
 * @brief Attached hierarchical state processor transitions.
 */

#include "archi/hsp/transition/attachment.fun.h"
#include "archi/hsp/transition/attachment.typ.h"

ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_attachments_handler)
{
    (void) trans_state;

    const archi_hsp_transition_attachment_t *global_attachment = data;

    if (prev_state.function != NULL)
    {
        const archi_hsp_transition_attachment_t *attachment = prev_state.metadata;
        if ((attachment != NULL) && (attachment->post.function != NULL))
            attachment->post.function(prev_state, next_state, NULL, attachment->post.data);
    }
    else if ((global_attachment != NULL) && (global_attachment->pre.function != NULL))
        global_attachment->pre.function(prev_state, next_state, NULL, global_attachment->pre.data);

    if (next_state.function != NULL)
    {
        const archi_hsp_transition_attachment_t *attachment = next_state.metadata;
        if ((attachment != NULL) && (attachment->pre.function != NULL))
            attachment->pre.function(prev_state, next_state, NULL, attachment->pre.data);
    }
    else if ((global_attachment != NULL) && (global_attachment->post.function != NULL))
        global_attachment->post.function(prev_state, next_state, NULL, global_attachment->post.data);
}

