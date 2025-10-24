/**
 * @file
 * @brief Application context interface for SDL window rendering data (CPU renderer).
 */

#include "archi/sdl2/ctx_hsp/window_cpu.var.h"
#include "archi/sdl2/hsp/window_cpu.typ.h"
#include "archipelago/base/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_sdl2_window_cpu_render_data_data {
    archi_pointer_t render_data;

    // References
    archi_pointer_t window;
    archi_pointer_t texture;
    archi_pointer_t texture_map_data;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_window_cpu_render_data_init)
{
    archi_pointer_t render_data_window = {0};
    archi_pointer_t render_data_texture = {0};
    archi_pointer_t render_data_texture_map_data = {0};

    bool param_window_set = false;
    bool param_texture_set = false;
    bool param_texture_map_data_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("window", params->name) == 0)
        {
            if (param_window_set)
                continue;
            param_window_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            render_data_window = params->value;
        }
        else if (strcmp("texture", params->name) == 0)
        {
            if (param_texture_set)
                continue;
            param_texture_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            render_data_texture = params->value;
        }
        else if (strcmp("texture_map_data", params->name) == 0)
        {
            if (param_texture_map_data_set)
                continue;
            param_texture_map_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            render_data_texture_map_data = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_sdl2_window_cpu_render_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_sdl2_window_cpu_render_data_t *render_data = malloc(sizeof(*render_data));
    if (render_data == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *render_data = (archi_sdl2_window_cpu_render_data_t){
        .window = render_data_window.ptr,
        .texture = render_data_texture.ptr,
        .texture_map_data = render_data_texture_map_data.ptr,
    };

    *context_data = (struct archi_context_sdl2_window_cpu_render_data_data){
        .render_data = {
            .ptr = render_data,
            .element = {
                .num_of = 1,
                .size = sizeof(*render_data),
                .alignment = alignof(archi_sdl2_window_cpu_render_data_t),
            },
        },
        .window = render_data_window,
        .texture = render_data_texture,
        .texture_map_data = render_data_texture_map_data,
    };

    archi_reference_count_increment(render_data_window.ref_count);
    archi_reference_count_increment(render_data_texture.ref_count);
    archi_reference_count_increment(render_data_texture_map_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_window_cpu_render_data_final)
{
    struct archi_context_sdl2_window_cpu_render_data_data *context_data =
        (struct archi_context_sdl2_window_cpu_render_data_data*)context;

    archi_reference_count_decrement(context_data->window.ref_count);
    archi_reference_count_decrement(context_data->texture.ref_count);
    archi_reference_count_decrement(context_data->texture_map_data.ref_count);

    free(context_data->render_data.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_sdl2_window_cpu_render_data_get)
{
    struct archi_context_sdl2_window_cpu_render_data_data *context_data =
        (struct archi_context_sdl2_window_cpu_render_data_data*)context;

    if (strcmp("window", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->window;
    }
    else if (strcmp("texture", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->texture;
    }
    else if (strcmp("texture_map_data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->texture_map_data;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_sdl2_window_cpu_render_data_set)
{
    struct archi_context_sdl2_window_cpu_render_data_data *context_data =
        (struct archi_context_sdl2_window_cpu_render_data_data*)context;

    archi_sdl2_window_cpu_render_data_t *render_data = context_data->render_data.ptr;

    if (strcmp("window", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->window.ref_count);

        render_data->window = value.ptr;
        context_data->window = value;
    }
    else if (strcmp("texture", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->texture.ref_count);

        render_data->texture = value.ptr;
        context_data->texture = value;
    }
    else if (strcmp("texture_map_data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->texture_map_data.ref_count);

        render_data->texture_map_data = value.ptr;
        context_data->texture_map_data = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_sdl2_window_cpu_render_data_interface = {
    .init_fn = archi_context_sdl2_window_cpu_render_data_init,
    .final_fn = archi_context_sdl2_window_cpu_render_data_final,
    .get_fn = archi_context_sdl2_window_cpu_render_data_get,
    .set_fn = archi_context_sdl2_window_cpu_render_data_set,
};

