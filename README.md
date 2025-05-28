# Archipelago

A versatile, modular application configured via memory-mapped files and plugins that define its specific behavior.

## Usage Overview

The `archi` executable accepts one or more memory-mapped initialization files containing instructions and supplementary data.
It processes these input files sequentially to reach the desired state and perform specified actions.

## Contexts

The basic unit of operation is a **context**: any object or resource conforming to a generic interface supporting:

- `init()`: Allocate/initialize context using provided parameters.
- `final()`: Finalize/destroy the context.
- `get()`: Retrieve a pointer from the context by slot (name + list of integer indices).
- `set()`: Assign a pointer within the context by slot (name + indices).
- `act()`: Invoke an action on the context by slot (name + indices).

All contexts are registered in an internal registry -- a hashmap with string keys.

For comprehensive descriptions see [docs/contexts.md](docs/contexts.md).

## Instruction Types

Initialization files may contain different instruction types:

- `NOOP`: No operation.
- `INIT_FROM_CONTEXT`: Initialize new context using the interface of another existing one; insert into registry.
- `INIT_FROM_SLOT`: Get interface from another's slot; initialize new context; insert into registry.
- `COPY`: Create a context alias.
- `FINAL`: Finalize/remove specified context from registry.
- `SET_TO_VALUE`: Set value/data pointer in given slot of specified context.
- `SET_TO_CONTEXT_DATA`: Set context data pointer in given slot of specified context.
- `SET_TO_CONTEXT_SLOT`: Copy pointer across slots between two contexts as instructed.
- `ACT`: Trigger named action on target context.

For comprehensive descriptions see [docs/instructions.md](docs/instructions.md).

## Built-in Contexts

Several built-in contexts exist at startup:
  - `"archi.registry"`: The registry itself (allowing introspection/manipulation).
  - `"archi.executable"`: Library handle of the executable (for access to built-ins).
  - `"archi.input_file"`: Currently processed input file (memory-mapped contents).
  - `"archi.signal"`: Signal management context (access signal flags/install handlers).

For comprehensive descriptions see [docs/builtins.md](docs/builtins.md).

## Built-in Interfaces

These are the exported context interface symbols built into the executable.
They provide foundational mechanisms for configuration, memory management,
processing, data structures, inter-process communication (IPC), resources access, and concurrency.

- **Meta Features**
  - `archi_context_parameters_interface`: Parameter list used for `init()` and `act()` context operations.
  - `archi_context_pointer_interface`: Copied pointer with custom attributes.

- **Memory Management**
  - `archi_context_memory_interface`: Generic memory allocation context.
  - `archi_context_memory_mapping_interface`: Pointer to a mapped region of memory.

- **Hierarchical State Processing (HSP)**
  - `archi_context_hsp_state_interface`: HSP state (function + data + metadata).
  - `archi_context_hsp_transition_interface`: HSP transition (function + data).
  - `archi_context_hsp_interface`: HSP instance (entry state + transition).
  - `archi_context_hsp_frame_interface`: HSP frame (sequence of states).
  - `archi_context_hsp_branch_state_data_interface`: Data for a HSP branch state.

- **Data Structures**
  - `archi_context_ds_hashmap_interface`: Hashmap.
  - `archi_context_ds_lfqueue_interface`: Lock-free queue.

- **Interprocess Communication & Signals**
  - `archi_context_ipc_env_interface`: Value of an environmental variable.
  - `archi_context_ipc_signal_handler_interface`: Signal handler (function + data).

- **OS Resources**
  - `archi_context_res_file_interface`: Open and/or mapped file.
  - `archi_context_res_library_interface`: Loaded dynamic library handle.

- **Thread Groups**
  - `archi_context_res_thread_group_interface`: Thread group context.
  - `archi_context_res_thread_group_work_interface`: Individual task assigned to thread groups.
  - `archi_context_res_thread_group_callback_interface`: Callback invoked upon completion of thread group tasks.
  - `archi_context_res_thread_group_dispatch_data_interface`: Data for a HSP state for thread group work dispatch.

For comprehensive descriptions see [docs/builtins.md](docs/builtins.md).

## External Plugins

There are several external plugins provided in `/plugin` directory:

  - `sdl` (SDL2 windows, PSFv2 font)
  - `opencl` (parallel compute offload using OpenCL)

## Python-based Generation of Initialization Files

Initialization files for `archi` can be generated using the `archi` Python module.
The typical workflow is as follows:

1. Create an instance of the `Application` class.
2. Manipulate its context registry as if operating directly on live contexts.
3. Serialize ("fossilize") the internal state of your application instance into a byte array.
4. Write this byte array to a file.

