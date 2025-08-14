/**
 * @file
 * @brief OpenCL event lists.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_EVENT_TYP_H_
#define _ARCHI_OPENCL_API_EVENT_TYP_H_

#include <CL/cl.h>

struct archi_opencl_event_ptr_list;

/**
 * @brief Linked list of pointers to events.
 */
typedef struct archi_opencl_event_ptr_list {
    struct archi_opencl_event_ptr_list *next; ///< Pointer to the next list node.
    cl_event *event_ptr; ///< Pointer to an event.
} archi_opencl_event_ptr_list_t;

/**
 * @brief Array of OpenCL events.
 */
typedef struct archi_opencl_event_array {
    const cl_uint num_events; ///< Number of events in the array.
    cl_event event[];   ///< Array of events.
} archi_opencl_event_array_t;

#endif // _ARCHI_OPENCL_API_EVENT_TYP_H_

