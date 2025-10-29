/**
 * @file
 * @brief Application context interface for SDL window rendering data (CPU renderer).
 */

#include "archi/sdl2/hsp-ctx/window_cpu.var.h"
#include "archi/sdl2/hsp/window_cpu.typ.h"
#include "archipelago/util/struct.typ.h"
#include "archipelago/util/size.fun.h"

static
const archi_struct_dptr_field_info_t dptr_fields[] = {
    ARCHI_STRUCT_DPTR_FIELD_INFO_OPAQUE(window, archi_sdl2_window_cpu_render_data_t),
    ARCHI_STRUCT_DPTR_FIELD_INFO_OPAQUE(texture, archi_sdl2_window_cpu_render_data_t),
    ARCHI_STRUCT_DPTR_FIELD_INFO_OPAQUE(texture_map_data, archi_sdl2_window_cpu_render_data_t),
};

static
const archi_sdl2_window_cpu_render_data_t initializer = {0};

static
const archi_struct_info_t struct_info = {
    .size = sizeof(archi_sdl2_window_cpu_render_data_t),
    .alignment = alignof(archi_sdl2_window_cpu_render_data_t),

    .initializer = &initializer,

    .num_dptr_fields = ARCHI_LENGTH_ARRAY(dptr_fields),

    .dptr_field = dptr_fields,
};

ARCHI_CONTEXT_STRUCTURE_DEFINE_INIT_FUNC(archi_context_sdl2_window_cpu_render_data_init, &struct_info)

const archi_context_interface_t archi_context_sdl2_window_cpu_render_data_interface =
    ARCHI_CONTEXT_STRUCTURE_INTERFACE(archi_context_sdl2_window_cpu_render_data_init);

