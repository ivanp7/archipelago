/**
 * @file
 * @brief Application context interface for SDL windows (OpenGL renderer).
 */

#include "archi/sdl2/ctx/window_opengl.var.h"
#include "archi/sdl2/api/window_opengl.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_opengl_init)
{
    archi_sdl2_window_opengl_params_t window_params = {0};
    archi_sdl2_window_opengl_params_t window_params_fields = {0};

    bool param_params_set = false;
    bool param_window_width_set = false;
    bool param_window_height_set = false;
    bool param_window_flags_set = false;
    bool param_window_title_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("params", params->name) == 0)
        {
            if (param_params_set)
                continue;
            param_params_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            window_params = *(archi_sdl2_window_opengl_params_t*)params->value.ptr;
        }
        else if (strcmp("window_width", params->name) == 0)
        {
            if (param_window_width_set)
                continue;
            param_window_width_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            window_params_fields.window.width = *(int*)params->value.ptr;
        }
        else if (strcmp("window_height", params->name) == 0)
        {
            if (param_window_height_set)
                continue;
            param_window_height_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            window_params_fields.window.height = *(int*)params->value.ptr;
        }
        else if (strcmp("window_flags", params->name) == 0)
        {
            if (param_window_flags_set)
                continue;
            param_window_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            window_params_fields.window.flags = *(uint32_t*)params->value.ptr;
        }
        else if (strcmp("window_title", params->name) == 0)
        {
            if (param_window_title_set)
                continue;
            param_window_title_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            window_params_fields.window.title = params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (param_window_width_set)
        window_params.window.width = window_params_fields.window.width;

    if (param_window_height_set)
        window_params.window.height = window_params_fields.window.height;

    if (param_window_flags_set)
        window_params.window.flags = window_params_fields.window.flags;

    if (param_window_title_set)
        window_params.window.title = window_params_fields.window.title;

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    archi_sdl2_window_opengl_context_t window =
        archi_sdl2_window_opengl_create(window_params, &code);

    if (window == NULL)
    {
        free(context_data);
        return code;
    }

    *context_data = (archi_pointer_t){
        .ptr = window,
        .element = {
            .num_of = 1,
        },
    };

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_window_opengl_final)
{
    archi_sdl2_window_opengl_destroy(context->ptr);
    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_sdl2_window_opengl_get)
{
    if (strcmp("window", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = archi_sdl2_window_opengl_get_window_handle(context->ptr),
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
            },
        };
    }
    else if (strcmp("opengl", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = archi_sdl2_window_opengl_get_opengl_context(context->ptr),
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_sdl2_window_opengl_interface = {
    .init_fn = archi_context_sdl2_window_opengl_init,
    .final_fn = archi_context_sdl2_window_opengl_final,
    .get_fn = archi_context_sdl2_window_opengl_get,
};

