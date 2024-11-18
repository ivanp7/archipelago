# Archipelago, a dynamic finite state machine implementation

The application helps with the initialization and release of resources
for the user-written plugins, which don't need to bother to do it manually.

Besides the features of the library, the application is also capable
to create OpenCL contexts (`opencl.*.h`).

The application works as follows:

1. Command line arguments are parsed.

2. Plugin file is loaded.

3. Plugin configuration function is called.
This function allows the plugin to select and configure features provided by
the application, and also to parse command line arguments for the plugin.

4. Application resources are initialized by the application.

5. Plugin initialization function is called,
the application resources are passed to it by pointers.
The plugin prepares the initial finite state machine state, and creates its own resources.

6. The finite state machine is executed.

```c
while (application.fsm.state.sfunc != NULL)
    application.fsm.state.sfunc(&application.fsm.state, application.fsm.data);
```

7. Plugin finalization function is called.
This function allows the plugin to destroy its own resources.

8. Application resources are released by the application.

## User plugin

The user plugin interface is defined in `plugin.*.h`.
A plugin must implement 4 functions:

1. `help()`: used to print usage help when `--help` option is provided to the application.

2. `configure()`: used to select and configure features provided by the application,
and also to parse command line arguments for the plugin.

3. `init()`: used to initialize the plugin's own resources, and also to prepare
the initial finite state machine state.

4. `final()`: used to destroy the plugin's own resources, and set the application exit code.

## Built-in plugins

The library provides types, functions, macros, constants for the following things:

* finite-state machines as algorithm of execution (`fsm.*.h`);
* concurrent processing using threads (`concurrent.*.h`);
* signal management in multithreading environment (`signal.*.h`);
* quick & easy SDL windows for drawing (`sdl.*.h`);
* fonts support for drawing (`font.*.h`);
* other!

## Examples

A quick and dirty demo plugin can be found in the `demo` subdirectory.

List of projects using Archipelago:

* [still-alive](https://github.com/ivanp7/still-alive)








## Documentation

Doxygen documentation is available at `docs` subdirectory. To build it, run `make`.

## How to build

The project is built using the Ninja build system.
The `build.ninja` file is generated using the `pyrate-build` pip package.

The installation steps:

1. `./prepare.sh`: prepare the Python virtual environment and install pyrate

2. `./configure.sh`: generate `build.ninja` and `archipelago.pc` (pkg-config file for `libarchipelago.a`)

3. `ninja`: build the project

## Build dependencies

* [ninja](https://ninja-build.org/) -- build system
* [pyrate](https://pypi.org/project/pyrate-build/) -- ninja build file generator

## Dependencies

* The GNU C Library -- for [argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html),
[ftok()](https://man7.org/linux/man-pages/man3/ftok.3.html), [shmget()](https://man7.org/linux/man-pages/man3/shmget.3p.html),
[shmat()](https://man7.org/linux/man-pages/man3/shmat.3p.html), [shmdt()](https://man7.org/linux/man-pages/man3/shmdt.3p.html),
[dlopen(), dlclose()](https://man7.org/linux/man-pages/man3/dlopen.3.html), [dlsym()](https://man7.org/linux/man-pages/man3/dlsym.3.html)

## License

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

