# Station, a resource provider library/application

## Features of the library (`libstation`)

The library provides types, functions, macros, constants for the following things:

* finite-state machines as algorithm of execution (`fsm.*.h`);
* concurrent processing using threads (`concurrent.*.h`);
* signal management in multithreading environment (`signal.*.h`);
* quick & easy SDL windows for drawing (`sdl.*.h`);
* fonts support for drawing (`font.*.h`);
* other!

## Features of the application (`station-app`)

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

It is possible to link `station-app` with a user plugin,
so that the latter becomes a standalone application.

## Contents of `station-app --help`

```
station-app [options...] PLUGIN_FILE [-- [plugin options...]]
    or
station-app --help [PLUGIN_FILE [-- [plugin help options...]]]
    or
station-app --cl-list[=TYPE]

Usage:

  -C, --conf=FILE               (multiple) Parse arguments from file

Alternative modes:
  -h, --help                    Display usage help for app or plugin
  -l, --cl-list[=TYPE]          Display list of OpenCL-compatible hardware
                                  (possible values="platforms", "devices"
                                  default=`devices')

Output options:
  -v, --verbose                 Display more information
  -i, --logo                    Display application logo

Execution options:
  -j, --threads=[-]THREADS      (multiple) Create concurrent processing
                                  context, negative means busy-wait
  -c, --cl-context=PLATFORM_IDX[:DEVICE_IDX_MASK]
                                (multiple) Create OpenCL context
  -n, --no-sdl                  Don't initialize SDL subsystems

Data options:
  -f, --file=FILE               (multiple) Create data buffer from file

Signal management:
      --SIGALRM                 Watch signal
      --SIGCHLD                 Watch signal
      --SIGCONT                 Watch signal
      --SIGHUP                  Watch signal
      --SIGINT                  Watch signal
      --SIGQUIT                 Watch signal
      --SIGTERM                 Watch signal
      --SIGTSTP                 Watch signal
      --SIGTTIN                 Watch signal
      --SIGTTOU                 Watch signal
      --SIGUSR1                 Watch signal
      --SIGUSR2                 Watch signal
      --SIGWINCH                Watch signal
```

## How to build

The project is built using the Ninja build system.
A `build.ninja` file is generated using `configure` script.
To see the possible build configuration options, run `configure` without arguments.
To generate `build.ninja`, run `configure <options>`.
To build, run `ninja`.

The configuration script also generates `station.pc` (pkg-config file for `libstation`)
and `station-app.pc` (pkg-config file for standalone executable user plugins).

## Build dependencies

* gcc-compatible compiler (like clang)
* pkg-config
* gengetopt
* ninja

## Dependencies

The code implies the availability of the POSIX interfaces.

The following dependencies are optional.
Each of them is required only if the corresponding feature
is enabled at the build configuration step.

* SDL2
* OpenCL

## Examples

A quick and dirty demo plugin can be found in the `demo` subdirectory.

List of projects using Station:

* [still-alive](https://github.com/ivanp7/still-alive) (built as a standalone executable plugin)

## Documentation

Doxygen documentation is available at `docs` subdirectory. To build it, run `make`.

## License

```
   Station is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Station is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with Station. If not, see <http://www.gnu.org/licenses/>.
```

