/**
 * @file
 * @brief Operations on OpenCL work description.
 */

#include "archi/opencl/api/work.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc()

archi_opencl_work_vector_t*
archi_opencl_work_vector_alloc(
        cl_uint num_dimensions)
{
    archi_opencl_work_vector_t *work_vector = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_opencl_work_vector_t, dimension, num_dimensions));
    if (work_vector == NULL)
        return NULL;

    cl_uint *num_dimensions_ptr = (cl_uint*)&work_vector->num_dimensions;
    *num_dimensions_ptr = num_dimensions;

    for (cl_uint i = 0; i < num_dimensions; i++)
        work_vector->dimension[i] = 0;

    return work_vector;
}

