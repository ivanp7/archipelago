/**
 * @file
 * @brief OpenCL event lists.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_EVENT_TYP_H_
#define _ARCHIP_OPENCL_EVENT_TYP_H_

#include <CL/cl.h>

struct archip_opencl_event_ptr_list;

/**
 * @brief Linked list of pointers to events.
 */
typedef struct archip_opencl_event_ptr_list {
    struct archip_opencl_event_ptr_list *next; ///< Pointer to the next list node.
    cl_event *event_ptr; ///< Pointer to an event.
} archip_opencl_event_ptr_list_t;

/**
 * @brief Array of OpenCL events.
 */
typedef struct archip_opencl_event_array {
    const cl_uint num_events; ///< Number of events in the array.
    cl_event event[];   ///< Array of events.
} archip_opencl_event_array_t;

#endif // _ARCHIP_OPENCL_EVENT_TYP_H_

