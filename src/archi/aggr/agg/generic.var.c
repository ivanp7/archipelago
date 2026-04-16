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
 * @brief Generic aggregate type interface for arbitrary types.
 */

#include "archi/aggr/agg/generic.var.h"
#include "archi/aggr/agg/tag.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/string.fun.h"

#include <string.h> // for memcpy()


#define ARCHI_AGGR_TYPE_WALK_FUNC(func_name)   bool func_name( \
        archi_aggr_member_t member,                            \
        void *data,                                                 \
        ARCHI_ERROR_PARAM_DECL)

typedef ARCHI_AGGR_TYPE_WALK_FUNC((*archi_aggr_type_walk_func_t));

static
bool
archi_aggr_type_walk(
        const archi_aggr_member_type__aggregate_t *aggregate,
        archi_aggr_type_walk_func_t walk_fn,
        void *walk_data,
        size_t fam_length,
        ARCHI_ERROR_PARAM_DECL)
{
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate member description is NULL");
        return false;
    }
    else if (aggregate->members == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of aggregate type member descriptions is NULL");
        return false;
    }

    for (size_t i = 0; i < aggregate->num_members; i++)
    {
        if (aggregate->members[i].name == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate member name is NULL");
            return false;
        }
        else if (aggregate->members[i].layout == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate member type layout is NULL");
            return false;
        }

        switch (aggregate->members[i].kind)
        {
            case ARCHI_AGGR_MEMBER_KIND__VALUE:
            case ARCHI_AGGR_MEMBER_KIND__POINTER:
            case ARCHI_AGGR_MEMBER_KIND__AGGREGATE:
                break;

            default:
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unknown aggregate member type kind %i",
                        aggregate->members[i].kind);
                return false;
        }

        archi_aggr_member_t member = aggregate->members[i];

        if ((member.length == 0) && (i == aggregate->num_members - 1))
            member.length = fam_length;

        if (!walk_fn(member, walk_data, ARCHI_ERROR_PARAM))
            return false;
    }

    ARCHI_ERROR_RESET();
    return true;
}

static
ARCHI_AGGR_LAYOUT_FUNC(archi_aggr_layout__generic)
{
    const archi_aggr_type_t *aggregate = metadata;
    if (aggregate == NULL)
        return (archi_layout_struct_t){0};

    archi_layout_struct_t layout = {.base = aggregate->top_level.layout};
    {
        const archi_aggr_member_t *last_member = NULL;

        if ((aggregate->top_level.num_members != 0) && (aggregate->top_level.members != NULL))
            last_member = &aggregate->top_level.members[aggregate->top_level.num_members - 1];

        if ((last_member->length == 0) && (last_member->layout != NULL))
            layout.fam_stride = last_member->layout->size;
    }

    return layout;
}

static
ARCHI_AGGR_TYPE_WALK_FUNC(archi_aggr_type_walk__count_refs)
{
    size_t *num_refs = data;

    switch (member.kind)
    {
        case ARCHI_AGGR_MEMBER_KIND__POINTER:
            *num_refs += member.length;
            break;

        case ARCHI_AGGR_MEMBER_KIND__AGGREGATE:
            if (member.length != 0)
            {
                size_t num_subrefs = 0;
                if (!archi_aggr_type_walk(member.aggregate,
                            archi_aggr_type_walk__count_refs, &num_subrefs,
                            0, ARCHI_ERROR_PARAM))
                    return false;

                *num_refs += member.length * num_subrefs;
            }
            break;

        default:
            /* nothing to count */
            break;
    }

    return true;
}

static
ARCHI_AGGR_NUMREF_FUNC(archi_aggr_numref__generic)
{
    const archi_aggr_type_t *aggregate = metadata;
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type description is NULL");
        return 0;
    }

    size_t num_refs = 0;

    if (!archi_aggr_type_walk(&aggregate->top_level,
                archi_aggr_type_walk__count_refs, &num_refs,
                fam_length, ARCHI_ERROR_PARAM))
        return 0;

    return num_refs;
}

struct archi_aggr_type_walk_data__get_refindex {
    const archi_aggr_member_spec_t *submember;
    size_t submember_depth;

    size_t ref_index;
};

