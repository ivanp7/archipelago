/**
 * @file
 * @brief Operations with SDL windows (CPU renderer).
 */

#pragma once
#ifndef _ARCHI_SDL2_API_WINDOW_OPENGL_FUN_H_
#define _ARCHI_SDL2_API_WINDOW_OPENGL_FUN_H_

#include "archi/sdl2/api/window_opengl.typ.h"
#include "archipelago/base/status.typ.h"

struct archi_sdl2_window_opengl_context;
struct SDL_Window;

/**
 * @brief Pointer to SDL window context (OpenGL renderer).
 */
typedef struct archi_sdl2_window_opengl_context *archi_sdl2_window_opengl_context_t;

/**
 * @brief Create an SDL window with an OpenGL renderer.
 *
 * Steps:
 * 1. SDL_CreateWindow()
 * 2. SDL_GL_CreateContext()
 *
 * This function sets the output status code to the number of the failed step.
 *
 * @return Window context.
 */
archi_sdl2_window_opengl_context_t
archi_sdl2_window_opengl_create(
        archi_sdl2_window_opengl_params_t params, ///< [in] Window creation parameters.

        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Destroy an SDL window with an OpenGL renderer.
 *
 * Steps:
 * 1. SDL_GL_DeleteContext()
 * 2. SDL_DestroyWindow()
 */
void
archi_sdl2_window_opengl_destroy(
        archi_sdl2_window_opengl_context_t context ///< [in] Window to destroy.
);

/*****************************************************************************/

/**
 * @brief Get a window handle from a context.
 *
 * @return SDL window handle.
 */
struct SDL_Window*
archi_sdl2_window_opengl_get_window_handle(
        archi_sdl2_window_opengl_context_t context ///< [in] Window context.
);

/**
 * @brief Get an OpenGL context from a window context.
 *
 * @return OpenGL context.
 */
void*
archi_sdl2_window_opengl_get_opengl_context(
        archi_sdl2_window_opengl_context_t context ///< [in] Window context.
);

/**
 * @brief Get a window drawable size from a context.
 */
void
archi_sdl2_window_opengl_get_drawable_size(
        archi_sdl2_window_opengl_context_t context, ///< [in] Window context.

        int *width, ///< [out] Drawable area width in pixels.
        int *height ///< [out] Drawable area height in pixels.
);

#endif // _ARCHI_SDL2_API_WINDOW_OPENGL_FUN_H_

