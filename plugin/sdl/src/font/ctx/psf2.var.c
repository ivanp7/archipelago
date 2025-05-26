/**
 * @file
 * @brief Application context interface for PSFv2 font.
 */

#include "archip/font/ctx/psf2.var.h"
#include "archip/font/psf2.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()

ARCHI_CONTEXT_INIT_FUNC(archip_context_font_psf2_init)
{
    archi_pointer_t contents = {0};

    bool param_contents_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("contents", params->name) == 0)
        {
            if (param_contents_set)
                continue;
            param_contents_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            contents = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    archip_font_psf2_t font = archip_font_psf2_load(contents.ptr, contents.element.num_of, &code);

    if (font == NULL)
    {
        free(context_data);
        return code;
    }

    *context_data = (archi_pointer_t){
        .ptr = font,
        .ref_count = contents.ref_count,
        .element = {
            .num_of = 1,
        },
    };

    archi_reference_count_increment(contents.ref_count);

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archip_context_font_psf2_final)
{
    archip_font_psf2_unload(context->ptr);
    archi_reference_count_decrement(context->ref_count);
    free(context);
}

const archi_context_interface_t archip_context_font_psf2_interface = {
    .init_fn = archip_context_font_psf2_init,
    .final_fn = archip_context_font_psf2_final,
};