static
ARCHI_AGGR_TYPE_WALK_FUNC(archi_aggr_type_walk__get_refindex)
{
    struct archi_aggr_type_walk_data__get_refindex *walk_data = data;

    if (ARCHI_STRING_COMPARE(walk_data->submember->name, !=, member.name)) // skip over
    {
        switch (member.kind)
        {
            case ARCHI_AGGR_MEMBER_KIND__POINTER:
                walk_data->ref_index += member.length;
                break;

            case ARCHI_AGGR_MEMBER_KIND__AGGREGATE:
                if (member.length != 0)
                {
                    size_t num_subrefs = 0;
                    if (!archi_aggr_type_walk(member.aggregate,
                                archi_aggr_type_walk__count_refs, &num_subrefs,
                                0, ARCHI_ERROR_PARAM))
                        return false;

                    walk_data->ref_index += member.length * num_subrefs;
                }
                break;

            default:
                /* nothing to count */
                break;
        }

        return true;
    }
    else // delve deeper
    {
        if (walk_data->submember->index >= member.length)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "aggregate member index (%zu) is not less than member length (%zu)",
                    walk_data->submember->index, member.length);
            return false;
        }

        switch (member.kind)
        {
            case ARCHI_AGGR_MEMBER_KIND__VALUE:
                if (walk_data->submember_depth != 1)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "scalar member of an aggregate object cannot have submembers");
                    return false;
                }

                walk_data->ref_index = -1; // the submember is not a reference
                break;

            case ARCHI_AGGR_MEMBER_KIND__POINTER:
                if (walk_data->submember_depth != 1)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "scalar member of an aggregate object cannot have submembers");
                    return false;
                }

                walk_data->ref_index += walk_data->submember->index;
                break;

            case ARCHI_AGGR_MEMBER_KIND__AGGREGATE:
                if (walk_data->submember_depth == 1)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate member of an aggregate object cannot be accessed as a whole");
                    return false;
                }

                if (walk_data->submember->index != 0)
                {
                    size_t num_subrefs = 0;
                    if (!archi_aggr_type_walk(member.aggregate,
                                archi_aggr_type_walk__count_refs, &num_subrefs,
                                0, ARCHI_ERROR_PARAM))
                        return false;

                    walk_data->ref_index += walk_data->submember->index * num_subrefs;
                }

                walk_data->submember++;
                walk_data->submember_depth--;

                if (archi_aggr_type_walk(member.aggregate,
                            archi_aggr_type_walk__get_refindex, walk_data,
                            0, ARCHI_ERROR_PARAM))
                {
                    ARCHI_ERROR_SET(ARCHI__EKEY, "unknown submember specified");
                    return false;
                }
                break;
        }

        return false;
    }
}

static
ARCHI_AGGR_REFINDEX_FUNC(archi_aggr_refindex__generic)
{
    const archi_aggr_type_t *aggregate = metadata;
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type description is NULL");
        return 0;
    }

    struct archi_aggr_type_walk_data__get_refindex walk_data = {
        .submember = submember,
        .submember_depth = submember_depth,
    };

    ARCHI_ERROR_VAR(error);

    bool uninterrupted = archi_aggr_type_walk(&aggregate->top_level,
            archi_aggr_type_walk__get_refindex, &walk_data,
            fam_length, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return -1;
    else if (uninterrupted)
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown submember specified");
        return -1;
    }

    return walk_data.ref_index;
}

static
ARCHI_AGGR_INIT_FUNC(archi_aggr_init__generic)
{
    const archi_aggr_type_t *aggregate = metadata;
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type description is NULL");
        return;
    }

    if (aggregate->top_level.layout.size != 0)
    {
        // Initialize the object
        if (aggregate->init_value == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate object initialization value is NULL");
            return;
        }

        memcpy(object, aggregate->init_value, aggregate->top_level.layout.size);
    }

    // Get the last member of the aggregate
    const archi_aggr_member_t *last_member = NULL;

    if ((aggregate->top_level.num_members != 0) && (aggregate->top_level.members != NULL))
        last_member = &aggregate->top_level.members[aggregate->top_level.num_members - 1];

    if (last_member->length == 0)
    {
        // Initialize the flexible array member
        if (last_member->layout == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "flexible array member description has no type description");
            return;
        }

        char *fam_element = (char*)object + aggregate->top_level.layout.size;
        for (size_t i = 0; i < fam_length; i++)
        {
            memcpy(fam_element, aggregate->init_value_fam, last_member->layout->size);
            fam_element += last_member->layout->size;
        }
    }
    else
    {
        if (fam_length != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "non-zero flexible array length provided for an aggregate type without FAM");
            return;
        }
    }

    ARCHI_ERROR_RESET();
}

