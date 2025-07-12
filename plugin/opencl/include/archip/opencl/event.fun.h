/**
 * @file
 * @brief Operations on OpenCL event lists.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_EVENT_FUN_H_
#define _ARCHIP_OPENCL_EVENT_FUN_H_

#include "archip/opencl/event.typ.h"

/**
 * @brief Allocate an event array.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Newly allocated event array.
 */
archip_opencl_event_array_t*
archip_opencl_event_array_alloc(
        cl_uint num_events ///< [in] Number of events in the array.
);

/**
 * @brief Release all events in array and reset pointers.
 */
void
archip_opencl_event_array_reset(
        archip_opencl_event_array_t *event_array ///< [in,out] Event array.
);

#endif // _ARCHIP_OPENCL_EVENT_FUN_H_

