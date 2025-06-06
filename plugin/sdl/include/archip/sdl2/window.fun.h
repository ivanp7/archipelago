/**
 * @file
 * @brief Operations with SDL windows.
 */

#pragma once
#ifndef _ARCHIP_SDL2_WINDOW_FUN_H_
#define _ARCHIP_SDL2_WINDOW_FUN_H_

#include "archip/sdl2/window.typ.h"
#include "archi/util/status.typ.h"

#include <stdbool.h>

struct archip_sdl2_window_context;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

/**
 * @brief Pointer to SDL window context.
 */
typedef struct archip_sdl2_window_context *archip_sdl2_window_context_t;

/**
 * @brief Texture pixel color.
 */
typedef uint32_t archip_sdl2_pixel_t;

/**
 * @brief Create a SDL window with accompanying resources - a renderer and a texture.
 *
 * Steps:
 * 1. SDL_CreateWindow()
 * 2. SDL_CreateRenderer()
 * 3. SDL_CreateTexture()
 *
 * This function sets the output status code to the number of the failed step.
 *
 * @return Window context.
 */
archip_sdl2_window_context_t
archip_sdl2_window_create(
        archip_sdl2_window_params_t params, ///< [in] Window creation parameters.

        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Destroy a SDL window.
 *
 * Steps:
 * 1. SDL_DestroyTexture()
 * 2. SDL_DestroyRenderer()
 * 3. SDL_DestroyWindow()
 */
void
archip_sdl2_window_destroy(
        archip_sdl2_window_context_t context ///< [in] Window to destroy.
);

/**
 * @brief Lock a whole window texture for update.
 *
 * @return Status code (0 on success, 1 if the SDL call failed).
 */
archi_status_t
archip_sdl2_window_lock_whole_texture(
        archip_sdl2_window_context_t context ///< [in] Window context.
);

/**
 * @brief Lock a rectangle in window texture for update.
 *
 * @return Status code (0 on success, 1 if the SDL call failed).
 */
archi_status_t
archip_sdl2_window_lock_texture_area(
        archip_sdl2_window_context_t context, ///< [in] Window context.

        int x,     ///< [in] X coordinate of rectangle to lock.
        int y,     ///< [in] Y coordinate of rectangle to lock.
        int width, ///< [in] Width of rectangle to lock.
        int height ///< [in] Height of rectangle to lock.
);

/**
 * @brief Unlock a window texture and render the updated rectangle.
 *
 * @return Status code (0 on success, 1 if the SDL call failed).
 */
archi_status_t
archip_sdl2_window_unlock_texture_and_render(
        archip_sdl2_window_context_t context ///< [in] Window context.
);

/*****************************************************************************/

/**
 * @brief Draw a glyph on a window texture.
 *
 * Glyph rows must always begin at byte border.
 *
 * @return True if texture has been modified, otherwise false.
 */
bool
archip_sdl2_window_texture_draw_glyph(
        archip_sdl2_window_context_t context, ///< [in] Window context.

        int x, ///< [in] X coordinate of glyph's upper left corner.
        int y, ///< [in] Y coordinate of glyph's upper left corner.

        const unsigned char *glyph, ///< [in] Glyph to draw.
        int glyph_width,  ///< [in] Glyph width in pixels.
        int glyph_height, ///< [in] Glyph height in pixels.

        int glyph_col_idx,  ///< [in] Index of the first glyph column to draw.
        int glyph_row_idx,  ///< [in] Index of the first glyph row to draw.
        int glyph_num_cols, ///< [in] Number of glyph columns to draw.
        int glyph_num_rows, ///< [in] Number of glyph rows to draw.

        archip_sdl2_pixel_t fg, ///< [in] Foreground color of glyph.
        archip_sdl2_pixel_t bg, ///< [in] Background color of glyph.

        bool draw_fg, ///< [in] Whether to draw foreground pixels.
        bool draw_bg  ///< [in] Whether to draw background pixels.
);

/*****************************************************************************/

/**
 * @brief Get a window handle from a context.
 *
 * @return SDL window handle.
 */
struct SDL_Window*
archip_sdl2_window_get_handle(
        archip_sdl2_window_context_t context ///< [in] Window context.
);

/**
 * @brief Get a renderer handle from a context.
 *
 * @return SDL renderer handle.
 */
struct SDL_Renderer*
archip_sdl2_window_get_renderer(
        archip_sdl2_window_context_t context ///< [in] Window context.
);

/**
 * @brief Get a texture handle from a context.
 *
 * @return SDL texture handle.
 */
struct SDL_Texture*
archip_sdl2_window_get_texture(
        archip_sdl2_window_context_t context ///< [in] Window context.
);

/**
 * @brief Get a texture size from a context.
 */
void
archip_sdl2_window_get_texture_size(
        archip_sdl2_window_context_t context, ///< [in] Window context.

        int *width, ///< [out] Texture width in pixels.
        int *height ///< [out] Texture height in pixels.
);

/**
 * @brief Get a texture lock from a context.
 *
 * @return Pointer to locked pixels of the texture.
 */
archip_sdl2_pixel_t*
archip_sdl2_window_get_texture_lock(
        archip_sdl2_window_context_t context, ///< [in] Window context.

        int *pitch, ///< [out] Size of a full texture row in pixels.

        int *x,     ///< [out] X coordinate of the locked rectangle.
        int *y,     ///< [out] Y coordinate of the locked rectangle.
        int *width, ///< [out] Width of the locked rectangle.
        int *height ///< [out] Height of the locked rectangle.
);

#endif // _ARCHIP_SDL2_WINDOW_FUN_H_