struct archi_aggr_type_walk_data__get_submember {
    const archi_aggr_member_spec_t *submember;
    size_t submember_depth;

    size_t offset;
    archi_aggr_member_t member;
};

static
ARCHI_AGGR_TYPE_WALK_FUNC(archi_aggr_type_walk__get_submember)
{
    struct archi_aggr_type_walk_data__get_submember *walk_data = data;

    if (ARCHI_STRING_COMPARE(walk_data->submember->name, !=, member.name)) // skip over
        return true;
    else // delve deeper
    {
        if (walk_data->submember->index >= member.length)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "aggregate member index (%zu) is not less than member length (%zu)",
                    walk_data->submember->index, member.length);
            return false;
        }

        walk_data->offset += member.offset + walk_data->submember->index * member.layout->size;

        if (walk_data->submember_depth != 1)
        {
            if (member.kind != ARCHI_AGGR_MEMBER_KIND__AGGREGATE)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "scalar member of an aggregate object cannot have submembers");
                return false;
            }

            walk_data->submember++;
            walk_data->submember_depth--;

            if (archi_aggr_type_walk(member.aggregate,
                        archi_aggr_type_walk__get_submember, walk_data,
                        0, ARCHI_ERROR_PARAM))
            {
                ARCHI_ERROR_SET(ARCHI__EKEY, "unknown submember specified");
                return false;
            }
        }
        else
        {
            member.length -= walk_data->submember->index; // number of remaining elements
            walk_data->member = member;
        }

        return false;
    }
}

static
ARCHI_AGGR_GET_FUNC(archi_aggr_get__generic)
{
    const archi_aggr_type_t *aggregate = metadata;
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type description is NULL");
        return (archi_pointer_t){0};
    }

    struct archi_aggr_type_walk_data__get_submember walk_data = {
        .submember = submember,
        .submember_depth = submember_depth,
    };

    ARCHI_ERROR_VAR(error);

    bool uninterrupted = archi_aggr_type_walk(&aggregate->top_level,
            archi_aggr_type_walk__get_submember, &walk_data,
            fam_length, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return (archi_pointer_t){0};
    else if (uninterrupted)
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown submember specified");
        return (archi_pointer_t){0};
    }

    // Get the member value
    archi_pointer_attr_t attr;

    switch (walk_data.member.kind)
    {
        case ARCHI_AGGR_MEMBER_KIND__VALUE:
            {
                const archi_aggr_member_type__value_t *value_type = walk_data.member.value;

                if (value_type->value_tag == 0)
                {
                    attr = archi_pointer_attr__pdata(walk_data.member.length,
                            value_type->layout.size, value_type->layout.alignment,
                            ARCHI_ERROR_PARAM);

                    if (attr == (archi_pointer_attr_t)-1)
                        return (archi_pointer_t){0};
                }
                else
                {
                    attr = archi_pointer_attr__cdata(value_type->value_tag);

                    if (attr == (archi_pointer_attr_t)-1)
                    {
                        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data tag of a value member is invalid");
                        return (archi_pointer_t){0};
                    }
                }
            }
            break;

        case ARCHI_AGGR_MEMBER_KIND__POINTER:
            {
                const archi_aggr_member_type__pointer_t *pointer_type = walk_data.member.pointer;

                attr = archi_pointer_attr__pdata(1, pointer_type->layout.size, pointer_type->layout.alignment,
                        ARCHI_ERROR_PARAM);

                if (attr == (archi_pointer_attr_t)-1)
                    return (archi_pointer_t){0};
            }
            break;

        default:
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "specified submember kind is not value or pointer");
            return (archi_pointer_t){0};
    }

    ARCHI_ERROR_RESET();
    return (archi_pointer_t){
        .ptr = (char*)object + walk_data.offset,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE | attr,
    };
}

