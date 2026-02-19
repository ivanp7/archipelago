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
 * @brief Aggregate object interface.
 */

#pragma once
#ifndef _ARCHI_AGGR_API_INTERFACE_TYP_H_
#define _ARCHI_AGGR_API_INTERFACE_TYP_H_

#include "archi_base/pointer.typ.h"
#include "archi_base/error.typ.h"
#include "archi_base/util/size.typ.h"


/**
 * @brief Member name and optional array indices.
 */
typedef struct archi_aggr_member_spec {
    const char *name; ///< Member name.
    size_t index; ///< Member array index.
} archi_aggr_member_spec_t;

/*****************************************************************************/

/**
 * @brief Signature of an aggregate type layout function.
 *
 * @return Valid type layout structure, or default-initialized in case of error.
 */
#define ARCHI_AGGR_LAYOUT_FUNC(func_name)       archi_layout_struct_t func_name(    \
        const void *metadata) /* [in] Aggregate type metadata. */

/**
 * @brief Type of aggregate type layout function.
 */
typedef ARCHI_AGGR_LAYOUT_FUNC((*archi_aggr_layout_func_t));

/**
 * @brief Signature of a function obtaining number of references in aggregate object.
 *
 * @return Number of references.
 */
#define ARCHI_AGGR_NUMREF_FUNC(func_name)       size_t func_name(   \
        size_t fam_length, /* [in] Flexible array length. */        \
        const void *metadata, /* [in] Aggregate type metadata. */   \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Type of function obtaining number of references in aggregate object.
 */
typedef ARCHI_AGGR_NUMREF_FUNC((*archi_aggr_numref_func_t));

/**
 * @brief Signature of a function obtaining index of a reference in aggregate object.
 *
 * @return Index of a reference, or (size_t)-1 if member is not a reference.
 */
#define ARCHI_AGGR_REFINDEX_FUNC(func_name)     size_t func_name(                           \
        size_t fam_length, /* [in] Flexible array length. */                                \
        const archi_aggr_member_spec_t submember[], /* [in] Submember specifier array. */   \
        size_t submember_depth, /* [in] Length of submember specifier array. */             \
        const void *metadata, /* [in] Aggregate type metadata. */                           \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Type of function obtaining index of a reference in aggregate object.
 */
typedef ARCHI_AGGR_REFINDEX_FUNC((*archi_aggr_refindex_func_t));

/**
 * @brief Signature of an aggregate object initialization function.
 */
#define ARCHI_AGGR_INIT_FUNC(func_name)         void func_name(     \
        void *object, /* [in] Aggregate object. */                  \
        size_t fam_length, /* [in] Flexible array length. */        \
        const void *metadata, /* [in] Aggregate type metadata. */   \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Type of aggregate object initialization function.
 */
typedef ARCHI_AGGR_INIT_FUNC((*archi_aggr_init_func_t));

/**
 * @brief Signature of an aggregate object finalization function.
 *
 * Contract:
 * (1) this function must not deallocate memory of an object.
 */
#define ARCHI_AGGR_FINAL_FUNC(func_name)        void func_name(     \
        void *object, /* [in] Aggregate object. */                  \
        size_t fam_length, /* [in] Flexible array length. */        \
        const void *metadata) /* [in] Aggregate type metadata. */

/**
 * @brief Type of aggregate object finalization function.
 */
typedef ARCHI_AGGR_FINAL_FUNC((*archi_aggr_final_func_t));

/**
 * @brief Signature of an aggregate object member getter function.
 *
 * `submember_depth` is never equal to zero; `submember` is never NULL.
 *
 * @return Pointer to a member, or NULL in case of failure.
 */
#define ARCHI_AGGR_GET_FUNC(func_name)          archi_pointer_t func_name(                  \
        void *object, /* [in] Aggregate object. */                                          \
        size_t fam_length, /* [in] Flexible array length. */                                \
        const archi_aggr_member_spec_t submember[], /* [in] Submember specifier array. */   \
        size_t submember_depth, /* [in] Length of submember specifier array. */             \
        const void *metadata, /* [in] Aggregate type metadata. */                           \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Type of aggregate object member getter function.
 */
typedef ARCHI_AGGR_GET_FUNC((*archi_aggr_get_func_t));

/**
 * @brief Signature of an aggregate object member setter function.
 *
 * `submember_depth` is never equal to zero; `submember` is never NULL.
 */
#define ARCHI_AGGR_SET_FUNC(func_name)          void func_name(                             \
        void *object, /* [in] Aggregate object. */                                          \
        size_t fam_length, /* [in] Flexible array length. */                                \
        const archi_aggr_member_spec_t submember[], /* [in] Submember specifier array. */   \
        size_t submember_depth, /* [in] Length of submember specifier array. */             \
        archi_rcpointer_t value, /* [in] Value to set. */                                   \
        const void *metadata, /* [in] Aggregate type metadata. */                           \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Type of aggregate object member setter function.
 */
typedef ARCHI_AGGR_SET_FUNC((*archi_aggr_set_func_t));

/*****************************************************************************/

/**
 * @brief Aggregate type interface functions.
 */
typedef struct archi_aggr_interface {
    archi_aggr_layout_func_t layout_fn; ///< Aggregate type layout function.

    archi_aggr_numref_func_t numref_fn;     ///< Aggregate object function for obtaining number of references.
    archi_aggr_refindex_func_t refindex_fn; ///< Aggregate object function for obtaining index of a reference.

    archi_aggr_init_func_t init_fn;     ///< Aggregate object initialization function.
    archi_aggr_final_func_t final_fn;   ///< Aggregate object finalization function.

    archi_aggr_get_func_t get_fn;       ///< Aggregate object member getter function.
    archi_aggr_set_func_t set_fn;       ///< Aggregate object member setter function.
} archi_aggr_interface_t;

#endif // _ARCHI_AGGR_API_INTERFACE_TYP_H_

