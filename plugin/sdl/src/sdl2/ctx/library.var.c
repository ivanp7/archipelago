/**
 * @file
 * @brief Application context interface for SDL library initialization.
 */

#include "archip/sdl2/ctx/library.var.h"

#include "SDL.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()

ARCHI_CONTEXT_INIT_FUNC(archip_context_sdl2_library_init)
{
    Uint32 flags = 0;

    bool param_timer_set = false;
    bool param_audio_set = false;
    bool param_video_set = false;
    bool param_joystick_set = false;
    bool param_haptic_set = false;
    bool param_gamecontroller_set = false;
    bool param_events_set = false;
    bool param_sensor_set = false;
    bool param_everything_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("timer", params->name) == 0)
        {
            if (param_timer_set)
                continue;
            param_timer_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_TIMER;
            else
                flags &= ~SDL_INIT_TIMER;
        }
        else if (strcmp("audio", params->name) == 0)
        {
            if (param_audio_set)
                continue;
            param_audio_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_AUDIO;
            else
                flags &= ~SDL_INIT_AUDIO;
        }
        else if (strcmp("video", params->name) == 0)
        {
            if (param_video_set)
                continue;
            param_video_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_VIDEO;
            else
                flags &= ~SDL_INIT_VIDEO;
        }
        else if (strcmp("joystick", params->name) == 0)
        {
            if (param_joystick_set)
                continue;
            param_joystick_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_JOYSTICK;
            else
                flags &= ~SDL_INIT_JOYSTICK;
        }
        else if (strcmp("haptic", params->name) == 0)
        {
            if (param_haptic_set)
                continue;
            param_haptic_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_HAPTIC;
            else
                flags &= ~SDL_INIT_HAPTIC;
        }
        else if (strcmp("gamecontroller", params->name) == 0)
        {
            if (param_gamecontroller_set)
                continue;
            param_gamecontroller_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_GAMECONTROLLER;
            else
                flags &= ~SDL_INIT_GAMECONTROLLER;
        }
        else if (strcmp("events", params->name) == 0)
        {
            if (param_events_set)
                continue;
            param_events_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_EVENTS;
            else
                flags &= ~SDL_INIT_EVENTS;
        }
        else if (strcmp("sensor", params->name) == 0)
        {
            if (param_sensor_set)
                continue;
            param_sensor_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_SENSOR;
            else
                flags &= ~SDL_INIT_SENSOR;
        }
        else if (strcmp("everything", params->name) == 0)
        {
            if (param_everything_set)
                continue;
            param_everything_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (*(char*)params->value.ptr)
                flags |= SDL_INIT_EVERYTHING;
            else
                flags &= ~SDL_INIT_EVERYTHING;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code = SDL_Init(flags);
    if (code < 0)
    {
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    *context_data = (archi_pointer_t){0};

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archip_context_sdl2_library_final)
{
    SDL_Quit();
    free(context);
}

const archi_context_interface_t archip_context_sdl2_library_interface = {
    .init_fn = archip_context_sdl2_library_init,
    .final_fn = archip_context_sdl2_library_final,
};

