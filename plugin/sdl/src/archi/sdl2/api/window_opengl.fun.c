/**
 * @file
 * @brief Operations with SDL windows (OpenGL renderer).
 */

#include "archi/sdl2/api/window_opengl.fun.h"

#include "SDL.h"
#include "SDL_video.h"

#include <stdlib.h> // for malloc(), free()

struct archi_sdl2_window_opengl_context {
    struct {
        SDL_Window *handle; ///< SDL window handle.
    } window;

    struct {
        SDL_GLContext *context; ///< OpenGL context.
    } opengl;
};

archi_sdl2_window_opengl_context_t
archi_sdl2_window_opengl_create(
        archi_sdl2_window_opengl_params_t params,

        archi_status_t *code)
{
    if ((params.window.width <= 0) || (params.window.height <= 0))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;
        return NULL;
    }

    // Allocate a context structure object
    archi_sdl2_window_opengl_context_t context = malloc(sizeof(*context));
    if (context == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;
        return NULL;
    }

    *context = (struct archi_sdl2_window_opengl_context){0};

    // Step 1: create window
    {
        const char *title = params.window.title;
        if (title == NULL)
            title = "";

        context->window.handle = SDL_CreateWindow(title,
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                params.window.width, params.window.height,
                params.window.flags | SDL_WINDOW_OPENGL);
    }

    if (context->window.handle == NULL)
    {
        if (code != NULL)
            *code = 1;
        goto failure;
    }

    // Step 2: create OpenGL context
    context->opengl.context = SDL_GL_CreateContext(context->window.handle);

    if (context->opengl.context == NULL)
    {
        if (code != NULL)
            *code = 2;
        goto failure;
    }

    if (code != NULL)
        *code = 0;

    return context;

failure:
    archi_sdl2_window_opengl_destroy(context);

    return NULL;
}

void
archi_sdl2_window_opengl_destroy(
        archi_sdl2_window_opengl_context_t context)
{
    if (context == NULL)
        return;

    if (context->opengl.context != NULL)
        SDL_GL_DeleteContext(context->opengl.context);

    if (context->window.handle != NULL)
        SDL_DestroyWindow(context->window.handle);

    free(context);
}

SDL_Window*
archi_sdl2_window_opengl_get_window_handle(
        archi_sdl2_window_opengl_context_t context)
{
    if (context == NULL)
        return NULL;

    return context->window.handle;
}

void*
archi_sdl2_window_opengl_get_opengl_context(
        archi_sdl2_window_opengl_context_t context)
{
    if (context == NULL)
        return NULL;

    return context->opengl.context;
}

void
archi_sdl2_window_opengl_get_drawable_size(
        archi_sdl2_window_opengl_context_t context,

        int *width,
        int *height)
{
    if (context == NULL)
        return;

    SDL_GL_GetDrawableSize(context->window.handle, width, height);
}

