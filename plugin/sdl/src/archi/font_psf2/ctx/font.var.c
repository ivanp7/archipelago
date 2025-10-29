/**
 * @file
 * @brief Application context interface for PSFv2 font.
 */

#include "archi/font_psf2/ctx/font.var.h"
#include "archi/font_psf2/api/font.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

ARCHI_CONTEXT_INIT_FUNC(archi_context_font_psf2_init)
{
    // Parse parameters
    archi_pointer_t data = {0};

    bool param_data_set = false;

    for (; params != NULL; params = params->next)
    {
        if (ARCHI_STRING_COMPARE("data", ==, params->key))
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'data' parameter attributes are incorrect");
                return NULL;
            }

            data = params->value;
        }
        else
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown parameter encountered");
            return NULL;
        }
    }

    // Construct the context
    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_font_psf2_t font = archi_font_psf2_load(data, ARCHI_ERROR_PARAMETER);
    if (font == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_pointer_t){
        .ptr = font,
        .attr = ARCHI_POINTER_ATTRIBUTES(ARCHI_POINTER_TYPE__READONLY_MEM, 0, 0),
        .length = 1,
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_font_psf2_final)
{
    archi_font_psf2_unload(context->ptr);
    free(context);
}

const archi_context_interface_t archi_context_font_psf2_interface = {
    .init_fn = archi_context_font_psf2_init,
    .final_fn = archi_context_font_psf2_final,
};

