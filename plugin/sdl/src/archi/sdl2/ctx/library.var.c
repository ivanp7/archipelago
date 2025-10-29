/**
 * @file
 * @brief Application context interface for SDL library initialization.
 */

#include "archi/sdl2/ctx/library.var.h"
#include "archipelago/util/string.fun.h"

#include "SDL.h"

#include <stdbool.h>

static
archi_pointer_t archi_context_sdl2_library_context_data;

ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_library_init)
{
    if (archi_context_sdl2_library_context_data.length != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "SDL2 library is initialized already");
        return NULL;
    }

    // Parse parameters
    bool timer = false;
    bool audio = false;
    bool video = false;
    bool joystick = false;
    bool haptic = false;
    bool gamecontroller = false;
    bool events = false;
    bool sensor = false;
    bool everything = false;

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
        if (ARCHI_STRING_COMPARE("timer", ==, params->key))
        {
            if (param_timer_set)
                continue;
            param_timer_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'timer' parameter attributes are incorrect");
                return NULL;
            }

            timer = *(char*)params->value.ptr != 0;
        }
        else if (ARCHI_STRING_COMPARE("audio", ==, params->key))
        {
            if (param_audio_set)
                continue;
            param_audio_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'audio' parameter attributes are incorrect");
                return NULL;
            }

            audio = *(char*)params->value.ptr != 0;
        }
        else if (ARCHI_STRING_COMPARE("video", ==, params->key))
        {
            if (param_video_set)
                continue;
            param_video_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'video' parameter attributes are incorrect");
                return NULL;
            }

            video = *(char*)params->value.ptr != 0;
        }
        else if (ARCHI_STRING_COMPARE("joystick", ==, params->key))
        {
            if (param_joystick_set)
                continue;
            param_joystick_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'joystick' parameter attributes are incorrect");
                return NULL;
            }

            joystick = *(char*)params->value.ptr != 0;
        }
        else if (ARCHI_STRING_COMPARE("haptic", ==, params->key))
        {
            if (param_haptic_set)
                continue;
            param_haptic_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'haptic' parameter attributes are incorrect");
                return NULL;
            }

            haptic = *(char*)params->value.ptr != 0;
        }
        else if (ARCHI_STRING_COMPARE("gamecontroller", ==, params->key))
        {
            if (param_gamecontroller_set)
                continue;
            param_gamecontroller_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'gamecontroller' parameter attributes are incorrect");
                return NULL;
            }

            gamecontroller = *(char*)params->value.ptr != 0;
        }
        else if (ARCHI_STRING_COMPARE("events", ==, params->key))
        {
            if (param_events_set)
                continue;
            param_events_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'events' parameter attributes are incorrect");
                return NULL;
            }

            events = *(char*)params->value.ptr != 0;
        }
        else if (ARCHI_STRING_COMPARE("sensor", ==, params->key))
        {
            if (param_sensor_set)
                continue;
            param_sensor_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'sensor' parameter attributes are incorrect");
                return NULL;
            }

            sensor = *(char*)params->value.ptr != 0;
        }
        else if (ARCHI_STRING_COMPARE("everything", ==, params->key))
        {
            if (param_everything_set)
                continue;
            param_everything_set = true;

            if (!ARCHI_POINTER_TO_DATA_TYPE(params->value, 1, char))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "'everything' parameter attributes are incorrect");
                return NULL;
            }

            everything = *(char*)params->value.ptr != 0;
        }
        else
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown parameter encountered");
            return NULL;
        }
    }

    Uint32 flags = everything ? SDL_INIT_EVERYTHING : 0;

#define SUBSYSTEM(var, flag)    \
    if (var)                    \
        flags |= flag;          \
    else                        \
        flags &= ~flag

    SUBSYSTEM(timer, SDL_INIT_TIMER);
    SUBSYSTEM(audio, SDL_INIT_AUDIO);
    SUBSYSTEM(video, SDL_INIT_VIDEO);
    SUBSYSTEM(joystick, SDL_INIT_JOYSTICK);
    SUBSYSTEM(haptic, SDL_INIT_HAPTIC);
    SUBSYSTEM(gamecontroller, SDL_INIT_GAMECONTROLLER);
    SUBSYSTEM(events, SDL_INIT_EVENTS);
    SUBSYSTEM(sensor, SDL_INIT_SENSOR);

#undef SUBSYSTEM

    // Construct the context
    int ret = SDL_Init(flags);
    if (ret < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't initialize SDL2 library (error code %i)", ret);
        return NULL;
    }

    archi_context_sdl2_library_context_data = (archi_pointer_t){
        .attr = ARCHI_POINTER_DATA_ATTRIBUTES(false, 0, flags),
        .length = 1,
    };

    ARCHI_ERROR_RESET();
    return &archi_context_sdl2_library_context_data;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_library_final)
{
    SDL_Quit();
    archi_context_sdl2_library_context_data = (archi_pointer_t){0};
}

const archi_context_interface_t archi_context_sdl2_library_interface = {
    .init_fn = archi_context_sdl2_library_init,
    .final_fn = archi_context_sdl2_library_final,
};

