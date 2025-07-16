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
 * @brief Context interface for arrays of pointers.
 */

#include "archi/ctx/interface/array.var.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), realloc(), free()
#include <string.h> // for strcmp(), memcpy(), memset()
#include <stdalign.h> // for alignof()

struct archi_context_array_data {
    archi_pointer_t array;
    archi_pointer_t *element;
    bool func_ptrs;
};

static
void
archi_resize_array_init_new_elements(
        void *ptr,

        const void *new_element,
        size_t element_size,
        size_t padded_size,

        size_t old_num_elements,
        size_t new_num_elements)
{
    if (new_element != NULL)
    {
        for (size_t i = old_num_elements; i < new_num_elements; i++)
            memcpy((char*)ptr + padded_size * i, new_element, element_size);
    }
    else
    {
        memset((char*)ptr + padded_size * old_num_elements, 0,
                padded_size * (new_num_elements - old_num_elements));
    }
}

/**
 * @brief Resize array of values together with array of references to individual elements.
 *
 * This function is protected from intermediate memory allocation errors.
 * If such an error occurs, the original arrays are not modified.
 *
 * If the array is shrunk, reference counters of deleted elements are decremented.
 *
 * `new_size` can be zero, in which case arrays are freed and pointers set to NULL.
 * If `empty_element` is NULL, the new array memory is memset() to 0.
 *
 * @return Status code.
 */
static
archi_status_t
archi_resize_array(
        archi_pointer_t *array,
        archi_pointer_t **elements,

        size_t new_num_elements,
        const void *new_element)
{
    if (array == NULL)
        return ARCHI_STATUS_EMISUSE;
    else if (array->element.size == 0)
        return ARCHI_STATUS_EMISUSE;

    size_t old_num_elements = array->element.num_of;

    if (new_num_elements == old_num_elements)
        return 0;

    size_t padded_size = array->element.size;
    if (array->element.alignment != 0)
        padded_size = ARCHI_SIZE_PADDED(padded_size, array->element.alignment);

    if (elements == NULL) // resizing a single array, use realloc()
    {
        if (new_num_elements > 0)
        {
            size_t new_full_size = padded_size * new_num_elements;

            // Check for integer overflow
            if (new_full_size / padded_size != new_num_elements)
                return ARCHI_STATUS_ENOMEMORY;

            // Reallocate the array
            void *new_ptr = realloc(array->ptr, new_full_size);
            if (new_ptr == NULL)
                return ARCHI_STATUS_ENOMEMORY;

            array->ptr = new_ptr;

            if (new_num_elements > old_num_elements)
                archi_resize_array_init_new_elements(new_ptr, new_element, array->element.size,
                        padded_size, old_num_elements, new_num_elements);
        }
        else
        {
            free(array->ptr);
            array->ptr = NULL;
        }

        array->element.num_of = new_num_elements;
    }
    else // resizing two arrays, use malloc()
    {
        void *new_ptr = NULL;

        // Allocate new array of contents
        if (new_num_elements > 0)
        {
            size_t new_full_size = padded_size * new_num_elements;

            // Check for integer overflow
            if (new_full_size / padded_size != new_num_elements)
                return ARCHI_STATUS_ENOMEMORY;

            // Allocate new memory
            new_ptr = malloc(new_full_size);
            if (new_ptr == NULL)
                return ARCHI_STATUS_ENOMEMORY;

            // Copy array contents and initialize new elements
            if (new_num_elements > old_num_elements)
            {
                memcpy(new_ptr, array->ptr, padded_size * old_num_elements);

                archi_resize_array_init_new_elements(new_ptr, new_element, array->element.size,
                        padded_size, old_num_elements, new_num_elements);
            }
            else
                memcpy(new_ptr, array->ptr, new_full_size);
        }

        // Allocate new array of references
        archi_pointer_t *new_elements = NULL;

        if (new_num_elements > 0)
        {
            size_t new_full_size = sizeof(*new_elements) * new_num_elements;

            // Check for integer overflow
            if (new_full_size / sizeof(*new_elements) != new_num_elements)
            {
                free(new_ptr);
                return ARCHI_STATUS_ENOMEMORY;
            }

            // Allocate new memory
            new_elements = malloc(new_full_size);
            if (new_elements == NULL)
            {
                free(new_ptr);
                return ARCHI_STATUS_ENOMEMORY;
            }

            // Copy array contents and initialize new elements
            if (new_num_elements > old_num_elements)
            {
                for (size_t i = 0; i < old_num_elements; i++)
                    new_elements[i] = (*elements)[i];

                for (size_t i = old_num_elements; i < new_num_elements; i++)
                    new_elements[i] = (archi_pointer_t){0};
            }
            else
            {
                for (size_t i = 0; i < new_num_elements; i++)
                    new_elements[i] = (*elements)[i];

                // Decrement reference counts
                for (size_t i = old_num_elements; i-- > new_num_elements;)
                    archi_reference_count_decrement((*elements)[i].ref_count);
            }
        }

        // Free old arrays and set output parameters
        free(array->ptr);
        array->ptr = new_ptr;
        array->element.num_of = new_num_elements;

        free(*elements);
        *elements = new_elements;
    }

    return 0;
}

