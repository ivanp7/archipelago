#!/usr/bin/env python

import os
import pkgconfig

#------------------------------------------------------------------------------

PROJECT_NAME = "plugin_sdl"

SLIB_NAME = f"lib{PROJECT_NAME}.a"
DLIB_NAME = f"lib{PROJECT_NAME}.so"

#------------------------------------------------------------------------------

INCLUDE_DIR = "include"
SOURCE_DIR  = "src"
BUILD_DIR   = "build"

ARCHI_INC   = os.environ.get('ARCHI_INC', "../../include")
ARCHI_LIB   = os.environ.get('ARCHI_LIB', "../../build/libarchipelago.a")

#------------------------------------------------------------------------------

CFLAGS = ['-march=native', '-pipe', '-std=c17',
          '-Wall', '-Wextra', '-Wpedantic',
          '-Wmissing-prototypes', '-Wstrict-prototypes', '-Wold-style-definition']
LFLAGS = ['-shared', '-rdynamic']

if 'DEBUG' in os.environ:                       ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<
    CFLAGS += ['-O0', '-g3', '-ggdb']
else:
    CFLAGS += ['-O2', '-g0', '-flto', '-ffat-lto-objects']
    LFLAGS += ['-flto']

if 'PROFILE' in os.environ:                     ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<
    CFLAGS += ['-pg']

#------------------------------------------------------------------------------

CFLAGS += [f'-I{INCLUDE_DIR}']

CFLAGS += [f'-I{ARCHI_INC}']

CFLAGS += pkgconfig.cflags('sdl2').split(' ')
LFLAGS += pkgconfig.libs('sdl2').split(' ')

#------------------------------------------------------------------------------

if 'LLVM' in os.environ:                        ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<
    CC = 'clang'
    CC_FLAGS = ['-fcolor-diagnostics']

    LINKER_STATIC = 'llvm-ar'
    LINKER_STATIC_FLAGS = ['rcs']

    LINKER_EXE = 'clang'
    LINKER_EXE_FLAGS = ['-fuse-ld=lld']
else:
    CC = 'gcc'
    CC_FLAGS = ['-fdiagnostics-color=always']

    LINKER_STATIC = 'gcc-ar'
    LINKER_STATIC_FLAGS = ['rcs']

    LINKER_EXE = 'gcc'
    LINKER_EXE_FLAGS = []

#------------------------------------------------------------------------------

CFLAGS = list(filter(None, CFLAGS))
LFLAGS = list(filter(None, LFLAGS))

os.chdir(os.path.dirname(__file__))

SOURCES_LIBRARY = []
OBJECTS_LIBRARY = []
for entry in os.walk(f'{SOURCE_DIR}'):
    src = [f'{entry[0]}/{s}' for s in entry[2] if s[-2:] == '.c']
    SOURCES_LIBRARY += src
    OBJECTS_LIBRARY += [f'{BUILD_DIR}/{s[:-1]}o' for s in src] # replace .c extension with .o

#------------------------------------------------------------------------------

build_ninja_separator = '\n###############################################################################\n'

build_ninja_segments = []
TARGETS = []

build_ninja_segments.append(f'''\
CC = {CC}
CC_FLAGS = {' '.join(CC_FLAGS + CFLAGS)}

rule compile
    command = $CC $CC_FLAGS $opts -MMD -MT $out -MF $out.d -c $in -o $out
    description = compile: $out
    depfile = $out.d
    deps = gcc


LINKER_STATIC = {LINKER_STATIC}
LINKER_STATIC_FLAGS = {' '.join(LINKER_STATIC_FLAGS)}

rule link_static
    command = rm -f $out && $LINKER_STATIC $LINKER_STATIC_FLAGS $opts $out $in
    description = link(static): $out


LINKER_EXE = {LINKER_EXE}
LINKER_EXE_FLAGS = {' '.join(LINKER_EXE_FLAGS + LFLAGS)}

rule link_shared
    command = $LINKER_EXE $LINKER_EXE_FLAGS -o $out $in $opts
    description = link(shared): $out
''')

build_ninja_segments.append(f'''\
{'\n'.join([f'build {obj}: compile {src}' for obj, src in zip(OBJECTS_LIBRARY, SOURCES_LIBRARY)])}

build {BUILD_DIR}/{SLIB_NAME}: link_static {' '.join(OBJECTS_LIBRARY)}
build lib_static: phony {BUILD_DIR}/{SLIB_NAME}
''')
TARGETS.append('lib_static')

build_ninja_segments.append(f'''\
build {BUILD_DIR}/{DLIB_NAME}: link_shared {' '.join(OBJECTS_LIBRARY)} {ARCHI_LIB}
build lib_shared: phony {BUILD_DIR}/{DLIB_NAME}
''')
TARGETS.append('lib_shared')

build_ninja_segments.append(f'''\
build all: phony {' '.join(TARGETS)}
default all
''')

with open('build.ninja', 'w') as file:
    file.write(f'{build_ninja_separator}\n'.join(build_ninja_segments) + '\n')

