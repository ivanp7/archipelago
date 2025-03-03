/**
 * @file
 * @brief Operations with SDL windows.
 */

#include "sdl/interface.fun.h"
#include "sdl/window.fun.h"
#include "sdl/font.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <string.h> // for strcmp()

#include "SDL.h"

static
ARCHI_LIST_ACT_FUNC(plugin_sdl_library_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    uint32_t *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_FLAGS) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(*config)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        *config = *(uint32_t*)value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_TIMER) == 0)
    {
        if ((value.type != ARCHI_VALUE_FALSE) && (value.type != ARCHI_VALUE_TRUE))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_TRUE)
            *config |= SDL_INIT_TIMER;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_AUDIO) == 0)
    {
        if ((value.type != ARCHI_VALUE_FALSE) && (value.type != ARCHI_VALUE_TRUE))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_TRUE)
            *config |= SDL_INIT_AUDIO;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_VIDEO) == 0)
    {
        if ((value.type != ARCHI_VALUE_FALSE) && (value.type != ARCHI_VALUE_TRUE))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_TRUE)
            *config |= SDL_INIT_VIDEO;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_JOYSTICK) == 0)
    {
        if ((value.type != ARCHI_VALUE_FALSE) && (value.type != ARCHI_VALUE_TRUE))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_TRUE)
            *config |= SDL_INIT_JOYSTICK;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_HAPTIC) == 0)
    {
        if ((value.type != ARCHI_VALUE_FALSE) && (value.type != ARCHI_VALUE_TRUE))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_TRUE)
            *config |= SDL_INIT_HAPTIC;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_GAMECONTROLLER) == 0)
    {
        if ((value.type != ARCHI_VALUE_FALSE) && (value.type != ARCHI_VALUE_TRUE))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_TRUE)
            *config |= SDL_INIT_GAMECONTROLLER;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_EVENTS) == 0)
    {
        if ((value.type != ARCHI_VALUE_FALSE) && (value.type != ARCHI_VALUE_TRUE))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_TRUE)
            *config |= SDL_INIT_EVENTS;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_EVERYTHING) == 0)
    {
        if ((value.type != ARCHI_VALUE_FALSE) && (value.type != ARCHI_VALUE_TRUE))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_TRUE)
            *config |= SDL_INIT_EVERYTHING;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(plugin_sdl_library_init)
{
    (void) context;
    (void) metadata;

    archi_status_t code;

    uint32_t flags = 0;
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                plugin_sdl_library_init_config, &flags, true, 0, NULL);
        if (code != 0)
            return code;
    }

    code = SDL_Init(flags);
    if (code < 0)
        return ARCHI_ERROR_OPERATION;

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(plugin_sdl_library_final)
{
    (void) context;
    (void) metadata;

    SDL_Quit();
}

const archi_context_interface_t plugin_sdl_library_interface = {
    .init_fn = plugin_sdl_library_init,
    .final_fn = plugin_sdl_library_final,
};

/*****************************************************************************/

static
ARCHI_LIST_ACT_FUNC(plugin_sdl_window_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    plugin_sdl_window_config_t *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, PLUGIN_SDL_WINDOW_CONFIG_KEY) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size != sizeof(*config)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_WINDOW_CONFIG_KEY_TEXTURE_WIDTH) == 0)
    {
        if ((value.type != ARCHI_VALUE_SINT) || (value.ptr == NULL) ||
                (value.size != sizeof(config->texture.width)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->texture.width = *(int*)value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_WINDOW_CONFIG_KEY_TEXTURE_HEIGHT) == 0)
    {
        if ((value.type != ARCHI_VALUE_SINT) || (value.ptr == NULL) ||
                (value.size != sizeof(config->texture.height)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->texture.height = *(int*)value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_WIDTH) == 0)
    {
        if (((value.type != ARCHI_VALUE_SINT) && (value.type != ARCHI_VALUE_FLOAT)) ||
                (value.ptr == NULL) || (value.size != sizeof(config->window.width)) ||
                (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_SINT)
            config->window.width = *(int*)value.ptr;
        else
            config->window.width = *(float*)value.ptr * config->texture.width;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_HEIGHT) == 0)
    {
        if (((value.type != ARCHI_VALUE_SINT) && (value.type != ARCHI_VALUE_FLOAT)) ||
                (value.ptr == NULL) || (value.size != sizeof(config->window.height)) ||
                (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        if (value.type == ARCHI_VALUE_SINT)
            config->window.height = *(int*)value.ptr;
        else
            config->window.height = *(float*)value.ptr * config->texture.height;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_FLAGS) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(config->window.flags)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->window.flags = *(uint32_t*)value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_TITLE) == 0)
    {
        if ((value.type != ARCHI_VALUE_STRING) || (value.ptr == NULL) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->window.title = value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(plugin_sdl_window_context_init)
{
    (void) metadata;

    archi_status_t code;

    plugin_sdl_window_config_t window_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                plugin_sdl_window_context_init_config, &window_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    struct plugin_sdl_window_context *window_context = plugin_sdl_window_create(window_config, &code);
    if (code != 0)
        return code;

    *context = window_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(plugin_sdl_window_context_final)
{
    (void) metadata;

    plugin_sdl_window_destroy(context);
}

const archi_context_interface_t plugin_sdl_window_context_interface = {
    .init_fn = plugin_sdl_window_context_init,
    .final_fn = plugin_sdl_window_context_final,
};

/*****************************************************************************/

static
ARCHI_LIST_ACT_FUNC(plugin_font_psf2_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    archi_value_t *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, PLUGIN_FONT_PSF2_CONFIG_KEY_BYTES) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size == 0) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        *config = value;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(plugin_font_psf2_context_init)
{
    (void) metadata;

    archi_status_t code;

    archi_value_t font_bytes = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                plugin_font_psf2_context_init_config, &font_bytes, true, 0, NULL);
        if (code != 0)
            return code;
    }

    plugin_font_psf2_t *font_context =
        plugin_font_psf2_load_from_bytes(font_bytes.ptr, font_bytes.size, &code);
    if (code != 0)
        return code;

    *context = font_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(plugin_font_psf2_context_final)
{
    (void) metadata;

    plugin_font_psf2_unload(context);
}

const archi_context_interface_t plugin_font_psf2_context_interface = {
    .init_fn = plugin_font_psf2_context_init,
    .final_fn = plugin_font_psf2_context_final,
};

