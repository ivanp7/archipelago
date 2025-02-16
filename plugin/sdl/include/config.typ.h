/**
 * @file
 * @brief Configuration types of the plugin.
 */

#pragma once
#ifndef _PLUGIN_SDL_CONFIG_TYP_H_
#define _PLUGIN_SDL_CONFIG_TYP_H_

#include <stdint.h> // for uint32_t

/**
 * @brief Properties of a created SDL window.
 *
 * If window width is 0, it is substituted with texture width.
 * If window height is 0, it is substituted with texture height.
 */
typedef struct plugin_sdl_window_config {
    struct {
        int width;  ///< Texture width in pixels.
        int height; ///< Texture height in pixels.
    } texture;

    struct {
        int width;  ///< Window width in pixels.
        int height; ///< Window height in pixels.

        uint32_t flags; ///< Window flags.

        const char *title; ///< Window title.
    } window;
} plugin_sdl_window_config_t;

/**
 * @brief SDL window configuration key for the whole configuration structure.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY "config"

/**
 * @brief SDL window configuration key -- texture width.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_TEXTURE_WIDTH "texture.width"

/**
 * @brief SDL window configuration key --  height.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_TEXTURE_HEIGHT "texture.height"

/**
 * @brief SDL window configuration key -- window width.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_WIDTH "window.width"

/**
 * @brief SDL window configuration key -- window height.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_HEIGHT "window.height"

/**
 * @brief SDL window configuration key -- window flags.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_FLAGS "window.flags"

/**
 * @brief SDL window configuration key -- window title.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_TITLE "window.title"

#endif // _PLUGIN_SDL_CONFIG_TYP_H_

