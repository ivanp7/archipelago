#!/usr/bin/env python

import os

#------------------------------------------------------------------------------

PROJECT_NAME = "archipelago"

LIB_NAME = f"lib{PROJECT_NAME}.a"
EXEC_NAME = "archi"

#------------------------------------------------------------------------------

INCLUDE_DIR = "include"
SOURCE_DIR  = "src"
BUILD_DIR   = "build"

EXEC_SOURCE = "main.c"

#------------------------------------------------------------------------------

CFLAGS = ['-march=native', '-pipe', '-std=c17',
          '-Wall', '-Wextra', '-Wpedantic',
          '-Wmissing-prototypes', '-Wstrict-prototypes', '-Wold-style-definition',
          '-pthread']
LFLAGS = ['-ldl', '-pthread']

if 'DEBUG' in os.environ:                       ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<
    CFLAGS += ['-O0', '-g3', '-ggdb']
else:
    CFLAGS += ['-O2', '-g0', '-flto', '-ffat-lto-objects']
    LFLAGS += ['-flto']

if 'PROFILE' in os.environ:                     ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<
    CFLAGS += ['-pg']

#------------------------------------------------------------------------------

CFLAGS += [f'-I{INCLUDE_DIR}']

if 'COLORLESS' in os.environ:                   ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<
    CFLAGS += ['-DARCHI_FEATURE_COLORLESS']

if 'QUEUE32' in os.environ:                     ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<
    CFLAGS += ['-DARCHI_FEATURE_QUEUE32']

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

os.chdir(os.path.dirname(__file__))

SOURCES_LIBRARY = []
for entry in os.walk(SOURCE_DIR):
    if (os.path.samefile(entry[0], SOURCE_DIR)):
        entry[2].remove(EXEC_SOURCE)

    SOURCES_LIBRARY += [f'{entry[0]}/{s}' for s in entry[2] if s[-2:] == '.c']

CFLAGS = list(filter(None, CFLAGS))
LFLAGS = list(filter(None, LFLAGS))

build_ninja = open('build.ninja', 'w')

build_ninja.write(f'''\
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

rule link_exe
    command = $LINKER_EXE $LINKER_EXE_FLAGS -o $out $in $opts
    description = link(executable): $out

{'\n'.join([f'build {BUILD_DIR}/{src[:-1]}o: compile {src}' for src in SOURCES_LIBRARY])}
build {BUILD_DIR}/{LIB_NAME}: link_static {
        ' '.join([f'{BUILD_DIR}/{src[:-1]}o' for src in SOURCES_LIBRARY])}

build {BUILD_DIR}/{SOURCE_DIR}/{EXEC_SOURCE[:-1]}o: compile {SOURCE_DIR}/{EXEC_SOURCE}
build {BUILD_DIR}/{EXEC_NAME}: link_exe \
{BUILD_DIR}/{SOURCE_DIR}/{EXEC_SOURCE[:-1]}o \
{BUILD_DIR}/{LIB_NAME}

build lib: phony {BUILD_DIR}/{LIB_NAME}
build app: phony {BUILD_DIR}/{EXEC_NAME}
build all: phony lib app
default all

''')

build_ninja.close()

