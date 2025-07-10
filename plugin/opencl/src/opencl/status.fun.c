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
 * @brief Functions for getting OpenCL status names from codes.
 */

#include "archip/opencl/status.fun.h"

#define CASE_STR(x) case x: return #x;

const char*
archip_opencl_error_string(
        cl_int err)
{
    switch (err)
    {
        // run-time and JIT compiler errors
        CASE_STR(CL_SUCCESS)
        CASE_STR(CL_DEVICE_NOT_FOUND)
        CASE_STR(CL_DEVICE_NOT_AVAILABLE)
        CASE_STR(CL_COMPILER_NOT_AVAILABLE)
        CASE_STR(CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CASE_STR(CL_OUT_OF_RESOURCES)
        CASE_STR(CL_OUT_OF_HOST_MEMORY)
        CASE_STR(CL_PROFILING_INFO_NOT_AVAILABLE)
        CASE_STR(CL_MEM_COPY_OVERLAP)
        CASE_STR(CL_IMAGE_FORMAT_MISMATCH)
        CASE_STR(CL_IMAGE_FORMAT_NOT_SUPPORTED)
        CASE_STR(CL_BUILD_PROGRAM_FAILURE)
        CASE_STR(CL_MAP_FAILURE)
        CASE_STR(CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CASE_STR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CASE_STR(CL_COMPILE_PROGRAM_FAILURE)
        CASE_STR(CL_LINKER_NOT_AVAILABLE)
        CASE_STR(CL_LINK_PROGRAM_FAILURE)
        CASE_STR(CL_DEVICE_PARTITION_FAILED)
        CASE_STR(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)

        // compile-time errors
        CASE_STR(CL_INVALID_VALUE)
        CASE_STR(CL_INVALID_DEVICE_TYPE)
        CASE_STR(CL_INVALID_PLATFORM)
        CASE_STR(CL_INVALID_DEVICE)
        CASE_STR(CL_INVALID_CONTEXT)
        CASE_STR(CL_INVALID_QUEUE_PROPERTIES)
        CASE_STR(CL_INVALID_COMMAND_QUEUE)
        CASE_STR(CL_INVALID_HOST_PTR)
        CASE_STR(CL_INVALID_MEM_OBJECT)
        CASE_STR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        CASE_STR(CL_INVALID_IMAGE_SIZE)
        CASE_STR(CL_INVALID_SAMPLER)
        CASE_STR(CL_INVALID_BINARY)
        CASE_STR(CL_INVALID_BUILD_OPTIONS)
        CASE_STR(CL_INVALID_PROGRAM)
        CASE_STR(CL_INVALID_PROGRAM_EXECUTABLE)
        CASE_STR(CL_INVALID_KERNEL_NAME)
        CASE_STR(CL_INVALID_KERNEL_DEFINITION)
        CASE_STR(CL_INVALID_KERNEL)
        CASE_STR(CL_INVALID_ARG_INDEX)
        CASE_STR(CL_INVALID_ARG_VALUE)
        CASE_STR(CL_INVALID_ARG_SIZE)
        CASE_STR(CL_INVALID_KERNEL_ARGS)
        CASE_STR(CL_INVALID_WORK_DIMENSION)
        CASE_STR(CL_INVALID_WORK_GROUP_SIZE)
        CASE_STR(CL_INVALID_WORK_ITEM_SIZE)
        CASE_STR(CL_INVALID_GLOBAL_OFFSET)
        CASE_STR(CL_INVALID_EVENT_WAIT_LIST)
        CASE_STR(CL_INVALID_EVENT)
        CASE_STR(CL_INVALID_OPERATION)
        CASE_STR(CL_INVALID_GL_OBJECT)
        CASE_STR(CL_INVALID_BUFFER_SIZE)
        CASE_STR(CL_INVALID_MIP_LEVEL)
        CASE_STR(CL_INVALID_GLOBAL_WORK_SIZE)
        CASE_STR(CL_INVALID_PROPERTY)
        CASE_STR(CL_INVALID_IMAGE_DESCRIPTOR)
        CASE_STR(CL_INVALID_COMPILER_OPTIONS)
        CASE_STR(CL_INVALID_LINKER_OPTIONS)
        CASE_STR(CL_INVALID_DEVICE_PARTITION_COUNT)

        default: return NULL;
    }
}

const char*
archip_opencl_build_status_string(
        cl_build_status status)
{
    switch (status)
    {
        CASE_STR(CL_BUILD_NONE)
        CASE_STR(CL_BUILD_ERROR)
        CASE_STR(CL_BUILD_SUCCESS)
        CASE_STR(CL_BUILD_IN_PROGRESS)

        default: return NULL;
    }
}

