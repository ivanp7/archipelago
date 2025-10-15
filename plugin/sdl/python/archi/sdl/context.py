# @file
# @brief Context types in the plugin.

import ctypes as c

from archi.base.app import (
        PrivateType, PublicType,
        ContextWhitelistable, ParametersWhitelistable,
        )
from archi.base.ctypes.base import archi_pointer_t
from archi.base.ctypes.context import archi_context_interface_t
from archi.builtin.context import LibraryContext

###############################################################################

_TYPE_BOOL = PublicType(c.c_byte, constr=lambda v: c.c_byte(bool(v)))
_TYPE_INT = PublicType(c.c_int)
_TYPE_UINT32 = PublicType(c.c_uint32)
_TYPE_STR = PublicType(c.c_char, array=True, constr=lambda v: c.create_string_buffer(v.encode()))

_TYPE_MEMORY = PrivateType('archi.memory')

_TYPE_FONT_PSF_V2 = PrivateType('font.psf_v2')

_TYPE_SDL2_LIBRARY = PrivateType('sdl2.library')
_TYPE_SDL2_WINDOW_HANDLE = PrivateType('sdl2.window.handle')
_TYPE_SDL2_RENDERER_HANDLE = PrivateType('sdl2.renderer.handle')
_TYPE_SDL2_TEXTURE_HANDLE = PrivateType('sdl2.texture.handle')
_TYPE_SDL2_WINDOW_CPU = PrivateType('sdl2.window_cpu')
_TYPE_SDL2_WINDOW_CPU_PARAMS = PrivateType('sdl2.window_cpu.params')
_TYPE_SDL2_WINDOW_CPU_RENDER_DATA = PrivateType('sdl2.window_cpu.render_data')
_TYPE_SDL2_WINDOW_OPENGL = PrivateType('sdl2.window_opengl')
_TYPE_SDL2_WINDOW_OPENGL_PARAMS = PrivateType('sdl2.window_opengl.params')

_TYPE_OPENGL_CONTEXT = PrivateType('opengl.context')

_TYPE_CONTEXT_INTERFACE = PublicType(archi_context_interface_t)

###############################################################################

class FontPSFv2Context(ContextWhitelistable):
    """Context type for PSFv2 fonts.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'contents': None,
                }

    INTERFACE_SYMBOL = 'archi_context_font_psf2_interface'

    DATA_TYPE = _TYPE_FONT_PSF_V2

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class SDL2LibraryContext(ContextWhitelistable):
    """Context type for SDL2 library initialization context.
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'timer': _TYPE_BOOL,
                'audio': _TYPE_BOOL,
                'video': _TYPE_BOOL,
                'joystick': _TYPE_BOOL,
                'haptic': _TYPE_BOOL,
                'gamecontroller': _TYPE_BOOL,
                'events': _TYPE_BOOL,
                'sensor': _TYPE_BOOL,
                'everything': _TYPE_BOOL,
                }

    INTERFACE_SYMBOL = 'archi_context_sdl2_library_interface'

    DATA_TYPE = _TYPE_SDL2_LIBRARY

    INIT_PARAMETERS_CLASS = InitParameters

###############################################################################

class SDL2WindowCPUContext(ContextWhitelistable):
    """Context type for SDL2 windows (CPU renderer).
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': _TYPE_SDL2_WINDOW_CPU_PARAMS,
                'texture_width': _TYPE_INT,
                'texture_height': _TYPE_INT,
                'window_width': _TYPE_INT,
                'window_height': _TYPE_INT,
                'window_flags': _TYPE_UINT32,
                'window_title': _TYPE_STR,
                }

    INTERFACE_SYMBOL = 'archi_context_sdl2_window_cpu_interface'

    DATA_TYPE = _TYPE_SDL2_WINDOW_CPU

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'window': {0: _TYPE_SDL2_WINDOW_HANDLE},
            'renderer': {0: _TYPE_SDL2_RENDERER_HANDLE},
            'texture': {0: _TYPE_SDL2_TEXTURE_HANDLE},
            'texture.width': {0: _TYPE_INT},
            'texture.height': {0: _TYPE_INT},
            'texture.lock': {0: _TYPE_UINT32},
            'texture.lock.pitch': {0: _TYPE_INT},
            'texture.lock.x': {0: _TYPE_INT},
            'texture.lock.y': {0: _TYPE_INT},
            'texture.lock.width': {0: _TYPE_INT},
            'texture.lock.height': {0: _TYPE_INT},
            }


class SDL2WindowCPURenderDataContext(ContextWhitelistable):
    """Context type for SDL2 window renderer data (CPU renderer).
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'window': _TYPE_SDL2_WINDOW_CPU,
                'texture': _TYPE_MEMORY,
                'texture_map_data': None,
                }

    INTERFACE_SYMBOL = 'archi_context_sdl2_window_cpu_render_data_interface'

    DATA_TYPE = _TYPE_SDL2_WINDOW_CPU_RENDER_DATA

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'window': {0: _TYPE_SDL2_WINDOW_CPU},
            'texture': {0: _TYPE_MEMORY},
            'texture_map_data': {0: None},
            }

    SETTER_SLOT_TYPES = {
            'window': {0: _TYPE_SDL2_WINDOW_CPU},
            'texture': {0: _TYPE_MEMORY},
            'texture_map_data': {0: None},
            }

###############################################################################

class SDL2WindowOpenGLContext(ContextWhitelistable):
    """Context type for SDL2 windows (OpenGL renderer).
    """
    class InitParameters(ParametersWhitelistable):
        PARAMETERS = {
                'params': _TYPE_SDL2_WINDOW_OPENGL_PARAMS,
                'window_width': _TYPE_INT,
                'window_height': _TYPE_INT,
                'window_flags': _TYPE_UINT32,
                'window_title': _TYPE_STR,
                }

    INTERFACE_SYMBOL = 'archi_context_sdl2_window_opengl_interface'

    DATA_TYPE = _TYPE_SDL2_WINDOW_OPENGL

    INIT_PARAMETERS_CLASS = InitParameters

    GETTER_SLOT_TYPES = {
            'window': {0: _TYPE_SDL2_WINDOW_OPENGL},
            'opengl': {0: _TYPE_OPENGL_CONTEXT},
            }

###############################################################################
###############################################################################

CONTEXT_CLASSES = [
        FontPSFv2Context,
        SDL2LibraryContext,
        SDL2WindowCPUContext, SDL2WindowCPURenderDataContext,
        SDL2WindowOpenGLContext,
        ]


class SDLPluginContext(LibraryContext):
    """Library context type for the SDL plugin.
    """
    SYMBOLS = {cls.INTERFACE_SYMBOL: _TYPE_CONTEXT_INTERFACE for cls in CONTEXT_CLASSES}