You interact with contexts in the registry via dictionary-like access: `app[key]`,
where `app` is your Application instance and `key` is a string identifier.

- Assigning to `app[key]` initializes a new context and inserts it into the registry.
- Deleting (`del app[key]`) finalizes and removes that context from the registry.

To access slots within contexts:
- Use attribute-style selectors: e.g., `app["foo"].bar`
- Nested fields are supported (`comp1.comp2.comp3`)
- Integer indices may also be used (`comp1.comp2[index1][index2]`)
- Invoke actions on slots using function-call syntax; these accept keyword arguments or an optional parameter list context as positional argument—e.g.,  `app["foo"].bar(arg1=value1, arg2=value2)`, or `app["foo"].bar(app["baz"], arg1=value1, arg2=value2)`.

To create new contexts:
First declare their interface by instantiating a ``ContextInterface`` object (constructor accepts another context or slot).
This provides its own callable operator accepting similar arguments; instead of executing immediately,
it returns a ``ContextSpec`` which you assign to ``app[key]`` in order to initialize/register that new context.

Here is an example script:

```python
import ctypes as c
import sys

import archi


VALUE_TRUE = archi.CValue(c.c_bool(True)) # define a value like this to make it reusable in the file and save bytes


app = archi.Application()

# Add signals to the watch set
def watch_signals():
    signal_watch_set = archi.archi_signal_watch_set_t()
    signal_watch_set.f_SIGINT = True
    signal_watch_set.f_SIGQUIT = True
    signal_watch_set.f_SIGTERM = True
    signal_watch_set.f_SIGCONT = True

    app.add_contents({archi.Application.CONTENT_SIGNALS: signal_watch_set})
watch_signals()
del watch_signals

# Load plugins and create contexts
executable = app[archi.Application.KEY_EXECUTABLE] # handle of the executable itself

library_loader = archi.ContextInterface(executable) # the context interface for library handles
app["plugin.sdl"] = library_loader(pathname="libarchip_sdl.so") # load SDL plugin
del library_loader

sdl_library_interface = archi.ContextInterface(
        app["plugin.sdl"].archip_context_sdl2_library_interface) # SDL2 library interface (SDL_Init(), SDL_Quit())
app["context.sdl_library"] = sdl_library_interface(
        video=VALUE_TRUE, audio=VALUE_TRUE) # initialize SDL2 video and audio subsystems
del sdl_library_interface

sdl_window_interface = archi.ContextInterface(
        app["plugin.sdl"].archip_context_sdl2_window_interface) # SDL2 window context interface
app["context.window"] = sdl_window_interface(texture_width=c.c_int(512), texture_height=c.c_int(256),
                                             window_title="Still Alive") # create a window
del sdl_window_interface

del app["plugin.sdl"] # unload SDL plugin

# Fossilize the application
app_memory = app.memory() # get high-level memory description object
app_memory.pack() # pack the memory to decrease size
app_file_contents = app_memory.fossilize(0x7ffffff00000) # fossilize the memory
                                                         # it is to be mapped at the specified address

# Write the file
with open(sys.argv[1], mode='wb') as file:
    file.write(app_file_contents)

print(f"Wrote {app_memory.size()} bytes to '{sys.argv[1]}',")
print(f"including {app_memory.padding()} padding bytes")

```

For comprehensive descriptions see [docs/python.md](docs/python.md).

## binfmt\_misc

Archipelago init files can be made directly executable on Linux systems.

To enable this feature, run `register-binfmt.sh` as root to register the file format with the kernel via `binfmt_misc`.
This allows you to execute `.archi` files without explicitly calling the `archi` binary:

```sh
./file.archi
```

instead of:

```sh
archi ./file.archi
```

Ensure that `archi` is installed in your system path so the kernel can locate it when executing `.archi` files.

# Doxygen ocumentation

Doxygen documentation is available at `/doxygen` subdirectory. To build it, run `make`.

# How to build

0. go to the repository root directory;
1. (optional) export environmental variables to control behavior of `configure.py` (see accepted environmental variables in the script);
2. run `configure.py` to generate `build.ninja`;
3. run `ninja` to build the project.

# Build dependencies

* [ninja](https://ninja-build.org/) -- build system

# Dependencies

* [glibc](https://www.gnu.org/software/libc/) -- POSIX; argument parsing with [argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html).

# License

```
   Archipelago is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Archipelago is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with Archipelago. If not, see <http://www.gnu.org/licenses/>.
```

