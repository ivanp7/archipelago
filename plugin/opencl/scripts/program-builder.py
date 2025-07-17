#!/usr/bin/env python

# This script generates an Archipelago file for building an OpenCL program
# from sources and writing its binary to a file.

import argparse
import ctypes as c
import os
from pathlib import Path
import subprocess

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
print(f"OpenCL compiler flags: {f"'args.cflags'" if args.cflags else '<none>'}")
print(f"OpenCL linker flags: {f"'args.lflags'" if args.lflags else '<none>'}")
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

def read_sources(path_map, allow_duplicates=True):
    content = {}

    for dirpath, filepaths in path_map.items():
        for filepath in filepaths:
            if not allow_duplicates and filepath in content:
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

content_headers = read_sources(args.hdr_map, allow_duplicates=False)
content_sources = read_sources(args.src_map)
content_libraries = read_binaries(args.lib_map)

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
str2num_interface = ContextInterface(executable.archi_context_converter_string_to_number_interface)

# Load OpenCL plugin
with app.context('plugin.opencl', library_interface(pathname="libarchip_opencl.so")) as plugin_opencl:
    # Prepare used OpenCL plugin interfaces
    opencl_context_interface = ContextInterface(plugin_opencl.archip_context_opencl_context_interface)
    opencl_program_src_interface = ContextInterface(plugin_opencl.archip_context_opencl_program_src_interface)
    opencl_program_bin_interface = ContextInterface(plugin_opencl.archip_context_opencl_program_bin_interface)

    # Create the OpenCL context
    with app.context('string.platform_idx', envvar_interface(name="PLATFORM_IDX",
                                                             default_value=str(args.plt))) as platform_idx_str, \
            app.context('string.device_idx', envvar_interface(name="DEVICE_IDX",
                                                              default_value=str(args.dev))) as device_idx_str, \
            app.context('value.platform_idx', str2num_interface(as_uint=platform_idx_str)) as platform_idx, \
            app.context('value.device_idx', str2num_interface(as_uint=device_idx_str)) as device_idx:
        app['context.opencl'] = opencl_context_interface(platform_idx=platform_idx, device_idx=device_idx)

    with app['context.opencl'] as opencl_context:
        # Prepare the list of dependency libraries of the program
        map_libraries = {}
        with app.context('params.library', Parameters(context=opencl_context,
                                                      device_id=opencl_context.device_id)) as params_library:
            for i, binary in enumerate(content_libraries):
                with app.context('array.binary', [CValue(binary)]) as array_binary:
                    params_library.binaries = array_binary.elements

                library_key = f'context.library[{i}]'
                app[library_key] = opencl_program_bin_interface(params_library)
                map_libraries[library_key] = app[library_key]

        # Prepare program headers, sources, and library dependencies
        with app.context('array.libraries', list(map_libraries.values())) as libraries, \
                app.context('hashmap.headers', hashmap_interface(capacity=HASHMAP_CAPACITY)) as headers, \
                app.context('hashmap.sources', hashmap_interface(capacity=HASHMAP_CAPACITY)) as sources, \
                app.context('string.cflags', envvar_interface(name='CFLAGS', default_value=args.cflags)) as cflags, \
                app.context('string.lflags', envvar_interface(name='LFLAGS', default_value=args.lflags)) as lflags:
            # Release the temporary contexts
            for library_key in map_libraries.keys():
                del app[library_key]

            del map_libraries

            # Fill the hashmaps
            for path, contents in content_headers.items():
                setattr(headers, path, CValue(contents))

            for path, contents in content_sources.items():
                setattr(sources, path, CValue(contents))

            app['context.program'] = opencl_program_src_interface(context=opencl_context,
                                                                  device_id=opencl_context.device_id,
                                                                  headers=headers,
                                                                  sources=sources,
                                                                  libraries=libraries,
                                                                  cflags=cflags,
                                                                  lflags=lflags)

    with app['context.program'] as opencl_program:
        # Open the output file and map it to memory
        with app.context('string.pathname', envvar_interface(name='OUT', default_value=args.out)) as pathname, \
                app.context('file.out', file_interface(pathname=pathname,
                                                       size=opencl_program.binary_size[0],
                                                       create=TRUE, truncate=TRUE,
                                                       readable=TRUE, writable=TRUE,
                                                       mode=c.c_int(0o644))) as file, \
                app.context('file.memory', file.map(writable=TRUE, shared=TRUE)) as file_memory:
            # Write the program binary into the output file
            file_memory.copy(source=opencl_program.binary[0])

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

# Report the sizes
file_size_b = file_memory.size()
file_size_kib = file_size_b / 1024
file_size_mib = file_size_kib / 1024
file_size_gib = file_size_mib / 1024

file_padding_b = file_memory.padding()
file_padding_kib = file_padding_b / 1024
file_padding_mib = file_padding_kib / 1024

print(f"Wrote {file_size_b} bytes ({file_size_kib:.1f} KiB, {file_size_mib:.1f} MiB, {file_size_gib:.1f} GiB) to '{args.file}',")
print(f"including {file_padding_b} padding bytes ({file_padding_kib:.1f} KiB, {file_padding_mib:.1f} MiB)")

