/**
 * @file
 * @brief Operations on OpenCL event lists.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_EVENT_FUN_H_
#define _ARCHI_OPENCL_API_EVENT_FUN_H_

#include "archi/opencl/api/event.typ.h"

/**
 * @brief Allocate an event array.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Newly allocated event array.
 */
archi_opencl_event_array_t*
archi_opencl_event_array_alloc(
        cl_uint num_events ///< [in] Number of events in the array.
);

/**
 * @brief Release all events in array and reset pointers.
 */
void
archi_opencl_event_array_reset(
        archi_opencl_event_array_t *event_array ///< [in,out] Event array.
);

#endif // _ARCHI_OPENCL_API_EVENT_FUN_H_