static
ARCHI_AGGR_SET_FUNC(archi_aggr_set__generic)
{
    const archi_aggr_type_t *aggregate = metadata;
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type description is NULL");
        return;
    }

    struct archi_aggr_type_walk_data__get_submember walk_data = {
        .submember = submember,
        .submember_depth = submember_depth,
    };

    ARCHI_ERROR_VAR(error);

    bool uninterrupted = archi_aggr_type_walk(&aggregate->top_level,
            archi_aggr_type_walk__get_submember, &walk_data,
            fam_length, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return;
    else if (uninterrupted)
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown submember specified");
        return;
    }

    // Assign the value
    switch (walk_data.member.kind)
    {
        case ARCHI_AGGR_MEMBER_KIND__VALUE:
            {
                const archi_aggr_member_type__value_t *value_type = walk_data.member.value;

                archi_pointer_attr_t attr;

                if (value_type->value_tag == 0)
                {
                    attr = archi_pointer_attr__pdata(1, value_type->layout.size, value_type->layout.alignment,
                            ARCHI_ERROR_PARAM);

                    if (attr == (archi_pointer_attr_t)-1)
                        return;
                }
                else
                {
                    attr = archi_pointer_attr__cdata(value_type->value_tag);

                    if (attr == (archi_pointer_attr_t)-1)
                    {
                        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data tag of a value member is invalid");
                        return;
                    }
                }

                if (!archi_pointer_attr_compatible(value.attr, attr))
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not compatible with the specified submember");
                    return;
                }
                else if (value.ptr == NULL)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "value pointer is NULL");
                    return;
                }

                // Copy the data
                memmove((char*)object + walk_data.offset, value.ptr, value_type->layout.size);
            }
            break;

        case ARCHI_AGGR_MEMBER_KIND__POINTER:
            {
                const archi_aggr_member_type__pointer_t *pointer_type = walk_data.member.pointer;

                archi_pointer_attr_t attr;

                switch (pointer_type->pointee_kind)
                {
                    case ARCHI_POINTEE__UNSPEC:
                        break;

                    case ARCHI_POINTEE__PDATA:
                        attr = archi_pointer_attr__pdata(pointer_type->pointee_layout.length,
                                pointer_type->pointee_layout.base.size, pointer_type->pointee_layout.base.alignment,
                                ARCHI_ERROR_PARAM);

                        if (attr == (archi_pointer_attr_t)-1)
                            return;

                        break;

                    case ARCHI_POINTEE__CDATA:
                        attr = archi_pointer_attr__cdata(pointer_type->pointee_tag);

                        if (attr == (archi_pointer_attr_t)-1)
                        {
                            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data tag of a pointer member is invalid");
                            return;
                        }

                        break;

                    case ARCHI_POINTEE__FUNC:
                        attr = archi_pointer_attr__func(pointer_type->pointee_tag);

                        if (attr == 0)
                        {
                            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "function tag of a pointer member is invalid");
                            return;
                        }

                        break;

                    default:
                        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unknown pointee type kind %i for pointer member encountered",
                                pointer_type->pointee_kind);
                        return;
                }

                if ((pointer_type->pointee_kind != ARCHI_POINTEE__UNSPEC) &&
                        !archi_pointer_attr_compatible(value.attr, attr))
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not compatible with the specified submember");
                    return;
                }

                // Copy the pointer
                memcpy((char*)object + walk_data.offset, &value.ptr, sizeof(pointer_type->layout.size));
            }
            break;

        default:
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "specified submember kind is not value or pointer");
            return;
    }

    ARCHI_ERROR_RESET();
}

const archi_aggr_interface_t
archi_aggr_interface__generic = {
    .metadata_tag = ARCHI_POINTER_DATA_TAG__AGGR_TYPE,
    .layout_fn = archi_aggr_layout__generic,
    .numref_fn = archi_aggr_numref__generic,
    .refindex_fn = archi_aggr_refindex__generic,
    .init_fn = archi_aggr_init__generic,
    .get_fn = archi_aggr_get__generic,
    .set_fn = archi_aggr_set__generic,
};

