# @file
# @brief Context types in the plugin.

import ctypes as c

from archi.base.app import (
        ContextWhitelistable, ParametersWhitelistable,
        )
from archi.base.ctypes.base import archi_pointer_t
from archi.base.ctypes.context import archi_context_interface_t
from archi.builtin.context import LibraryContext


class FontPSFv2Context(ContextWhitelistable):
    """Context type for PSFv2 fonts.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'contents': ...,
                }

    INTERFACE_SYMBOL = 'archi_context_font_psf2_interface'

    CONTEXT_TYPE = 'font.psf_v2'

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class SDL2LibraryContext(ContextWhitelistable):
    """Context type for SDL2 library initialization context.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'timer': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'audio': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'video': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'joystick': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'haptic': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'gamecontroller': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'events': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'sensor': (c.c_byte, lambda v: c.c_byte(bool(v))),
                'everything': (c.c_byte, lambda v: c.c_byte(bool(v))),
                }

    INTERFACE_SYMBOL = 'archi_context_sdl2_library_interface'

    CONTEXT_TYPE = 'sdl2.library'

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class SDL2WindowContext(ContextWhitelistable):
    """Context type for SDL2 windows.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': 'sdl2.window.params',
                'texture_width': (c.c_int, lambda v: c.c_int(v)),
                'texture_height': (c.c_int, lambda v: c.c_int(v)),
                'window_width': (c.c_int, lambda v: c.c_int(v)),
                'window_height': (c.c_int, lambda v: c.c_int(v)),
                'window_flags': (c.c_uint32, lambda v: c.c_uint32(v)),
                'window_title': (str, lambda v: str(v)),
                }

    INTERFACE_SYMBOL = 'archi_context_sdl2_window_interface'

    CONTEXT_TYPE = 'sdl2.window'

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'window': {0: 'sdl2.window.handle'},
            'renderer': {0: 'sdl2.window.renderer'},
            'texture': {0: 'sdl2.window.texture'},
            'texture.width': {0: c.c_int},
            'texture.height': {0: c.c_int},
            'texture.lock': {0: c.c_uint32},
            'texture.lock.pitch': {0: c.c_int},
            'texture.lock.x': {0: c.c_int},
            'texture.lock.y': {0: c.c_int},
            'texture.lock.width': {0: c.c_int},
            'texture.lock.height': {0: c.c_int},
            }

###############################################################################
###############################################################################

CONTEXT_CLASSES = [
        FontPSFv2Context,
        SDL2LibraryContext, SDL2WindowContext,
        ]


class SDLPluginContext(LibraryContext):
    """Library context type for the SDL plugin.
    """
    SYMBOLS = {cls.INTERFACE_SYMBOL: archi_context_interface_t for cls in CONTEXT_CLASSES}

