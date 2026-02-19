#!/usr/bin/env python

# This script generates an Archipelago file for building an OpenCL program
# from sources and writing its binary to a file.

import argparse
import ctypes as c
import os
from pathlib import Path
import subprocess

from archi.object import PrimitiveData, String, KeyValueList
from archi.context import Parameters, Registry
from archi.script import errprint, write_input_file
from archi.builtin import (
        FileContext,
        LibraryContext,
        EnvVariableContext,
        )
from archi.opencl import (
        OpenCLContext,
        OpenCLProgramSrcContext,
        OpenCLProgramBinContext,
        )

###############################################################################
# Parse command line arguments

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
# Read contents of all input files

def read_sources(path_map):
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
    content = []

    for dirpath, filepaths in path_map.items():
        for filepath in filepaths:
            with open(Path(dirpath) / filepath, 'rb') as file:
                content.append(file.read())

    return content

content_headers = read_sources(args.hdr_map)
content_sources = read_sources(args.src_map)
content_libraries = read_binaries(args.lib_map)

###############################################################################
# Forming the list of operations

PLUGIN_OPENCL_PATHNAME = "libarchi_opencl.so"

app = Registry()
executable = app.require_context(Registry.KEY_EXECUTABLE, LibraryContext)

# Prepare built-in interfaces
library_interface = LibraryContext.interface(library=executable)
file_interface = FileContext.interface(library=executable)
envvar_interface = EnvVariableContext.interface(library=executable)

# Load OpenCL plugin
with app.temp_context(library_interface(pathname=PLUGIN_OPENCL_PATHNAME), key='plugin.opencl') as plugin_opencl:
    # Prepare OpenCL plugin interfaces
    opencl_context_interface = OpenCLContext.interface(library=plugin_opencl)
    opencl_program_src_interface = OpenCLProgramSrcContext.interface(library=plugin_opencl)
    opencl_program_bin_interface = OpenCLProgramBinContext.interface(library=plugin_opencl)

    # Create the OpenCL context
    with app.temp_context(opencl_context_interface(platform=args.platform, device=args.devices),
                          key='context.opencl') as opencl_context:
        # Prepare the list of dependency libraries of the program
        map_libraries = {}
        with app.temp_context(OpenCLProgramBinContext.InitParameters( \
                context=opencl_context, device_id=opencl_context.device_id), key='params.library') as params_library:
            for i, binary in enumerate(content_libraries):
                with app.temp_context([PrimitiveData.from_bytes(binary)], key='array.binaries') as array_binaries, \
                        app.temp_context(PrimitiveData((c.c_size_t * 1)(len(binary))), key='array.binary_sizes') as array_binary_sizes:
                    params_library.binaries = array_binaries
                    params_library.binary_sizes = array_binary_sizes

                library_key = f'context.program_library[{i}]'
                app[library_key] = opencl_program_bin_interface(params_library)
                map_libraries[library_key] = app[library_key]

        # Prepare program headers, sources, and library dependencies
        with app.temp_context(list(map_libraries.values()), key='array.program_libraries') as libraries, \
                app.temp_context(Parameters(**{path: String(content) for path, content in content_headers.items()}),
                                 key='params.headers') as headers, \
                app.temp_context(Parameters(**{path: String(content) for path, content in content_sources.items()}),
                                 key='params.sources') as sources, \
                app.temp_context(envvar_interface(default_value=args.cflags), key='envvar.cflags') as cflags_env, \
                app.temp_context(envvar_interface(default_value=args.lflags), key='envvar.lflags') as lflags_env:
            # Release the temporary contexts
            for library_key in map_libraries.keys():
                del app[library_key]

            del map_libraries

            # Build the program
            app['context.program'] = opencl_program_src_interface(context=opencl_context,
                                                                  device_id=opencl_context.device_id,
                                                                  headers=headers,
                                                                  sources=sources,
                                                                  libraries=libraries,
                                                                  cflags=cflags_env.CFLAGS,
                                                                  lflags=lflags_env.LFLAGS)

    with app.deleted_context('context.program') as opencl_program:
        # Write program binaries to output files
        for i, out_file in enumerate(args.out):
            # Open an output file and map it to memory
            with app.temp_context(file_interface(pathname=out_file,
                                                 size=opencl_program.binary.size[i],
                                                 readable=True, writable=True,
                                                 create=True, truncate=True,
                                                 mode=0o644), key='context.out_file') as file:
                # Write the program binary for the current device into the output file
                app.eval(file.write(src=opencl_program.binary[i]))

###############################################################################
# Generate the .archi file

file_contents = [(Registry.INPUT_FILE_KEY, KeyValueList.construct(app.operation_kvlist()))]
write_input_file(file_contents, pathname=args.file, mapaddr=args.mapaddr, print_report=True)

