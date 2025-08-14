/**
 * @file
 * @brief Application context interface for SDL windows.
 */

#include "archi/sdl2/ctx/window.var.h"
#include "archi/sdl2/api/window.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_sdl2_window_data {
    archi_pointer_t context;

    struct {
        int width;
        int height;
    } texture;

    struct {
        int pitch;
        int x;
        int y;
        int width;
        int height;
    } texture_lock;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_init)
{
    archi_sdl2_window_params_t window_params = {0};
    archi_sdl2_window_params_t window_params_fields = {0};

    bool param_params_set = false;
    bool param_texture_width_set = false;
    bool param_texture_height_set = false;
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

            window_params = *(archi_sdl2_window_params_t*)params->value.ptr;
        }
        else if (strcmp("texture_width", params->name) == 0)
        {
            if (param_texture_width_set)
                continue;
            param_texture_width_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            window_params_fields.texture.width = *(int*)params->value.ptr;
        }
        else if (strcmp("texture_height", params->name) == 0)
        {
            if (param_texture_height_set)
                continue;
            param_texture_height_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            window_params_fields.texture.height = *(int*)params->value.ptr;
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

    if (param_texture_width_set)
        window_params.texture.width = window_params_fields.texture.width;

    if (param_texture_height_set)
        window_params.texture.height = window_params_fields.texture.height;

    if (param_window_width_set)
        window_params.window.width = window_params_fields.window.width;

    if (param_window_height_set)
        window_params.window.height = window_params_fields.window.height;

    if (param_window_flags_set)
        window_params.window.flags = window_params_fields.window.flags;

    if (param_window_title_set)
        window_params.window.title = window_params_fields.window.title;

    struct archi_context_sdl2_window_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    archi_sdl2_window_context_t window = archi_sdl2_window_create(window_params, &code);

    if (window == NULL)
    {
        free(context_data);
        return code;
    }

    *context_data = (struct archi_context_sdl2_window_data){
        .context = {
            .ptr = window,
            .element = {
                .num_of = 1,
            },
        },
        .texture = {
            .width = window_params.texture.width,
            .height = window_params.texture.height,
        },
    };

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_window_final)
{
    struct archi_context_sdl2_window_data *context_data =
        (struct archi_context_sdl2_window_data*)context;

    archi_sdl2_window_destroy(context_data->context.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_sdl2_window_get)
{
    struct archi_context_sdl2_window_data *context_data =
        (struct archi_context_sdl2_window_data*)context;

    if (strcmp("window", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = archi_sdl2_window_get_handle(context_data->context.ptr),
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
            },
        };
    }
    else if (strcmp("renderer", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = archi_sdl2_window_get_renderer(context_data->context.ptr),
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
            },
        };
    }
    else if (strcmp("texture", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = archi_sdl2_window_get_texture(context_data->context.ptr),
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
            },
        };
    }
    else if (strcmp("texture.width", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->texture.width,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->texture.width),
                .alignment = alignof(int),
            },
        };
    }
    else if (strcmp("texture.height", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->texture.height,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->texture.height),
                .alignment = alignof(int),
            },
        };
    }
    else if (strcmp("texture.lock", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_sdl2_pixel_t *pixels = archi_sdl2_window_get_texture_lock(
                context_data->context.ptr, NULL, NULL, NULL,
                &context_data->texture_lock.width, NULL);

        *value = (archi_pointer_t){
            .ptr = pixels,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = context_data->texture_lock.width, // only `width` pixels are contiguous
                .size = sizeof(archi_sdl2_pixel_t),
                .alignment = alignof(archi_sdl2_pixel_t),
            },
        };
    }
    else if (strcmp("texture.lock.pitch", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_sdl2_window_get_texture_lock(
                context_data->context.ptr,
                &context_data->texture_lock.pitch,
                NULL, NULL, NULL, NULL);

        *value = (archi_pointer_t){
            .ptr = &context_data->texture_lock.pitch,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->texture_lock.pitch),
                .alignment = alignof(int),
            },
        };
    }
    else if (strcmp("texture.lock.x", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_sdl2_window_get_texture_lock(
                context_data->context.ptr, NULL,
                &context_data->texture_lock.x,
                NULL, NULL, NULL);

        *value = (archi_pointer_t){
            .ptr = &context_data->texture_lock.x,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->texture_lock.x),
                .alignment = alignof(int),
            },
        };
    }
    else if (strcmp("texture.lock.y", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_sdl2_window_get_texture_lock(
                context_data->context.ptr, NULL, NULL,
                &context_data->texture_lock.y,
                NULL, NULL);

        *value = (archi_pointer_t){
            .ptr = &context_data->texture_lock.y,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->texture_lock.y),
                .alignment = alignof(int),
            },
        };
    }
    else if (strcmp("texture.lock.width", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_sdl2_window_get_texture_lock(
                context_data->context.ptr, NULL, NULL, NULL,
                &context_data->texture_lock.width,
                NULL);

        *value = (archi_pointer_t){
            .ptr = &context_data->texture_lock.width,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->texture_lock.width),
                .alignment = alignof(int),
            },
        };
    }
    else if (strcmp("texture.lock.height", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_sdl2_window_get_texture_lock(
                context_data->context.ptr, NULL, NULL, NULL, NULL,
                &context_data->texture_lock.height);

        *value = (archi_pointer_t){
            .ptr = &context_data->texture_lock.height,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->texture_lock.height),
                .alignment = alignof(int),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_sdl2_window_interface = {
    .init_fn = archi_context_sdl2_window_init,
    .final_fn = archi_context_sdl2_window_final,
    .get_fn = archi_context_sdl2_window_get,
};

