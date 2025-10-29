/**
 * @file
 * @brief Application context interface for SDL library initialization.
 */

#pragma once
#ifndef _ARCHI_SDL2_CTX_LIBRARY_VAR_H_
#define _ARCHI_SDL2_CTX_LIBRARY_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief SDL library context initialization function.
 *
 * Accepts the following parameters:
 * - "timer"            : initialize timer subsystem
 * - "audio"            : initialize audio subsystem
 * - "video"            : initialize video subsystem
 * - "joystick"         : initialize joystick subsystem
 * - "haptic"           : initialize haptic subsystem
 * - "gamecontroller"   : initialize gamecontroller subsystem
 * - "events"           : initialize events subsystem
 * - "sensor"           : initialize sensor subsystem
 * - "everything"       : initialize all subsystems
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_sdl2_library_init);

/**
 * @brief SDL library context finalization function.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_sdl2_library_final);

/**
 * @brief SDL library context interface.
 */
extern
const archi_context_interface_t archi_context_sdl2_library_interface;

#endif // _ARCHI_SDL2_CTX_LIBRARY_VAR_H_

