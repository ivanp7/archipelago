/**
 * @file
 * @brief Operations with SDL windows.
 */

#include "sdl/window.fun.h"
#include "archi/util/error.def.h"

#include "SDL.h"
#include "SDL_video.h"
#include "SDL_render.h"

#include <stdlib.h> // for malloc(), free()

struct plugin_sdl_window_context {
    struct {
        SDL_Window *handle; ///< SDL window handle.
    } window;

    struct {
        SDL_Renderer *handle; ///< SDL renderer handle.
    } renderer;

    struct {
        SDL_Texture *handle; ///< SDL texture handle.

        int width;  ///< SDL texture width in pixels.
        int height; ///< SDL texture height in pixels.

        struct {
            plugin_sdl_pixel_t *pixels; ///< Pointer to locked pixels of the texture.
            int pitch;          ///< Size of a full texture row in pixels.

            SDL_Rect rectangle; ///< Locked rectangle.
        } lock;
    } texture;
};

struct plugin_sdl_window_context*
plugin_sdl_window_create(
        plugin_sdl_window_config_t config,

        archi_status_t *code)
{
    if ((config.texture.width <= 0) || (config.texture.height <= 0) ||
            (config.window.width < 0) || (config.window.height < 0))
    {
        if (code != NULL)
            *code = ARCHI_ERROR_MISUSE;
        return NULL;
    }

    // Allocate a context structure object
    struct plugin_sdl_window_context *context = malloc(sizeof(*context));
    if (context == NULL)
    {
        if (code != NULL)
            *code = ARCHI_ERROR_ALLOC;
        return NULL;
    }

    *context = (struct plugin_sdl_window_context){0};

    // Step 1: create window
    {
        const char *title = config.window.title;
        if (title == NULL)
            title = "";

        int width = config.window.width;
        if (width == 0)
            width = config.texture.width;

        int height = config.window.height;
        if (height == 0)
            height = config.texture.height;

        context->window.handle = SDL_CreateWindow(title,
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                width, height, config.window.flags);
    }

    if (context->window.handle == NULL)
    {
        if (code != NULL)
            *code = 1;
        goto failure;
    }

    // Step 2: create renderer
    context->renderer.handle = SDL_CreateRenderer(context->window.handle,
            -1, SDL_RENDERER_ACCELERATED);

    if (context->renderer.handle == NULL)
        context->renderer.handle = SDL_CreateRenderer(context->window.handle,
                -1, SDL_RENDERER_SOFTWARE);

    if (context->renderer.handle == NULL)
    {
        if (code != NULL)
            *code = 2;
        goto failure;
    }

    // Step 3: create texture
    context->texture.handle = SDL_CreateTexture(context->renderer.handle,
            SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
            config.texture.width, config.texture.height);

    if (context->texture.handle == NULL)
    {
        if (code != NULL)
            *code = 3;
        goto failure;
    }

    context->texture.width = config.texture.width;
    context->texture.height = config.texture.height;

    if (code != NULL)
        *code = 0;

    return context;

failure:
    plugin_sdl_window_destroy(context);

    return NULL;
}

void
plugin_sdl_window_destroy(
        struct plugin_sdl_window_context *context)
{
    if (context == NULL)
        return;

    if (context->texture.handle != NULL)
        SDL_DestroyTexture(context->texture.handle);

    if (context->renderer.handle != NULL)
        SDL_DestroyRenderer(context->renderer.handle);

    if (context->window.handle != NULL)
        SDL_DestroyWindow(context->window.handle);

    free(context);
}

static
bool
plugin_sdl_window_lock_texture(
        SDL_Texture *texture,
        const SDL_Rect *rectangle,
        plugin_sdl_pixel_t **pixels,
        int *pitch)
{
    return SDL_LockTexture(texture, rectangle, (void**)pixels, pitch) == 0;
}

archi_status_t
plugin_sdl_window_lock_whole_texture(
        struct plugin_sdl_window_context *context)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (context->texture.lock.pixels != NULL)
        return ARCHI_ERROR_MISUSE;

    if (!plugin_sdl_window_lock_texture(context->texture.handle, NULL,
                &context->texture.lock.pixels, &context->texture.lock.pitch))
        return 1;

    context->texture.lock.pitch /= sizeof(plugin_sdl_pixel_t);
    context->texture.lock.rectangle = (SDL_Rect){
        .w = context->texture.width,
        .h = context->texture.height,
    };

    return 0;
}

archi_status_t
plugin_sdl_window_lock_texture_area(
        struct plugin_sdl_window_context *context,

        int x,
        int y,
        int width,
        int height)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (context->texture.lock.pixels != NULL)
        return ARCHI_ERROR_MISUSE;

    SDL_Rect rectangle = {.x = x, .y = y, .w = width, .h = height};

    if (!plugin_sdl_window_lock_texture(context->texture.handle, &rectangle,
                &context->texture.lock.pixels, &context->texture.lock.pitch))
        return 1;

    context->texture.lock.pitch /= sizeof(plugin_sdl_pixel_t);
    context->texture.lock.rectangle = rectangle;

    return 0;
}

