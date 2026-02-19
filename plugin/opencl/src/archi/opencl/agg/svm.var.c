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
 * @brief Aggregate type descriptions for data of memory interface functions for OpenCL SVM.
 */

#include "archi/opencl/agg/svm.var.h"
#include "archi/opencl/mem/svm.typ.h"
#include "archi/opencl/api/tag.def.h"


static
const archi_aggr_member_type__value_t
VTYPE_svm_mem_flags = ARCHI_AGGR_MEMBER_TYPE__VALUE(cl_svm_mem_flags, 0);

static
const archi_aggr_member_type__value_t
VTYPE_map_flags = ARCHI_AGGR_MEMBER_TYPE__VALUE(cl_map_flags, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_context = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(cl_context,
        ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT);

static
const archi_aggr_member_type__pointer_t
PTYPE_command_queue = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(cl_command_queue,
        ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_memory_alloc_data__opencl_svm[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_memory_alloc_data__opencl_svm_t, context, 1, PTYPE_context),
    ARCHI_AGGR_MEMBER__VALUE(archi_memory_alloc_data__opencl_svm_t, mem_flags, 1, VTYPE_svm_mem_flags),
};

const archi_aggr_type_t
archi_aggr_type__memory_alloc_data__opencl_svm = ARCHI_AGGR_TYPE(
        archi_memory_alloc_data__opencl_svm_t, MEMBERS_memory_alloc_data__opencl_svm);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_memory_map_data__opencl_svm[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_memory_map_data__opencl_svm_t, command_queue, 1, PTYPE_command_queue),
    ARCHI_AGGR_MEMBER__VALUE(archi_memory_map_data__opencl_svm_t, map_flags, 1, VTYPE_map_flags),
};

const archi_aggr_type_t
archi_aggr_type__memory_map_data__opencl_svm = ARCHI_AGGR_TYPE(
        archi_memory_map_data__opencl_svm_t, MEMBERS_memory_map_data__opencl_svm);

