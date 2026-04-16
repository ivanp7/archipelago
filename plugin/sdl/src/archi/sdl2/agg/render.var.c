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
 * @brief Aggregate type descriptions for data of operation functions for SDL2 rendering operations.
 */

#include "archi/sdl2/agg/render.var.h"
#include "archi/sdl2/exe/render.typ.h"
#include "archi/sdl2/api/tag.def.h"


static
const archi_aggr_member_type__value_t
VTYPE_bool = ARCHI_AGGR_MEMBER_TYPE__VALUE(bool, 0);

static
const archi_aggr_member_type__value_t
VTYPE_int = ARCHI_AGGR_MEMBER_TYPE__VALUE(int, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_data = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(void*, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_renderer = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(SDL_Renderer*,
        ARCHI_POINTER_DATA_TAG__SDL2_RENDERER);

static
const archi_aggr_member_type__pointer_t
PTYPE_texture = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(SDL_Texture*,
        ARCHI_POINTER_DATA_TAG__SDL2_TEXTURE);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_sdl2_rect[] = {
    ARCHI_AGGR_MEMBER__VALUE(SDL_Rect, x, 1, VTYPE_int),
    ARCHI_AGGR_MEMBER__VALUE(SDL_Rect, y, 1, VTYPE_int),
    ARCHI_AGGR_MEMBER__VALUE(SDL_Rect, w, 1, VTYPE_int),
    ARCHI_AGGR_MEMBER__VALUE(SDL_Rect, h, 1, VTYPE_int),
};

const archi_aggr_type_t
archi_aggr_type__sdl2_rect = ARCHI_AGGR_TYPE(
        SDL_Rect, 0,
        MEMBERS_sdl2_rect);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_node_data__sdl2_texture_copy[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__sdl2_texture_copy_t, texture, 1, PTYPE_texture),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__sdl2_texture_copy_t, entire, 1, VTYPE_bool),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_op_data__sdl2_texture_copy_t, area, 1,
            archi_aggr_type__sdl2_rect.top_level),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__sdl2_texture_copy_t, pixels, 1, PTYPE_data),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__sdl2_texture_copy_t, pitch, 1, VTYPE_int),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_node_data__sdl2_texture_copy = ARCHI_AGGR_TYPE(
        archi_dexgraph_op_data__sdl2_texture_copy_t, 0,
        MEMBERS_dexgraph_node_data__sdl2_texture_copy);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_node_data__sdl2_render_copy[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__sdl2_render_copy_t, renderer, 1, PTYPE_renderer),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__sdl2_render_copy_t, texture, 1, PTYPE_texture),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__sdl2_render_copy_t, entire_target, 1, VTYPE_bool),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__sdl2_render_copy_t, entire_source, 1, VTYPE_bool),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_op_data__sdl2_render_copy_t, target_area, 1,
            archi_aggr_type__sdl2_rect.top_level),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_op_data__sdl2_render_copy_t, source_area, 1,
            archi_aggr_type__sdl2_rect.top_level),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_node_data__sdl2_render_copy = ARCHI_AGGR_TYPE(
        archi_dexgraph_op_data__sdl2_render_copy_t, 0,
        MEMBERS_dexgraph_node_data__sdl2_render_copy);

