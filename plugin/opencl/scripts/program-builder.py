#!/usr/bin/env python

# This script generates an Archipelago file for building an OpenCL program
# from sources and writing its binary to a file.

import argparse
import ctypes as c
from pathlib import Path

from archi.memory import CValue
from archi.registry import Registry, ContextInterface, Parameters
from archi.file import File

###############################################################################
# Parse command line arguments

DEFAULT_MAP_ADDRESS = 0x7f0000000000

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

parser.add_argument('--file', metavar="PATHNAME", required=True, help="Pathname of the generated .archi file")
parser.add_argument('--mapaddr', type=lambda value: int(value, base=16), default=DEFAULT_MAP_ADDRESS,
                    metavar="ADDRESS", help="Memory map address")

parser.add_argument('--plt', type=int, default=0, metavar="ID", help="OpenCL platform number")
parser.add_argument('--dev', type=int, default=0, metavar="ID", help="OpenCL device number")

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

parser.add_argument('--out', metavar="PATHNAME", required=True, help="Build output file")

parser.set_defaults(hdr_map={}, src_map={}, lib_map={})
args = parser.parse_args()

print(f"Generated file: '{args.file}'")
print(f"Map address: 0x{args.mapaddr:x}")
print()
print(f"OpenCL platform #: {args.plt}")
print(f"OpenCL device #: {args.dev}")
print(f"OpenCL compiler flags: '{args.cflags if args.cflags else ""}'")
print(f"OpenCL linker flags: '{args.lflags if args.lflags else ""}'")
print()
print("Headers:")
for dirpath, filepaths in args.hdr_map.items():
    print(f"    '{dirpath}': {filepaths}")
print()
print("Sources:")
for dirpath, filepaths in args.src_map.items():
    print(f"    '{dirpath}': {filepaths}")
print()
print("Libraries:")
for dirpath, filepaths in args.lib_map.items():
    print(f"    '{dirpath}': {filepaths}")
print()
print(f"Build output file: '{args.out}'")
print()

###############################################################################
# Read contents of all input files

content_headers = {}
for dirpath, filepaths in args.hdr_map.items():
    for filepath in filepaths:
        with open(Path(dirpath) / filepath, 'r') as file:
            if filepath in content_headers:
                raise ValueError("Duplicate header detected")

            content_headers[filepath] = file.read()

content_sources = {}
for dirpath, filepaths in args.src_map.items():
    for filepath in filepaths:
        with open(Path(dirpath) / filepath, 'r') as file:
            if filepath in content_sources:
                raise ValueError("Duplicate source detected")

            content_sources[filepath] = file.read()

content_libraries = []
for dirpath, filepaths in args.lib_map.items():
    for filepath in filepaths:
        with open(Path(dirpath) / filepath, 'rb') as file:
            content_libraries.append(file.read())

###############################################################################
# Forming the list of instructions

HASHMAP_CAPACITY = CValue(c.c_size_t(1024))
TRUE = CValue(c.c_bool(True))

app = Registry()

# Prepare used built-in interfaces
executable = app[Registry.KEY_EXECUTABLE]
library_interface = ContextInterface(executable) # or executable.archi_context_res_library_interface
file_interface = ContextInterface(executable.archi_context_res_file_interface)
hashmap_interface = ContextInterface(executable.archi_context_ds_hashmap_interface)
envvar_interface = ContextInterface(executable.archi_context_ipc_env_interface)

# Load OpenCL plugin
app['plugin.opencl'] = library_interface(pathname="libarchip_opencl.so")
with app['plugin.opencl'] as plugin_opencl:
    # Prepare used OpenCL plugin interfaces
    opencl_context_interface = ContextInterface(plugin_opencl.archip_context_opencl_context_interface)
    opencl_program_src_interface = ContextInterface(plugin_opencl.archip_context_opencl_program_src_interface)
    opencl_program_bin_interface = ContextInterface(plugin_opencl.archip_context_opencl_program_bin_interface)

    # Create the OpenCL context
    app['context.opencl'] = opencl_context_interface(platform_idx=c.c_uint(args.plt),
                                                     device_idx=c.c_uint(args.dev))
    with app['context.opencl'] as opencl_context:
        # Prepare program sources
        app['hashmap.headers'] = hashmap_interface(capacity=HASHMAP_CAPACITY)
        for key, value in content_headers.items():
            setattr(app['hashmap.headers'], key, CValue(value))

        app['hashmap.sources'] = hashmap_interface(capacity=HASHMAP_CAPACITY)
        for key, value in content_sources.items():
            setattr(app['hashmap.sources'], key, CValue(value))

        # Prepare libraries the program depends on
        app['params.library'] = Parameters()
        with app['params.library'] as params_library:
            params_library.device_id = app['context.opencl'].device_id
            params_library.context = app['context.opencl']

            libraries = []
            for i, binary in enumerate(content_libraries):
                app['array.binary'] = [CValue(binary)]
                with app['array.binary'] as array_binary:
                    params_library.binaries = array_binary.elements

                app[f'context.library[{i}]'] = opencl_program_bin_interface(params_library)
                libraries.append(app[f'context.library[{i}]'])
            app['array.libraries'] = libraries

            for i in range(len(content_libraries)):
                del app[f'context.library[{i}]']

        # Form the parameter list for program building
        app['params.program'] = Parameters(cflags=args.cflags, lflags=args.lflags)
        app['params.program'].libraries = app['array.libraries']
        app['params.program'].sources = app['hashmap.sources']
        app['params.program'].headers = app['hashmap.headers']
        app['params.program'].device_id = app['context.opencl'].device_id
        app['params.program'].context = app['context.opencl']

        del app['hashmap.headers']
        del app['hashmap.sources']
        del app['array.libraries']

    # Build the program
    app['context.program'] = opencl_program_src_interface(app['params.program'])
    with app['context.program'] as program:
        del app['params.program']

        # Form the parameter list for output file opening
        app['value.pathname'] = envvar_interface(name='OUT', default_value=args.out)
        with app['value.pathname'] as pathname:
            app['params.open'] = Parameters(create=TRUE, truncate=TRUE,
                                            readable=TRUE, writable=TRUE, mode=c.c_int(0o644))
            app['params.open'].size = app['context.program'].binary_size[0]
            app['params.open'].pathname = pathname

        # Open the output file
        app['file.out'] = file_interface(app['params.open'])
        with app['file.out'] as file:
            del app['params.open']

            # Create a pointer into mapped file memory
            app['file.memory'] = app['file.out'].map(writable=TRUE, shared=TRUE)
            with app['file.memory'] as memory:
                # Write the program binary into the output file
                app['params.copy'] = Parameters()
                app['params.copy'].source = app['context.program'].binary[0]

                app['file.memory'].copy(app['params.copy'])

                del app['params.copy']

###############################################################################
# Generate the .archi file

file = File()

file[File.KEY_REGISTRY] = app

file_memory = file.memory()
file_memory.pack() # get rid of padding bytes where possible

file_memory_buffer = file_memory.fossilize(args.mapaddr)

# Write the file
with open(args.file, mode='wb') as file:
    file.write(file_memory_buffer)

print(f"Wrote {file_memory.size()} bytes to '{args.file}',")
print(f"including {file_memory.padding()} padding bytes")

