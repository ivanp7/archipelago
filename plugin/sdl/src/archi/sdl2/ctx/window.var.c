/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Context interface for SDL windows.
 */

#include "archi/sdl2/ctx/window.var.h"
#include "archi/sdl2/api/tag.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"

#include "SDL_video.h"

#include <stdint.h> // for uint32_t
#include <stdlib.h> // for malloc(), free()


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__sdl2_window)
{
    // Parse parameters
    const char *title = NULL;
    int width = 0, height = 0, coord_x = 0, coord_y = 0;
    bool center_x = false, center_y = false;
    bool fullscreen = false, hidden = false, borderless = false, resizable = false,
    minimized = false, maximized = false, input_focus = false, mouse_focus = false,
    keyboard_grabbed = false, mouse_grabbed = false, mouse_capture = false,
    always_on_top = false, skip_taskbar = false, utility = false, tooltip = false, popup_menu = false,
    opengl = false;
    uint32_t flags = 0;
    bool width_set = false, height_set = false, coord_x_set = false, coord_y_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "title",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(0, char)}},
                .assign = {archi_plist_assign__dptr_n, &title, sizeof(title), NULL}},
            {.name = "width",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &width, sizeof(width), &width_set}},
            {.name = "height",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &height, sizeof(height), &height_set}},
            {.name = "coord_x",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &coord_x, sizeof(coord_x), &coord_x_set}},
            {.name = "coord_y",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &coord_y, sizeof(coord_y), &coord_y_set}},
            {.name = "center_x",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &center_x, sizeof(center_x), NULL}},
            {.name = "center_y",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &center_y, sizeof(center_y), NULL}},

#define FLAG(var)                                                                                                   \
            {.name = #var,                                                                                          \
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},   \
                .assign = {archi_plist_assign__bool, &var, sizeof(var), NULL}}

            FLAG(fullscreen),
            FLAG(hidden),
            FLAG(borderless),
            FLAG(resizable),
            FLAG(minimized),
            FLAG(maximized),
            FLAG(input_focus),
            FLAG(mouse_focus),
            FLAG(keyboard_grabbed),
            FLAG(mouse_grabbed),
            FLAG(mouse_capture),
            FLAG(always_on_top),
            FLAG(skip_taskbar),
            FLAG(utility),
            FLAG(tooltip),
            FLAG(popup_menu),
            FLAG(opengl),

#undef FLAG

            {.name = "flags",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, uint32_t)}},
                .assign = {archi_plist_assign__value, &flags, sizeof(flags), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    if (!width_set)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "window width is not set");
        return NULL;
    }
    else if (width <= 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "window width is non-positive");
        return NULL;
    }
    else if (!height_set)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "window height is not set");
        return NULL;
    }
    else if (height <= 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "window height is non-positive");
        return NULL;
    }
    else if (coord_x_set && center_x)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "window X coordinate is set and centered simultaneously");
        return NULL;
    }
    else if (coord_y_set && center_y)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "window Y coordinate is set and centered simultaneously");
        return NULL;
    }

    if (title == NULL)
        title = "";

    if (!coord_x_set)
        coord_x = center_x ? SDL_WINDOWPOS_CENTERED : SDL_WINDOWPOS_UNDEFINED;

    if (!coord_y_set)
        coord_y = center_y ? SDL_WINDOWPOS_CENTERED : SDL_WINDOWPOS_UNDEFINED;

#define FLAG(var, value)    \
    if (var) flags |= value

    FLAG(fullscreen, SDL_WINDOW_FULLSCREEN);
    FLAG(hidden, SDL_WINDOW_HIDDEN);
    FLAG(borderless, SDL_WINDOW_BORDERLESS);
    FLAG(resizable, SDL_WINDOW_RESIZABLE);
    FLAG(minimized, SDL_WINDOW_MINIMIZED);
    FLAG(maximized, SDL_WINDOW_MAXIMIZED);
    FLAG(input_focus, SDL_WINDOW_INPUT_FOCUS);
    FLAG(mouse_focus, SDL_WINDOW_MOUSE_FOCUS);
    FLAG(keyboard_grabbed, SDL_WINDOW_KEYBOARD_GRABBED);
    FLAG(mouse_grabbed, SDL_WINDOW_MOUSE_GRABBED);
    FLAG(mouse_capture, SDL_WINDOW_MOUSE_CAPTURE);
    FLAG(always_on_top, SDL_WINDOW_ALWAYS_ON_TOP);
    FLAG(skip_taskbar, SDL_WINDOW_SKIP_TASKBAR);
    FLAG(utility, SDL_WINDOW_UTILITY);
    FLAG(tooltip, SDL_WINDOW_TOOLTIP);
    FLAG(popup_menu, SDL_WINDOW_POPUP_MENU);
    FLAG(opengl, SDL_WINDOW_OPENGL);

#undef FLAG

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = SDL_CreateWindow(title, coord_x, coord_y, width, height, flags),
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__SDL2_WINDOW),
    };

    if (context_data->ptr == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create an SDL window: %s", SDL_GetError());
        return NULL;
    }

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__sdl2_window)
{
    SDL_DestroyWindow(context->ptr);
    free(context);
}

const archi_context_interface_t
archi_context_interface__sdl2_window = {
    .init_fn = archi_context_init__sdl2_window,
    .final_fn = archi_context_final__sdl2_window,
};

