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

parser = argparse.ArgumentParser(
        description="Build an OpenCL library program and write the binaries to file system.")

parser.add_argument('--file', metavar="PATHNAME", required=True, help="Pathname of the generated .archi file")
parser.add_argument('--mapaddr', type=lambda value: int(value, base=16), default=DEFAULT_MAP_ADDRESS,
                    metavar="ADDRESS", help="Memory map address")

parser.add_argument('--plt', type=int, default=0, metavar="ID", help="OpenCL platform number")
parser.add_argument('--dev', type=int, default=0, metavar="ID", help="OpenCL device number")

parser.add_argument('--cflags', metavar="FLAGS", help="Compiler flags")
parser.add_argument('--lflags', metavar="FLAGS", help="Linker flags")

parser.add_argument('--hdrdir', default=".", metavar="PATH", help="Directory of header file(s)")
parser.add_argument('--hdr', nargs='+', default=[], metavar="PATHNAME", help="Header file(s)")

parser.add_argument('--srcdir', default=".", metavar="PATH", help="Directory of source file(s)")
parser.add_argument('--src', nargs='+', default=[], metavar="PATHNAME", help="Source file(s)")

parser.add_argument('--libdir', default=".", metavar="PATH", help="Directory of library file(s)")
parser.add_argument('--lib', nargs='+', default=[], metavar="PATHNAME", help="Library file(s)")

parser.add_argument('--outdir', default=".", metavar="PATH", help="Directory of build output file")
parser.add_argument('--out', metavar="PATHNAME", required=True, help="Build output file")

args = parser.parse_args()

print(f"Generated file: {args.file}")
print(f"Map address: 0x{args.mapaddr:x}")
print()
print(f"OpenCL platform #: {args.plt}")
print(f"OpenCL device #: {args.dev}")
print(f"OpenCL compiler flags: {args.cflags if args.cflags else ""}")
print(f"OpenCL linker flags: {args.lflags if args.lflags else ""}")
print()
print(f"Header directory: {args.hdrdir}")
print(f"Headers: {args.hdr}")
print()
print(f"Source directory: {args.srcdir}")
print(f"Sources: {args.src}")
print()
print(f"Library directory: {args.libdir}")
print(f"Libraries: {args.lib}")
print()
print(f"Build output directory: {args.outdir}")
print(f"Build output: {args.out}")
print()

if len(args.hdr) != len(set(args.hdr)):
    raise RuntimeError("Header file pathnames must be all unique")
elif len(args.src) != len(set(args.src)):
    raise RuntimeError("Source file pathnames must be all unique")
elif len(args.lib) != len(set(args.lib)):
    raise RuntimeError("Library file pathnames must be all unique")

###############################################################################
# Read contents of all input files

content_headers = {}
for path in args.hdr:
    with open(Path(args.hdrdir) / path, 'r') as file:
        content_headers[path] = file.read()

content_sources = {}
for path in args.src:
    with open(Path(args.srcdir) / path, 'r') as file:
        content_sources[path] = file.read()

content_libraries = []
for path in args.lib:
    with open(Path(args.srcdir) / path, 'rb') as file:
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
        app['params.library'].device_id = app['context.opencl'].device_id
        app['params.library'].context = app['context.opencl']

        libraries = []
        for i, binary in enumerate(content_libraries):
            app['params.library'].binaries = [CValue(binary)]
            app[f'context.library[{i}]'] = opencl_program_bin_interface(app['params.library'])
            libraries.append(app[f'context.library[{i}]'])
        app['array.libraries'] = libraries

        for i in range(len(content_libraries)):
            del app[f'context.library[{i}]']

        # Form the parameter list for program building
        app['params.program'] = Parameters(app['params.library'], cflags=args.cflags, lflags=args.lflags)
        app['params.program'].libraries = app['array.libraries']
        app['params.program'].sources = app['hashmap.sources']
        app['params.program'].headers = app['hashmap.headers']

        del app['params.library']
        del app['hashmap.headers']
        del app['hashmap.sources']
        del app['array.libraries']

    # Build the program
    app['context.program'] = opencl_program_src_interface(app['params.program'])
    with app['context.program'] as program:
        del app['params.program']

        # Form the parameter list for output file opening
        app['params.open'] = Parameters(pathname=str(Path(args.outdir) / args.out),
                                        create=TRUE, truncate=TRUE, readable=TRUE, writable=TRUE, mode=c.c_int(0o644))
        app['params.open'].size = app['context.program'].binary_size[0]

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

