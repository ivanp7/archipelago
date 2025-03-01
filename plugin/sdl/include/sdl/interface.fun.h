/**
 * @file
 * @brief Context interfaces of the plugin.
 */

#pragma once
#ifndef _PLUGIN_SDL_INTERFACE_FUN_H_
#define _PLUGIN_SDL_INTERFACE_FUN_H_

#include "archi/app/context.typ.h"

ARCHI_CONTEXT_INIT_FUNC(plugin_sdl_library_init);   ///< SDL library initialization function.
ARCHI_CONTEXT_FINAL_FUNC(plugin_sdl_library_final); ///< SDL library finalization function.

extern
const archi_context_interface_t plugin_sdl_library_interface; ///< SDL library interface functions.

/**
 * @brief Key of SDL library interface.
 */
#define PLUGIN_SDL_LIBRARY_INTERFACE_KEY "sdl"

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(plugin_sdl_window_context_init);   ///< SDL window context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(plugin_sdl_window_context_final); ///< SDL window context finalization function.

extern
const archi_context_interface_t plugin_sdl_window_context_interface; ///< SDL window context interface functions.

/**
 * @brief Key of SDL window context interface.
 */
#define PLUGIN_SDL_WINDOW_INTERFACE "sdl_window"

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(plugin_font_psf2_context_init);   ///< Font context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(plugin_font_psf2_context_final); ///< Font context finalization function.

extern
const archi_context_interface_t plugin_font_psf2_context_interface; ///< Font context interface functions.

/**
 * @brief Key of font context interface.
 */
#define PLUGIN_FONT_PSF2_INTERFACE "font_psf2"

/**
 * @brief Font configuration key for data buffer.
 */
#define PLUGIN_FONT_PSF2_CONFIG_KEY_BYTES "bytes"

#endif // _PLUGIN_SDL_INTERFACE_FUN_H_

