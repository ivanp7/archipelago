/**
 * @file
 * @brief Types for operations with SDL windows.
 */

#pragma once
#ifndef _PLUGIN_SDL_WINDOW_TYP_H_
#define _PLUGIN_SDL_WINDOW_TYP_H_

#include <stdint.h> // for uint32_t

/**
 * @brief SDL library configuration key for subsystem intitialization flags.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_FLAGS "flags"

/**
 * @brief SDL library configuration key -- whether to initialize the timer subsystem.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_TIMER "timer"

/**
 * @brief SDL library configuration key -- whether to initialize the audio subsystem.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_AUDIO "audio"

/**
 * @brief SDL library configuration key -- whether to initialize the video subsystem.
 *
 * Automatically initializes the events subsystem.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_VIDEO "video"

/**
 * @brief SDL library configuration key -- whether to initialize the joystick subsystem.
 *
 * Automatically initializes the events subsystem.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_JOYSTICK "joystick"

/**
 * @brief SDL library configuration key -- whether to initialize the haptic subsystem.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_HAPTIC "haptic"

/**
 * @brief SDL library configuration key -- whether to initialize the controller subsystem.
 *
 * Automatically initializes the events subsystem.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_GAMECONTROLLER "controller"

/**
 * @brief SDL library configuration key -- whether to initialize the events subsystem.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_EVENTS "events"

/**
 * @brief SDL library configuration key -- whether to initialize all subsystems.
 */
#define PLUGIN_SDL_LIBRARY_CONFIG_KEY_INIT_EVERYTHING "everything"

/*****************************************************************************/

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
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_TEXTURE_WIDTH "texture_width"

/**
 * @brief SDL window configuration key --  height.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_TEXTURE_HEIGHT "texture_height"

/**
 * @brief SDL window configuration key -- window width.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_WIDTH "window_width"

/**
 * @brief SDL window configuration key -- window height.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_HEIGHT "window_height"

/**
 * @brief SDL window configuration key -- window flags.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_FLAGS "window_flags"

/**
 * @brief SDL window configuration key -- window title.
 */
#define PLUGIN_SDL_WINDOW_CONFIG_KEY_WINDOW_TITLE "window_title"

#endif // _PLUGIN_SDL_WINDOW_TYP_H_