ARCHI_CONTEXT_INIT_FUNC(archi_context_array_init)
{
    archi_pointer_flags_t flags = 0;
    size_t num_elements = 0;
    bool func_ptrs = false;

    bool param_flags_set = false;
    bool param_num_elements_set = false;
    bool param_func_ptrs_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("num_elements", params->name) == 0)
        {
            if (param_num_elements_set)
                continue;
            param_num_elements_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            num_elements = *(size_t*)params->value.ptr;
        }
        else if (strcmp("flags", params->name) == 0)
        {
            if (param_flags_set)
                continue;
            param_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            flags = *(archi_pointer_flags_t*)params->value.ptr;
        }
        else if (strcmp("func_ptrs", params->name) == 0)
        {
            if (param_func_ptrs_set)
                continue;
            param_func_ptrs_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            func_ptrs = *(char*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (flags & ARCHI_POINTER_FLAG_FUNCTION)
        return ARCHI_STATUS_EMISUSE;

    struct archi_context_array_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *context_data = (struct archi_context_array_data){
        .array = {
            .flags = flags,
            .element = {
                .size = !func_ptrs ? sizeof(void*) : sizeof(archi_function_t),
                .alignment = !func_ptrs ? alignof(void*) : alignof(archi_function_t),
            },
        },
        .func_ptrs = func_ptrs,
    };

    archi_status_t code;
    {
        void *null_ptr = NULL;
        archi_function_t null_fptr = NULL;

        const void *new_element = !func_ptrs ? (void*)&null_ptr : (void*)&null_fptr;

        code = archi_resize_array(&context_data->array,
                &context_data->element, num_elements, new_element);
    }

    if (code != 0)
    {
        free(context_data);
        return code;
    }

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_array_final)
{
    struct archi_context_array_data *context_data =
        (struct archi_context_array_data*)context;

    archi_resize_array(&context_data->array, &context_data->element, 0, NULL);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_array_get)
{
    struct archi_context_array_data *context_data =
        (struct archi_context_array_data*)context;

    if (strcmp("", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= context_data->array.element.num_of))
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->element[slot.index[0]];
    }
    else if (strcmp("elements", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = context_data->element,
            .ref_count = context_data->array.ref_count,
            .element = {
                .num_of = context_data->array.element.num_of,
                .size = sizeof(*context_data->element),
                .alignment = alignof(archi_pointer_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_array_set)
{
    struct archi_context_array_data *context_data =
        (struct archi_context_array_data*)context;

    if (strcmp("", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= context_data->array.element.num_of))
            return ARCHI_STATUS_EMISUSE;

        if (!context_data->func_ptrs)
        {
            if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EMISUSE;
        }
        else
        {
            if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EMISUSE;
        }

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->element[slot.index[0]].ref_count);

        if (!context_data->func_ptrs)
        {
            void **array = context_data->array.ptr;
            array[slot.index[0]] = value.ptr;
        }
        else
        {
            archi_function_t *array = context_data->array.ptr;
            array[slot.index[0]] = value.fptr;
        }

        context_data->element[slot.index[0]] = value;
    }
    else if (strcmp("num_elements", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EMISUSE;

        size_t num_elements = *(size_t*)value.ptr;

        archi_status_t code;
        {
            void *null_ptr = NULL;
            archi_function_t null_fptr = NULL;

            const void *new_element = !context_data->func_ptrs ? (void*)&null_ptr : (void*)&null_fptr;

            code = archi_resize_array(&context_data->array,
                    &context_data->element, num_elements, new_element);
        }

        if (code != 0)
            return code;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_array_interface = {
    .init_fn = archi_context_array_init,
    .final_fn = archi_context_array_final,
    .get_fn = archi_context_array_get,
    .set_fn = archi_context_array_set,
};

