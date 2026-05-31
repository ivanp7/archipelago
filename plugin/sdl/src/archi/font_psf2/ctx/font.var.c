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
 * @brief Context interface for PSFv2 font.
 */

#include "archi/font_psf2/ctx/font.var.h"
#include "archi/font_psf2/api/font.fun.h"
#include "archi/font_psf2/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__font_psf2)
{
    // Parse parameters
    archi_rcpointer_t data = {0};
    {
        archi_plist_param_t parsed[] = {
            {.name = "data",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(sizeof(archi_font_psf2_header_t), char)}},
                .assign = {archi_plist_assign__rcpointer, &data, sizeof(data), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_font_psf2_t font = archi_font_psf2_load(data, ARCHI_ERROR_PARAM);
    if (font == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = font,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__FONT_PSF2),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__font_psf2)
{
    archi_font_psf2_unload(context->ptr);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__font_psf2)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }

        if ((slot.index[0] < 0) || (slot.index[0] > ARCHI_STRING_UNICODE_CODEPOINT_MAX))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u]",
                    slot.index[0], ARCHI_STRING_UNICODE_CODEPOINT_MAX);
            return;
        }

        archi_rcpointer_t glyph = archi_font_psf2_glyph(context->ptr, slot.index[0]);
        if (glyph.cptr == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "no glyph exists for codepoint %lli",
                    slot.index[0]);
            return;
        }

        ARCHI_CONTEXT_YIELD(glyph);
    }
    else if (ARCHI_STRING_COMPARE("width", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        int width = archi_font_psf2_header(context->ptr).width;

        archi_rcpointer_t value = {
            .ptr = &width,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, int),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("height", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        int height = archi_font_psf2_header(context->ptr).height;

        archi_rcpointer_t value = {
            .ptr = &height,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, int),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__font_psf2 = {
    .init_fn = archi_context_init__font_psf2,
    .final_fn = archi_context_final__font_psf2,
    .eval_fn = archi_context_eval__font_psf2,
};

