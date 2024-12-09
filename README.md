# Archipelago, a dynamic finite state machine implementation

Archipelago provides the following modules:

* `util`: various auxiliary utilies, such as linked lists, logging, shared memory interface, shared libraries interface;

* `app`: application plugin system (depends on `util`);

* `fsm`: finite state machine implementation (depends on `util`);

* `exe`: universal, multi-purpose executable configured via a shared memory (depends on `util`, `app`, `fsm`).

* `plugin`: built-in plugins providing most used types of resources (depends on `util`, `app`, `fsm`).

Application plugins serve to provide contexts of different kind (e.g. graphics)
to other contexts of plugins which implement application logic.
All contexts are created, shared around, and prepared according to the provided configuration
during the initialization phase. After initialization phase comes execution phase,
during which a finite state machine is run as described below.
The entry state and state transition of FSM are specified by configuration in shared memory.

This application was originally designed as a part of [Rayway](https://github.com/ivanp7/rayway)
engine to implement a (dynamic) rendering pipeline that can be configured at runtime.

## Finite state machine

### State

A finite state machine state is a pair of a function and a `void*` pointer to data
that will be provided to the function during the call:
```
state: (function, data)
```
State functions perform logically cohesive chunks of application logic.

During execution, a finite state machine maintains a stack of states.
State functions can push and pop states while doing state transition.

State function initiates state transition by returning or by calling `proceed()` function.
`proceed()` accepts the following parameters:

1. finite state machine context;

2. number of states to pop from the stack;

3. number of states to push from the stack;

4. array of states to push.

`proceed()` can be invoked not only from the state function itself,
but from any nested call depth. `proceed()` unwinds the call stack up to the state function,
removing the need of returning from multiple calls.

State function returns void and accepts a finite state machine context pointer (`fsm`).
State data pointer can be fetched from a finite state machine context.

### Transition

A transition (pair of a function and a `void*` pointer to data)
can be specified optionally for a finite state machine.
```
transition: (function, data)
```

Transition is useful for debugging and profiling purposes.
It stays the same during the whole finite state machine execution
and cannot be changed, and is encapsulated from state functions.

Transition function is called every time the state changes,
before the entry state (previous state is null),
and after the exit state (next state is null).
Transition function can provide a translational state as output,
which is inserted before the next state.

A transition function returns void and accepts the following parameters:

* previous state (`prev_state`, input passed by value);

* next state (`next_state`, input passed by value);

* translational state (`trans_state`, output passed by pointer);

* status code (`code`, input/output passed by pointer);

* pointer to transition data (`data`).

## Application plugin interface

A plugin is a shared library which implements some of the following functions:

* `help(topic) -> status`: displays plugin help on the specified `topic`;

* `init(config) -> context`: creates and initializes new context according to `config`;

* `final(context)`: finalizes and destroys `context` previously created by `init()`;

* `set(context, slot, value) -> status`: accepts `value` for internal use by `context`, the meaning is specified by `slot`;

* `get(context, slot, value_ptr) -> status`: retrieves data provided by `context` for `slot` to a location pointed to by `value_ptr`;

* `act(context, action, config) -> status`: invokes an internal `action` for `context` according to `config`.

Pointers to these functions are provided through a virtual table structure.
A plugin can provide multiple virtual tables.

This architecture allows to separate resource consumers from producers.

## Application configuration

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

### Standard plugins

Several plugins providing most useful resources are located in `plugin` directory.

Resources and features provided by the standard plugins include, but not limited to:

* concurrent processing using threads (`plugin/threads`); XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX FIXME
* signal management in multithreading environment (`plugin/signals`); XXXXXXXXXXXXX UNITE WITH threads
* quick & easy SDL windows for drawing (`sdl.*.h`);
* fonts support for drawing (`font.*.h`);
* OpenCL contexts (`opencl.*.h`).







The application helps with the initialization and release of resources
for the user-written plugins, which don't need to bother to do it manually.


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









## Examples

An example demonstration plugin can be found in the `demo` subdirectory.

Some of projects using Archipelago:

* [still-alive](https://github.com/ivanp7/still-alive) -- built as a plugin.

* [port](https://github.com/ivanp7/port) -- provides plugin for execution of kernel functions.

## Documentation

Doxygen documentation is available at `docs` subdirectory. To build it, run `make`.

## How to build

0. change directory to the repository root;

1. execute `configure.sh` to generate `build.ninja`;

2. execute `ninja` to build.

## Build dependencies

* [ninja](https://ninja-build.org/) -- build system

## Dependencies

* [glibc](https://www.gnu.org/software/libc/) -- POSIX; argument parsing with [argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html).

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

