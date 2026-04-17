#!/usr/bin/env python

# This script generates an Archipelago file for building an OpenCL program
# from sources and writing its binary to a file.

import argparse
import ctypes as c
import os
from pathlib import Path
import subprocess

from archi.object import PrimitiveData, String, KeyValueList
from archi.script import errprint, write_input_file
from archi.context import (
        Parameters,
        Registry,
        FileContext,
        LibraryContext,
        )
from archi.opencl import (
        PLUGIN_OPENCL,
        OpenCLContext,
        OpenCLProgramSrcContext,
        OpenCLProgramBinContext,
        )
from archi.helper.env import env_variables

###############################################################################
# Command line argument parser

class DirFileGroupAction(argparse.Action):
    """Generalized Action for grouping files under directories for a specific pair.
    """
    def __init__(self, option_strings, dest, group_id, is_dir, **kwargs):
        super().__init__(option_strings, dest, **kwargs)
        self.group_id = group_id    # For example: 'hdr', 'src', 'lib'
        self.is_dir = is_dir        # True for dir, False for file

    def __call__(self, parser, namespace, values, option_string=None):
        # Each group gets its own _map and _last_dir
        map_name = f'{self.group_id}_map'
        last_dir_name = f'_{self.group_id}_last_dir'

        file_map = getattr(namespace, map_name)

        if not hasattr(namespace, last_dir_name):
            setattr(namespace, last_dir_name, "")
            file_map.setdefault("", [])

        if self.is_dir:
            directory = values
            file_map.setdefault(directory, [])
            setattr(namespace, last_dir_name, directory)
        else:
            files = values if isinstance(values, list) else [values]
            last_dir = getattr(namespace, last_dir_name)
            file_map[last_dir].extend(files)


parser = argparse.ArgumentParser(
        description="Build an OpenCL library program and write the binaries to file system.")

parser.add_argument('--file', metavar="PATHNAME", help="Pathname of the generated .archi file (stdout if none)")
parser.add_argument('--mapaddr', type=lambda value: int(value, base=16), metavar="ADDRESS", help="Memory map address")

parser.add_argument('--platform', type=int, default=0, metavar="ID", help="OpenCL platform number")
parser.add_argument('--devices', nargs='+', type=int, default=[0], metavar="ID", help="OpenCL device number(s)")

parser.add_argument('--cflags', metavar="FLAGS", help="Compiler flags")
parser.add_argument('--lflags', metavar="FLAGS", help="Linker flags")

parser.add_argument('--hdrdir', metavar="PATH", action=DirFileGroupAction,
                    group_id='hdr', is_dir=True, help="Directory of header file(s)")
parser.add_argument('--hdr', nargs='+', metavar="PATHNAME", action=DirFileGroupAction,
                    group_id="hdr", is_dir=False, help="Header file(s)")

parser.add_argument('--srcdir', metavar="PATH", action=DirFileGroupAction,
                    group_id='src', is_dir=True, help="Directory of source file(s)")
parser.add_argument('--src', nargs='+', metavar="PATHNAME", action=DirFileGroupAction,
                    group_id='src', is_dir=False, help="Source file(s)")

parser.add_argument('--libdir', metavar="PATH", action=DirFileGroupAction,
                    group_id='lib', is_dir=True, help="Directory of library file(s)")
parser.add_argument('--lib', nargs='+', metavar="PATHNAME",  action=DirFileGroupAction,
                    group_id='lib', is_dir=False, help="Library file(s)")

parser.add_argument('--out', nargs='+', default=[], metavar="PATHNAME", required=True, help="Build output file(s)")

parser.set_defaults(hdr_map={}, src_map={}, lib_map={})

###############################################################################
# Parse command line arguments

args = parser.parse_args()

errprint(f"Generated file: {f"'{args.file}'" if args.file is not None else '<stdout>'}")
errprint(f"Map address: {f'0x{args.mapaddr:x}' if args.mapaddr is not None else '<random>'}")
errprint()
errprint(f"OpenCL platform #: {args.platform}")
errprint(f"OpenCL device #: {args.devices}")
errprint(f"OpenCL compiler flags: {f"'{args.cflags}'" if args.cflags else '<none>'}")
errprint(f"OpenCL linker flags: {f"'{args.lflags}'" if args.lflags else '<none>'}")
errprint()
errprint("Headers:")
for dirpath, filepaths in args.hdr_map.items():
    errprint(f"    '{dirpath}': {filepaths}")
errprint()
errprint("Sources:")
for dirpath, filepaths in args.src_map.items():
    errprint(f"    '{dirpath}': {filepaths}")
errprint()
errprint("Libraries:")
for dirpath, filepaths in args.lib_map.items():
    errprint(f"    '{dirpath}': {filepaths}")
errprint()
errprint(f"Build output file(s): {args.out}")
errprint()

if len(args.devices) != len(args.out):
    raise ValueError(f"Number of output files ({len(args.out)}) must be the same as the number of devices ({len(args.devices)})")

