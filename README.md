# Archipelago, a dynamic finite state machine implementation

The executable:

1. obtains an application configuration from attached shared memory;
2. loads shared libraries providing plugins as specified in the configuration;
3. performs initialization instructions (creating contexts) as specified in the configuration;
4. executes a finite state machine with the specified entry state and state transition;
5. finalizes the application (destroying contexts).

Application plugins serve to provide contexts of different kind
to other plugins which implement application logic.
The application does the initialization and finalization of resources
for the user-written plugins, which don't need to bother to do it manually.
All contexts are created, shared around, and prepared according to the provided configuration
during the initialization phase. After initialization phase comes execution phase,
during which a finite state machine is run as described below.
The entry state and state transition of the FSM are specified by configuration in shared memory.

Archipelago library provides the following modules:

* `util`: various auxiliary utilies, such as linked lists, logging, shared memory interface, shared libraries interface, signal management interface;
* `app`: application plugin system (depends on `util`);
* `fsm`: finite state machine implementation (depends on `util`);
* `exe`: universal, multi-purpose executable configured via a shared memory (depends on `util`, `app`, `fsm`);
* `plugin`: built-in plugins providing most used types of resources (depends on `util`, `app`, `fsm`):
    - `files`: opening files;
    - `shared_memory`: attaching shared memory;
    - `shared_libraries`: loading shared libraries;
    - `threads`: creating threads for concurrent processing.

This application was originally designed as a part of [Rayway](https://github.com/ivanp7/rayway)
engine to implement a (dynamic) rendering pipeline that can be configured at runtime.

## Application plugin interface

A plugin is a shared library which implements some of the following functions,
which are provided via a special virtual table structure:

* `help(topic) -> status`: displays plugin help on the specified `topic`;
* `init(config) -> context`: creates and initializes new context according to `config`;
* `final(context)`: finalizes and destroys `context` previously created by `init()`;
* `set(context, slot, value) -> status`: accepts `value` for internal use by `context`, the meaning is specified by `slot`;
* `get(context, slot, value) -> status`: retrieves data provided by `context` for `slot` to a location pointed to by `value`;
* `act(context, action, config) -> status`: invokes an internal `action` for `context` according to `config`.

A plugin can provide multiple virtual tables.

This interface allows to separate resource consumers from producers, connecting them only via an application configuration.

## Application configuration

An application is configured by providing a special structure via a shared memory.
This structure specifies the list of plugins (shared libraries) to load,
and the list of initialization instructions. There are the following types of instructions:

* `init`: create a context and add it to the list of application contexts;
* `final`: destroy a context and remove it from the list of application contexts;
* `assign`: call a setter function and pass another context or output of a getter function;
* `act`: call an actor function.

## Finite state machine

### State

A state of a finite state machine is a pair of a function and a `void*` pointer to data
that will be provided to the function during the call.
State functions perform logically cohesive chunks of application logic.

During execution, a finite state machine maintains a stack of states.
State functions can push and pop states while doing state transition.

State function initiates state transition by returning or by calling `proceed()` function.
`proceed()` accepts the following parameters:

1. finite state machine context;
2. number of states to pop from the stack;
3. number of states to push from the stack;
4. array of states to push.

`proceed()` can be invoked not only from the state function itself, but from any nested call depth.
`proceed()` unwinds the call stack up to the state function,
eliminating the need of returning from multiple calls.

State function returns void and accepts a finite state machine context pointer (`fsm`).
State data pointer can be fetched from a finite state machine context.

### Transition

A transition (pair of a function and a `void*` pointer to data)
can be specified optionally for a finite state machine.

Transition is useful for debugging and profiling purposes.
It stays the same during the whole finite state machine execution
and cannot be changed, and is encapsulated from state functions.

Transition function is called every time the state changes,
before the entry state (previous state is null),
and after the exit state (next state is null).
Transition function can provide a translational state as output,
which is to be inserted before the next state.

A transition function returns void and accepts the following parameters:

* previous state (`prev_state`, input passed by value);
* next state (`next_state`, input passed by value);
* translational state (`trans_state`, output passed by pointer);
* status code (`code`, input/output passed by pointer);
* pointer to transition function data (`data`).

# Examples

Examples of plugins can be found in the `plugin` subdirectory.

Other projects using Archipelago:

* [still-alive](https://github.com/ivanp7/still-alive) -- built as a plugin.
* [port](https://github.com/ivanp7/port) -- provides plugin for execution of kernel functions.

# Documentation

Doxygen documentation is available at `docs` subdirectory. To build it, run `make`.

# How to build

0. change directory to the repository root;
1. execute `configure.sh` (providing optional environment variables) to generate `build.ninja`;
2. execute `ninja` to build.

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