archi_status_t
plugin_sdl_window_unlock_texture_and_render(
        struct plugin_sdl_window_context *context)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (context->texture.lock.pixels == NULL)
        return ARCHI_ERROR_MISUSE;

    SDL_UnlockTexture(context->texture.handle);

    context->texture.lock.pixels = NULL;
    context->texture.lock.pitch = 0;
    context->texture.lock.rectangle = (SDL_Rect){0};

    if (SDL_RenderCopy(context->renderer.handle, context->texture.handle, NULL, NULL) < 0)
        return 1;

    SDL_RenderPresent(context->renderer.handle);

    return 0;
}

/*****************************************************************************/

bool
plugin_sdl_window_texture_draw_glyph(
        struct plugin_sdl_window_context *context,

        int x,
        int y,

        const unsigned char *glyph,
        int glyph_width,
        int glyph_height,

        int glyph_col_idx,
        int glyph_row_idx,
        int glyph_num_cols,
        int glyph_num_rows,

        plugin_sdl_pixel_t fg,
        plugin_sdl_pixel_t bg,

        bool draw_fg,
        bool draw_bg)
{
    if ((context == NULL) || (context->texture.lock.pixels == NULL))
        return false;
    else if (!draw_fg && !draw_bg)
        return false;
    else if ((glyph == NULL) || (glyph_width <= 0) || (glyph_height <= 0))
        return false;
    else if ((glyph_num_cols == 0) || (glyph_num_rows == 0))
        return false;

    // Copy locked texture rectangle info
    plugin_sdl_pixel_t *pixels = context->texture.lock.pixels;
    int pitch = context->texture.lock.pitch;

    SDL_Rect rect = context->texture.lock.rectangle;

    // Prepare auxiliary values
    int bytes_per_row = (glyph_width + 7) / 8;

    int col_idx_delta = glyph_num_cols > 0 ? 1 : -1;
    int row_idx_delta = glyph_num_rows > 0 ? 1 : -1;

    // Draw all chosen rows
    for (int row_idx = glyph_row_idx, i = 0; row_idx != glyph_row_idx + glyph_num_rows;
            row_idx += row_idx_delta, i++)
    {
        if ((y + i < 0) || (y + i < rect.y) || (y + i >= rect.y + rect.h))
            continue; // skip out-of-bounds row

        int texture_row_displ = pitch * (y + i - rect.y);

        const unsigned char *row;
        if ((row_idx >= 0) && (row_idx < glyph_height))
            row = glyph + bytes_per_row * row_idx;
        else
            row = NULL;

        int byte_idx = bytes_per_row;
        unsigned char byte = 0;

        // Draw all chosen columns
        for (int col_idx = glyph_col_idx, j = 0; col_idx != glyph_col_idx + glyph_num_cols;
                col_idx += col_idx_delta, j++)
        {
            if ((x + j < 0) || (x + j < rect.x) || (x + j >= rect.x + rect.w))
                continue; // skip out-of-bounds column

            int texture_idx = texture_row_displ + (x + j - rect.x);

            bool pixel_is_fg = false;
            if ((row != NULL) && (col_idx >= 0) && (col_idx < glyph_width))
            {
                int current_byte_idx = col_idx / 8;
                if (current_byte_idx != byte_idx)
                {
                    byte_idx = current_byte_idx;
                    byte = row[byte_idx];
                }

                pixel_is_fg = byte & (1 << (7 - (col_idx % 8)));
            }

            if (draw_fg && pixel_is_fg)
                pixels[texture_idx] = fg;
            else if (draw_bg && !pixel_is_fg)
                pixels[texture_idx] = bg;
        }
    }

    return true;
}

/*****************************************************************************/

SDL_Window*
plugin_sdl_window_get_handle(
        struct plugin_sdl_window_context *context)
{
    if (context == NULL)
        return NULL;

    return context->window.handle;
}

SDL_Renderer*
plugin_sdl_window_get_renderer(
        struct plugin_sdl_window_context *context)
{
    if (context == NULL)
        return NULL;

    return context->renderer.handle;
}

SDL_Texture*
plugin_sdl_window_get_texture(
        struct plugin_sdl_window_context *context)
{
    if (context == NULL)
        return NULL;

    return context->texture.handle;
}

void
plugin_sdl_window_get_texture_size(
        struct plugin_sdl_window_context *context,

        int *width,
        int *height)
{
    if (context == NULL)
        return;

    if (width != NULL)
        *width = context->texture.width;

    if (height != NULL)
        *height = context->texture.height;
}

plugin_sdl_pixel_t*
plugin_sdl_window_get_texture_lock(
        struct plugin_sdl_window_context *context,

        int *pitch,

        int *x,
        int *y,
        int *width,
        int *height)
{
    if (context == NULL)
        return NULL;

    if (pitch != NULL)
        *pitch = context->texture.lock.pitch;

    if (x != NULL)
        *x = context->texture.lock.rectangle.x;

    if (y != NULL)
        *y = context->texture.lock.rectangle.y;

    if (width != NULL)
        *width = context->texture.lock.rectangle.w;

    if (height != NULL)
        *height = context->texture.lock.rectangle.h;

    return context->texture.lock.pixels;
}

