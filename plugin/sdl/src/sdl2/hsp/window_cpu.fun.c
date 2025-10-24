/**
 * @file
 * @brief Hierarchical state processor states for operations with SDL2 windows (CPU renderer).
 */

#include "archi/sdl2/hsp/window_cpu.fun.h"
#include "archi/sdl2/hsp/window_cpu.typ.h"
#include "archi/sdl2/api/window_cpu.fun.h"
#include "archi/hsp/api/state.fun.h"
#include "archi/memory/api/interface.fun.h"
#include "archipelago/log/print.fun.h"

#include <string.h> // for memcpy()

ARCHI_HSP_STATE_FUNCTION(archi_sdl2_hsp_state_window_cpu_render)
{
    (void) hsp;

    archi_sdl2_window_cpu_render_data_t *render_data = data;
    if ((render_data == NULL) || (render_data->window == NULL) || (render_data->texture == NULL))
        return;

    archi_status_t code;

    // Lock window texture
    code = archi_sdl2_window_cpu_lock_whole_texture(render_data->window);

    if (code != 0)
    {
        archi_log_error(__func__, "archi_sdl2_window_cpu_lock_whole_texture() -> %i", code);
        return;
    }

    // Get window texture lock
    archi_sdl2_pixel_t *texture_lock = archi_sdl2_window_cpu_get_texture_lock(
            render_data->window, NULL, NULL, NULL, NULL, NULL);

    // Map texture data
    archi_pointer_t texture_mapping = archi_memory_map(render_data->texture,
            render_data->texture_map_data, 0, 0, false, &code);
    if (texture_mapping.ptr == NULL)
    {
        archi_sdl2_window_cpu_unlock_texture_and_render(render_data->window);

        archi_log_error(__func__, "archi_memory_map() -> %i", code);
        return;
    }

    // Copy texture data to texture lock
    memcpy(texture_lock, texture_mapping.ptr,
            texture_mapping.element.num_of * texture_mapping.element.size);

    // Unmap texture data
    archi_memory_unmap(render_data->texture);

    // Unlock window texture and render
    code = archi_sdl2_window_cpu_unlock_texture_and_render(render_data->window);

    if (code != 0)
        archi_log_error(__func__, "archi_sdl2_window_cpu_unlock_texture_and_render() -> %i", code);
}

