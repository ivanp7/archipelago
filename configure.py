#!/usr/bin/env python

import os

#------------------------------------------------------------------------------

PROJECT_NAME = "archipelago"
PROJECT_NAME_SHORT = "archi"

LIB_NAME = f"lib{PROJECT_NAME}.a"
EXEC_NAME = f"{PROJECT_NAME_SHORT}"
TESTS_NAME = f"{EXEC_NAME}-tests"

#------------------------------------------------------------------------------

INCLUDE_DIR = "include"
SOURCE_DIR  = "src"
TEST_DIR    = "test"
BUILD_DIR   = "build"

LIB_SOURCE_DIR = "lib"
EXEC_SOURCE = "main.c"

#------------------------------------------------------------------------------

CFLAGS = ['-march=native', '-pipe', '-std=c17',
          '-Wall', '-Wextra', '-Wpedantic',
          '-Wmissing-prototypes', '-Wstrict-prototypes', '-Wold-style-definition',
          '-pthread', '-fPIC',
          '-fstack-protector-strong']
LFLAGS = ['-ldl', '-pthread', '-fPIC']

if 'DEBUG' in os.environ:                       ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<
    CFLAGS += ['-O0', '-g3', '-ggdb']
else:
    CFLAGS += ['-O2', '-g0', '-flto', '-ffat-lto-objects', '-U_FORTIFY_SOURCE', '-D_FORTIFY_SOURCE=2']
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

BUILD_TESTS = 'NO_TESTS' not in os.environ      ### <<<<<<<<<<<<<<<<<<<< INPUT ENVIRONMENT VARIABLE <<<<<<<<<<<<<<<<<<<<

#------------------------------------------------------------------------------

CFLAGS = list(filter(None, CFLAGS))
LFLAGS = list(filter(None, LFLAGS))

os.chdir(os.path.dirname(__file__))

EXEC_OBJECT = f"{BUILD_DIR}/{SOURCE_DIR}/{EXEC_SOURCE[:-1]}o"

SOURCES_LIBRARY = []
OBJECTS_LIBRARY = []
for entry in os.walk(f'{SOURCE_DIR}/{LIB_SOURCE_DIR}'):
    src = [f'{entry[0]}/{s}' for s in entry[2] if s[-2:] == '.c']
    SOURCES_LIBRARY += src
    OBJECTS_LIBRARY += [f'{BUILD_DIR}/{s[:-1]}o' for s in src] # replace .c extension with .o

if BUILD_TESTS:
    TEST_HEADER = f'{BUILD_DIR}/{TEST_DIR}/test.h'
    TEST_SOURCE = f'{TEST_HEADER[:-1]}c'
    TEST_OBJECT = f'{TEST_HEADER[:-1]}o'

    SOURCES_TESTS = []
    OBJECTS_TESTS = []
    for entry in os.walk(TEST_DIR):
        src = [f'{entry[0]}/{s}' for s in entry[2] if s[-2:] == '.c']
        SOURCES_TESTS += src
        OBJECTS_TESTS += [f'{BUILD_DIR}/{s[:-1]}o' for s in src] # replace .c extension with .o

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

rule link_exe
    command = $LINKER_EXE $LINKER_EXE_FLAGS -o $out $in $opts
    description = link(executable): $out


rule download
    command = curl -s $url -o $out
    description = download: $out
''')

build_ninja_segments.append(f'''\
{'\n'.join([f'build {obj}: compile {src}' for obj, src in zip(OBJECTS_LIBRARY, SOURCES_LIBRARY)])}

build {BUILD_DIR}/{LIB_NAME}: link_static {' '.join(OBJECTS_LIBRARY)}
build lib: phony {BUILD_DIR}/{LIB_NAME}
''')
TARGETS.append('lib')

build_ninja_segments.append(f'''\
build {EXEC_OBJECT}: compile {SOURCE_DIR}/{EXEC_SOURCE}

build {BUILD_DIR}/{EXEC_NAME}: link_exe {EXEC_OBJECT} {BUILD_DIR}/{LIB_NAME}
build app: phony {BUILD_DIR}/{EXEC_NAME}
''')
TARGETS.append('app')

if BUILD_TESTS:
    build_ninja_segments.append(f'''\
TEST_CODE_URL = "https://gist.githubusercontent.com/ivanp7/506fe8dc053952fd4a960666814cfd9a/raw"

build {TEST_HEADER}: download
    url = $TEST_CODE_URL/test.h
build {TEST_SOURCE}: download
    url = $TEST_CODE_URL/test.c
build {TEST_OBJECT}: compile {TEST_SOURCE} || {TEST_HEADER}
    opts = -I{BUILD_DIR}/{TEST_DIR}

{'\n'.join([f'build {obj}: compile {src} || {TEST_HEADER}\n\
    opts = -I{BUILD_DIR}/{TEST_DIR}' for obj, src in zip(OBJECTS_TESTS, SOURCES_TESTS)])}

build {BUILD_DIR}/{TESTS_NAME}: link_exe {TEST_OBJECT} {' '.join(OBJECTS_TESTS)} {BUILD_DIR}/{LIB_NAME}
build tests: phony {BUILD_DIR}/{TESTS_NAME}
''')
    TARGETS.append('tests')

build_ninja_segments.append(f'''\
build all: phony {' '.join(TARGETS)}
default all
''')

with open('build.ninja', 'w') as file:
    file.write(f'{build_ninja_separator}\n'.join(build_ninja_segments) + '\n')

