 #############################################################################
 # Copyright (C) 2023-2026 by Ivan Podmazov                                  #
 #                                                                           #
 # This file is part of Archipelago.                                         #
 #                                                                           #
 #   Archipelago is free software: you can redistribute it and/or modify it  #
 #   under the terms of the GNU Lesser General Public License as published   #
 #   by the Free Software Foundation, either version 3 of the License, or    #
 #   (at your option) any later version.                                     #
 #                                                                           #
 #   Archipelago is distributed in the hope that it will be useful,          #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of          #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
 #   GNU Lesser General Public License for more details.                     #
 #                                                                           #
 #   You should have received a copy of the GNU Lesser General Public        #
 #   License along with Archipelago. If not, see                             #
 #   <http://www.gnu.org/licenses/>.                                         #
 #############################################################################

# @file
# @brief Context types of the SDL plugin.

import ctypes as c

import archi.ctypes as actype
from .object import PrimitiveData, String
from .context import TypeAttributes, ContextWhitelist, ParametersWhitelist


PLUGIN_SDL = 'archi_sdl'

##############################################################################

_TYPE_DATA = TypeAttributes.complex_data()
_TYPE_BOOL = (TypeAttributes.from_type(c.c_char), lambda value: PrimitiveData(c.c_char(bool(value))))
_TYPE_INT = (TypeAttributes.from_type(c.c_int), lambda value: PrimitiveData(c.c_int(value)))
_TYPE_UINT32 = (TypeAttributes.from_type(c.c_uint32), lambda value: PrimitiveData(c.c_uint32(value)))
_TYPE_STRING = (TypeAttributes.from_type(c.c_char * 1), lambda value: String(value))

##############################################################################

### archi/font_psf2 ###

class FontPSFv2Context(ContextWhitelist):
    """PSFv2 font.
    """
    C_NAME = 'font_psf2'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__FONT_PSF2)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'data': _TYPE_DATA}

### archi/sdl2 ###

class SDL2LibraryContext(ContextWhitelist):
    """SDL2 library initialization.
    """
    C_NAME = 'sdl2_library'

    CONTEXT_TYPE = None

    class InitParameters(ParametersWhitelist):
        PARAMS = {'everything': _TYPE_BOOL,
                  'timer': _TYPE_BOOL,
                  'audio': _TYPE_BOOL,
                  'video': _TYPE_BOOL,
                  'joystick': _TYPE_BOOL,
                  'haptic': _TYPE_BOOL,
                  'gamecontroller': _TYPE_BOOL,
                  'events': _TYPE_BOOL,
                  'sensor': _TYPE_BOOL}


class SDL2WindowContext(ContextWhitelist):
    """SDL2 window.
    """
    C_NAME = 'sdl2_window'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__SDL2_WINDOW)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'title': _TYPE_STRING,
                  'width': _TYPE_INT,
                  'height': _TYPE_INT,
                  'coord_x': _TYPE_INT,
                  'coord_y': _TYPE_INT,
                  'center_x': _TYPE_BOOL,
                  'center_y': _TYPE_BOOL,
                  'fullscreen': _TYPE_BOOL,
                  'hidden': _TYPE_BOOL,
                  'borderless': _TYPE_BOOL,
                  'resizable': _TYPE_BOOL,
                  'minimized': _TYPE_BOOL,
                  'maximized': _TYPE_BOOL,
                  'input_focus': _TYPE_BOOL,
                  'mouse_focus': _TYPE_BOOL,
                  'keyboard_grabbed': _TYPE_BOOL,
                  'mouse_grabbed': _TYPE_BOOL,
                  'mouse_capture': _TYPE_BOOL,
                  'always_on_top': _TYPE_BOOL,
                  'skip_taskbar': _TYPE_BOOL,
                  'utility': _TYPE_BOOL,
                  'tooltip': _TYPE_BOOL,
                  'popup_menu': _TYPE_BOOL,
                  'opengl': _TYPE_BOOL,
                  'flags': _TYPE_UINT32}


class SDL2RendererContext(ContextWhitelist):
    """SDL2 renderer.
    """
    C_NAME = 'sdl2_renderer'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__SDL2_RENDERER)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'window': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__SDL2_WINDOW),
                  'driver_index': _TYPE_INT,
                  'software': _TYPE_BOOL,
                  'accelerated': _TYPE_BOOL,
                  'present_vsync': _TYPE_BOOL,
                  'target_texture': _TYPE_BOOL,
                  'flags': _TYPE_UINT32}

    GETTER_SLOTS = {'window': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__SDL2_WINDOW)}


class SDL2TextureContext(ContextWhitelist):
    """SDL2 texture.
    """
    C_NAME = 'sdl2_texture'

    CONTEXT_TYPE = TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__SDL2_TEXTURE)

    class InitParameters(ParametersWhitelist):
        PARAMS = {'renderer': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__SDL2_RENDERER),
                  'width': _TYPE_INT,
                  'height': _TYPE_INT,
                  'pixel_format': _TYPE_STRING,
                  'static': _TYPE_BOOL,
                  'streaming': _TYPE_BOOL,
                  'target': _TYPE_BOOL}

    GETTER_SLOTS = {'renderer': TypeAttributes.complex_data(actype.ARCHI_POINTER_DATA_TAG__SDL2_RENDERER)}

