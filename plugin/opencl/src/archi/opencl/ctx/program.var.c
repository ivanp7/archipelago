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
 * @brief Context interface for OpenCL programs.
 */

#include "archi/opencl/ctx/program.var.h"
#include "archi/opencl/api/program.fun.h"
#include "archi/opencl/api/device.fun.h"
#include "archi/opencl/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <CL/cl.h>

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()


struct archi_context_data__opencl_program {
    archi_rcpointer_t program;
    archi_opencl_platdev_t *platdev;
    archi_opencl_program_bin_t binaries;

    // References
    archi_rcpointer_t ref_context;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__opencl_program_src)
{
    // Parse parameters
    archi_rcpointer_t opencl_context = {0};
    size_t num_devices = 0;
    const cl_device_id *device_id = NULL;
    const archi_kvlist_t *list_headers = NULL;
    const archi_kvlist_t *list_sources = NULL;
    size_t num_libraries = 0;
    const cl_program *libraries = NULL;
    const char *cflags = NULL;
    const char *lflags = NULL;
    {
        archi_rcpointer_t device_ids = {0}, library_programs = {0};

        archi_plist_param_t parsed[] = {
            {.name = "context",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT)}},
                .assign = {archi_plist_assign__rcpointer, &opencl_context, sizeof(opencl_context), NULL}},
            {.name = "device_id",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, cl_device_id)}},
                .assign = {archi_plist_assign__rcpointer, &device_ids, sizeof(device_ids), NULL}},
            {.name = "headers",
                .check = {archi_value_check__attrs, (archi_pointer_attr_t[]){-1, archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KVLIST),
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KRCVLIST), -1}},
                .assign = {archi_plist_assign__dptr, &list_headers, sizeof(list_headers), NULL}},
            {.name = "sources",
                .check = {archi_value_check__attrs, (archi_pointer_attr_t[]){-1, archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KVLIST),
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KRCVLIST), -1}},
                .assign = {archi_plist_assign__dptr, &list_sources, sizeof(list_sources), NULL}},
            {.name = "libraries",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(0, cl_program)}},
                .assign = {archi_plist_assign__rcpointer, &library_programs, sizeof(library_programs), NULL}},
            {.name = "cflags",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(0, char)}},
                .assign = {archi_plist_assign__dptr_n, &cflags, sizeof(cflags), NULL}},
            {.name = "lflags",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(0, char)}},
                .assign = {archi_plist_assign__dptr_n, &lflags, sizeof(lflags), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;

        device_id = device_ids.cptr;
        archi_pointer_attr_unpk__pdata(device_ids.attr, &num_devices, NULL, NULL, NULL);

        ARCHI_POINTER_NULLIFY_EMPTY(library_programs);
        libraries = library_programs.cptr;
        archi_pointer_attr_unpk__pdata(library_programs.attr, &num_libraries, NULL, NULL, NULL);
    }

    if (opencl_context.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL context is not specified");
        return NULL;
    }
    else if (num_devices == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no devices are specified");
        return NULL;
    }

    // Construct the context
    struct archi_context_data__opencl_program *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__opencl_program){
        .program = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM),
        },
        .ref_context = archi_rcpointer_own(opencl_context, ARCHI_ERROR_PARAM),
    };

    if (!context_data->ref_context.attr)
        goto failure;

    ARCHI_ERROR_VAR(error);

    // Prepare program headers and sources
    archi_opencl_program_src_t headers =
        archi_opencl_program_src_from_kvlist(list_headers, false, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        goto failure;

    ARCHI_ERROR_VAR_UNSET(&error);

    archi_opencl_program_src_t sources =
        archi_opencl_program_src_from_kvlist(list_sources, false, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
    {
        archi_opencl_program_src_free(headers, false);
        goto failure;
    }

    // Build the program
    ARCHI_ERROR_VAR_UNSET(&error);

    context_data->program.ptr = archi_opencl_program_build(opencl_context.ptr,
            num_devices, device_id, headers, sources,
            num_libraries, libraries, cflags, lflags,
            true, &error);
    ARCHI_ERROR_ASSIGN(error);

    archi_opencl_program_src_free(headers, false);
    archi_opencl_program_src_free(sources, false);

    if (context_data->program.ptr == NULL)
        goto failure;

    // Extract the program binaries
    ARCHI_ERROR_VAR_UNSET(&error);

    context_data->binaries = archi_opencl_program_bin_extract(
            context_data->program.ptr, &context_data->platdev, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        goto failure;

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    if (context_data->program.ptr != NULL)
        clReleaseProgram(context_data->program.ptr);
    archi_rcpointer_disown(context_data->ref_context);
    free(context_data->platdev);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__opencl_program_bin)
{
    // Parse parameters
    archi_rcpointer_t opencl_context = {0};
    size_t num_devices = 0;
    const cl_device_id *device_id = NULL;
    archi_rcpointer_t array_binaries = {0};
    archi_rcpointer_t array_binary_sizes = {0};
    bool build = false;
    {
        archi_rcpointer_t device_ids = {0};

        archi_plist_param_t parsed[] = {
            {.name = "context",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT)}},
                .assign = {archi_plist_assign__rcpointer, &opencl_context, sizeof(opencl_context), NULL}},
            {.name = "device_id",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, cl_device_id)}},
                .assign = {archi_plist_assign__rcpointer, &device_ids, sizeof(device_ids), NULL}},
            {.name = "binaries",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char*)}},
                .assign = {archi_plist_assign__rcpointer, &array_binaries, sizeof(array_binaries), NULL}},
            {.name = "binary_sizes",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__rcpointer, &array_binary_sizes, sizeof(array_binary_sizes), NULL}},
            {.name = "build",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &build, sizeof(build), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;

        device_id = device_ids.cptr;
        archi_pointer_attr_unpk__pdata(device_ids.attr, &num_devices, NULL, NULL, NULL);
    }

    if (opencl_context.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL context is not specified");
        return NULL;
    }
    else if (num_devices == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no devices are specified");
        return NULL;
    }

    {
        size_t length;
        archi_pointer_attr_unpk__pdata(array_binaries.attr, &length, NULL, NULL, NULL);

        if (length != num_devices)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of binaries doesn't match number of devices");
            return NULL;
        }
    }

    {
        size_t length;
        archi_pointer_attr_unpk__pdata(array_binary_sizes.attr, &length, NULL, NULL, NULL);

        if (length != num_devices)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of binary sizes doesn't match number of binaries");
            return NULL;
        }
    }

    // Construct the context
    struct archi_context_data__opencl_program *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__opencl_program){
        .program = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM),
        },
        .ref_context = archi_rcpointer_own(opencl_context, ARCHI_ERROR_PARAM),
    };

    if (!context_data->ref_context.attr)
        goto failure;

    // Allocate platform/device IDs storage object
    context_data->platdev = archi_opencl_platdev_alloc(num_devices);
    if (context_data->platdev == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate platform/device IDs storage object");
        goto failure;
    }

    // Copy device IDs
    memcpy(context_data->platdev->device_id, device_id, sizeof(*device_id) * num_devices);

    // Get platform ID
    {
        cl_int ret = clGetDeviceInfo(*(cl_device_id*)device_id, CL_DEVICE_PLATFORM,
                sizeof(context_data->platdev->platform_id),
                &context_data->platdev->platform_id, NULL);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't obtain platform ID of the devices");
            goto failure;
        }
    }

    // Copy the program binaries
    ARCHI_ERROR_VAR(error);

    archi_opencl_program_bin_t binaries = {
        .num_binaries = num_devices,
        .content = array_binaries.ptr,
        .size = array_binary_sizes.ptr,
    };

    context_data->binaries = archi_opencl_program_bin_copy(binaries, true, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        goto failure;

    // Create the program
    ARCHI_ERROR_VAR_UNSET(&error);

    context_data->program.ptr = archi_opencl_program_create(opencl_context.ptr,
            context_data->platdev, binaries, build, true, ARCHI_ERROR_PARAM);

    if (context_data->program.ptr == NULL)
        goto failure;

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    if (context_data->program.ptr != NULL)
        clReleaseProgram(context_data->program.ptr);
    archi_rcpointer_disown(context_data->ref_context);
    archi_opencl_program_bin_free(context_data->binaries, true);
    free(context_data->platdev);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__opencl_program)
{
    struct archi_context_data__opencl_program *context_data =
        (struct archi_context_data__opencl_program*)context;

    clReleaseProgram(context_data->program.ptr);
    archi_rcpointer_disown(context_data->ref_context);
    archi_opencl_program_bin_free(context_data->binaries, true);
    free(context_data->platdev);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__opencl_program)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__opencl_program *context_data =
        (struct archi_context_data__opencl_program*)context;

    if (ARCHI_STRING_COMPARE("context", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_context);
    }
    else if (ARCHI_STRING_COMPARE("platform_id", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_rcpointer_t value = {
            .ptr = context_data->platdev->platform_id,
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("device_id", ==, slot.name))
    {
        if (slot.num_indices > 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0 or 1");
            return;
        }

        archi_rcpointer_t value;

        if (slot.num_indices == 0)
        {
            value = (archi_rcpointer_t){
                .ptr = context_data->platdev->device_id,
                .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                    ARCHI_POINTER_ATTR__PDATA(context_data->platdev->num_devices, cl_device_id),
                .ref_count = ARCHI_CONTEXT_REF_COUNT,
            };
        }
        else
        {
            if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= context_data->platdev->num_devices))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u)",
                        slot.index[0], context_data->platdev->num_devices);
                return;
            }

            value = (archi_rcpointer_t){
                .ptr = context_data->platdev->device_id[slot.index[0]],
                .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID),
            };
        }

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("binary.size", ==, slot.name))
    {
        if (slot.num_indices > 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0 or 1");
            return;
        }

        archi_rcpointer_t value;

        if (slot.num_indices == 0)
        {
            value = (archi_rcpointer_t){
                .ptr = context_data->binaries.size,
                .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                    ARCHI_POINTER_ATTR__PDATA(context_data->platdev->num_devices, size_t),
                .ref_count = ARCHI_CONTEXT_REF_COUNT,
            };
        }
        else
        {
            if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= context_data->platdev->num_devices))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u)",
                        slot.index[0], context_data->platdev->num_devices);
                return;
            }

            size_t size = context_data->binaries.size[slot.index[0]];

            value = (archi_rcpointer_t){
                .ptr = &size,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, size_t),
            };
        }

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("binary", ==, slot.name))
    {
        if (slot.num_indices > 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0 or 1");
            return;
        }

        archi_rcpointer_t value;

        if (slot.num_indices == 0)
        {
            value = (archi_rcpointer_t){
                .ptr = context_data->binaries.content,
                .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                    ARCHI_POINTER_ATTR__PDATA(context_data->platdev->num_devices, unsigned char*),
                .ref_count = ARCHI_CONTEXT_REF_COUNT,
            };
        }
        else
        {
            if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= context_data->platdev->num_devices))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u)",
                        slot.index[0], context_data->platdev->num_devices);
                return;
            }

            value = (archi_rcpointer_t){
                .ptr = context_data->binaries.content[slot.index[0]],
                .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                    ARCHI_POINTER_ATTR__PDATA(context_data->binaries.size[slot.index[0]], unsigned char),
                .ref_count = ARCHI_CONTEXT_REF_COUNT,
            };
        }

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__opencl_program_src = {
    .init_fn = archi_context_init__opencl_program_src,
    .final_fn = archi_context_final__opencl_program,
    .eval_fn = archi_context_eval__opencl_program,
};

const archi_context_interface_t
archi_context_interface__opencl_program_bin = {
    .init_fn = archi_context_init__opencl_program_bin,
    .final_fn = archi_context_final__opencl_program,
    .eval_fn = archi_context_eval__opencl_program,
};

