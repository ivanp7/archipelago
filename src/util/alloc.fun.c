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
 * @brief Memory allocation utilities.
 */

#include "archi/util/alloc.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), realloc(), free()
#include <string.h> // for memcpy(), memset()

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