###############################################################################
# Functions for reading input files

def read_sources(path_map):
    """Read source files as strings and create a map from file names to contents.
    """
    content = {}

    for dirpath, filepaths in path_map.items():
        for filepath in filepaths:
            if filepath in content:
                raise ValueError("Duplicate file path detected")

            path = Path(dirpath) / filepath

            if 'PREPROCESSOR' in os.environ:
                result = subprocess.run([os.environ['PREPROCESSOR'], str(path)],
                                        stdout=subprocess.PIPE, stderr=None, text=True)

                if result.returncode != 0:
                    raise RuntimeError(f"Preprocessor script returned code {result.returncode}")

                content[filepath] = result.stdout
            else:
                with open(path, 'r') as file:
                    content[filepath] = file.read()

    return content


def read_binaries(path_map):
    """Read binary files as bytes and create a list of contents.
    """
    content = []

    for dirpath, filepaths in path_map.items():
        for filepath in filepaths:
            with open(Path(dirpath) / filepath, 'rb') as file:
                content.append(file.read())

    return content

###############################################################################
# Form the list of registry operations

def key(k):
    return f'program_builder.{k}'

PLUGIN_OPENCL_PATHNAME = f'lib{PLUGIN_OPENCL}.so'


app = Registry()
executable = app[Registry.KEY_EXECUTABLE]

# Prepare built-in interfaces
I_LIBRARY = LibraryContext.interface(library=executable)
I_FILE = FileContext.interface(library=executable)

# Load OpenCL plugin
with app.temp_context(I_LIBRARY(pathname=PLUGIN_OPENCL_PATHNAME), key=key('plugin.opencl')) as plugin_opencl:
    # Prepare OpenCL plugin interfaces
    I_OPENCL_CONTEXT = OpenCLContext.interface(library=plugin_opencl)
    I_OPENCL_PROGRAM_SRC = OpenCLProgramSrcContext.interface(library=plugin_opencl)
    I_OPENCL_PROGRAM_BIN = OpenCLProgramBinContext.interface(library=plugin_opencl)

    # Create the OpenCL context
    with app.temp_context(I_OPENCL_CONTEXT(platform=args.platform, device=args.devices),
                          key=key('cl_context')) as opencl_context:
        # Prepare the list of dependency libraries of the program
        list_libraries = []

        with app.temp_context(OpenCLProgramBinContext.InitParameters(context=opencl_context,
                                                                     device_id=opencl_context.device_id),
                              key=key('library.params')) as params_library:
            # Read binaries of the program libraries
            list_binaries = [PrimitiveData.from_bytes(binary) for binary in read_binaries(args.lib_map)]

            # Create program libraries from binaries and append them to the list
            for i, binary in enumerate(list_binaries):
                with app.temp_context([binary], key=key('array_binary[{i}]')) as array_binary:
                    list_libraries.append(app.new_context(I_OPENCL_PROGRAM_BIN(params_library,
                                                                               binaries=array_binary,
                                                                               binary_sizes=[binary.total_size]),
                                                          key=key('library[{i}]')))

            del list_binaries

        # Prepare program headers, sources, and library dependencies
        dict_headers = {path: String(content) for path, content in read_sources(args.hdr_map).items()}
        dict_sources = {path: String(content) for path, content in read_sources(args.src_map).items()}

        with app.temp_context(list_libraries, key=key('array_libraries')) as libraries, \
                app.temp_context(Parameters(**dict_headers), key=key('kvlist_headers')) as headers, \
                app.temp_context(Parameters(**dict_sources), key=key('kvlist_sources')) as sources, \
                env_variables(app, CFLAGS=args.cflags, LFLAGS=args.lflags) as (cflags, lflags):
            # Delete contexts of program libraries
            for library_context in list_libraries:
                app.del_context(library_context)

            del list_libraries
            del dict_headers
            del dict_sources

            # Build the program
            app[key('program')] = I_OPENCL_PROGRAM_SRC(context=opencl_context, device_id=opencl_context.device_id,
                                                       headers=headers, sources=sources, libraries=libraries,
                                                       cflags=cflags, lflags=lflags)

    with app.deleted_context(key('program')) as opencl_program:
        # Write program binaries to output files
        for i, out_file in enumerate(args.out):
            with app.temp_context(I_FILE(pathname=out_file,
                                         size=opencl_program.binary.size[i],
                                         readable=True, writable=True,
                                         create=True, truncate=True,
                                         mode=0o644),
                                  key=key('out_file[{i}]')) as file:
                # Write the program binary for the current device into the output file
                app(file.write(src=opencl_program.binary[i]))

###############################################################################
# Generate the .archi file

file_contents = [(Registry.INPUT_FILE_KEY, KeyValueList.construct(app.operations()))]
write_input_file(file_contents, pathname=args.file, mapaddr=args.mapaddr, print_report=True)

