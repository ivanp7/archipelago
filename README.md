# Contents

* [About Archipelago](#about-archipelago)
    * [The Lifecycle](#the-lifecycle)
* [System Architecture](#system-architecture)
* [Component Breakdown](#component-breakdown)
    * [Logging](#logging)
    * [Base](#base)
        * [Reference Counters](#reference-counters)
        * [Generic Pointers](#generic-pointers)
        * [Key-Value Lists](#key-value-lists)
        * [Error Propagation](#error-propagation)
    * [Module: Contexts](#module-contexts)
        * [Contexts and Slots](#contexts-and-slots)
        * [Context Interfaces](#context-interfaces)
        * [Context Registry](#context-registry)
    * [Module: Aggregate Objects](#aggregate-objects)
    * [Module: Directed Execution Graphs](#directed-execution-graphs)
    * [Module: Threads](#threads)
    * [Module: Signal Management](#signal-management)
    * [Module: Memory](#memory)
    * [Module: Files](#files)
    * [Module: Shared Libraries](#shared-libraries)
    * [Module: Hashmaps](#hashmaps)
    * [Module: Environment Variables](#environment-variables)
    * [Module: Parsers](#parsers)
    * [Module: Timers](#timers)
    * [Application](#application)
* [Included Plugins](#included-plugins)
    * [SDL](#sdl)
    * [OpenCL](#opencl)
* [Technical Implementation Details](#technical-implementation-details)
    * [Generic Pointer Attribute Format](#generic-pointer-attribute-format)
        * [Constraints & Limits](#constraints-limits)
        * [Compatibility & Matching](#compatibility-matching)
    * [Generic Pointer Validness](#generic-pointer-validness)
* [Quick Start](#quick-start)
    * [Dependencies](#dependencies)
    * [Build Instructions](#build-instructions)
    * [Documentation](#documentation)
* [License](#license)

# About Archipelago

**No AI used during development of the project** (not counting use of AI for proofreading documentation comments).

**Archipelago** is a modular framework designed to decouple initialization from computational logic.
Originally developed for rapid OpenCL pipeline prototyping -- specifically to allow modifications to the pipeline without recompiling the application -- it enables developers to construct applications from separate components linked via memory-mapped configuration.
By managing a string-indexed registry of contexts based on instructions within input files, Archipelago allows for flexible, data-driven component composition.

The core concepts of the system:

* **Executable / Application**: The binary implemented in this repository.
    It provides *built-in* resources and interprets *input files* to manage the *process*.
* **Built-in**: Resources or functionality provided natively by the *executable*.
* **Plugins**: External, dynamically loaded libraries that provide custom functionality and resources.
* **Input Files**: Binary memory images specified as command-line arguments.
    Each image contains data structures (pointers are supported) and instructions used to modify the *context registry* and thus the *process* state.

* **Process**: A running instance of the *application*.
* **Contexts**: Opaque objects that encapsulate data structures or resources.
    Contexts are not isolated; they can obtain access to other contexts.
* **Context Registry**: A string-indexed dictionary of *contexts* maintained by the *application*.
    Context registry serves as the global state of the *process*.

## The Lifecycle

Input memory images are prepared using external tools prior to execution.
These tools can be implemented in any language; a Python-based generator is included in this repository.

The application is launched with all required memory images specified on the command line.
The order of these files is critical, as images are processed sequentially.

1. **Mapping**: Images are mapped to the fixed virtual addresses specified in their headers.
    Memory ranges must not overlap; otherwise, an error is triggered.

2. **Registry Operations**: The application reads the lists of registry operations in the mapped memory and executes them in the specified order.
    There are multiple operation types supported:
    - Contexts are created or deleted, updating the context registry.
    - Pointers are exchanged between contexts (contexts can provide and/or accept pointers, based on their respective interface specifications).
    - Context actions are invoked (contexts can support additional functionality, based on their respective interface specifications).

    The primary computation logic occurs during this stage: libraries are loaded, files are opened, threads are spawned, functions are called, etc.

3. **Finalization**: The application cleans up any remaining contexts that were not explicitly deleted from the registry.

# System Architecture

The system architecture consists of several stacked layers.

The application and external plugins are implemented using the core library, `libarchi`, but remain independent of one another.

The library is composed of loosely coupled modules built upon a common base.
The logging subsystem is available as a separate component.
The modules provide functionality used either by the application or as general-purpose utilities.

```
+-------------------+--------------------+
|    application    |      plugins       |
+-------------------+--------------------+ -\---\
| threads, memory, files, libraries, ... | m
+----------------------------------------+ o   l
|        directed execution graphs       | d   i
+----------------------------------------+ u   b
|             aggregate types            | l   r
+----------------------------------------+ e   a
|                contexts                | s   r
+------------------------+---------------+ -/  y
|    base & utilities    |    logging    |
+------------------------+---------------+ -----/
```

Each module is divided into submodules based on their dependencies.
Submodules follow a standardized naming convention based on the entities they implement:
- `i`: Module functionality implementation.
- `ctx`: Context interfaces (dependency: `context` module).
- `agg`: Aggregate type interfaces (dependency: `aggr` module).
- `mem`: Memory interfaces (dependency: `memory` module).
- `exe`: Directed execution graph functions (dependency: `exec` module).
- `thr`: Threading functions (dependency: `thread` module).
- `sig`: Signal handling functions (dependency: `signal` module).

Submodule names can be a combination of the above, joined by the hyphen `-` (for example, `sig-ctx`).

# Component Breakdown

## Logging

A standalone, singleton logging component that outputs to a configurable stream.

### Filtering

Messages are categorized by severity, which determines their output color:
1. *Error*
2. *Warning*
3. *Notice*
4. *Info*
5. *Debug*

Messages are only printed if their verbosity level is less than or equal to the global logger threshold,
which is configurable via a command-line option.

### Message Format

```
H:MM:SS.mmm.uuu [LVL] (origin) text
```

* `H:MM:SS.mmm.uuu`: Timestamp (hours, minutes, seconds, milliseconds, microseconds).
* `LVL`: Verbosity level abbreviation (ERR, WRN, NTC, INF, or DBG).
* `origin`: Optional source of the message (typically the calling function).
* `text`: The log message content.

## Base

The library base provides the foundational data structures and utilities upon which the rest of the system is built.

### Reference Counters

A reference counter is an opaque object comprising a value, a destructor function, and its associated data.

* **Initialization**: Created with an initial value of `1`.
* **Lifecycle**: The value is incremented or decremented as needed.
    When the value reaches `0`, the destructor is invoked and the counter is deallocated.
* **Manual Destruction**: Counters may be deallocated manually without triggering the destructor.

### Generic Pointers

Generic pointers provide a unified way to store arbitrary data or function pointers,
utilizing an **attribute field** to describe the pointee (the attribute type is unsigned integer).

There are two variants:
1. **Basic**: Stores the pointer and its attributes.
2. **Reference-Counted**: Includes an additional field for the pointee's reference counter.

A reference-counted generic pointer is called *weak* if the pointer to a reference counter is null.

### Key-Value Lists

A key-value list is a single-linked list of nodes comprising of a C-string key and a generic pointer (both variants).

Generally, key-value lists are used for passing a list of "parameters" to functions
that are part of an interface (e.g., context initialization function).
Key-value lists can be stored in memory-mapped input files as-is.

### Error Propagation

The system uses a dedicated error structure to provide detailed diagnostic information:

- **Error Code**: An integer where 0 indicates success.
    Negative values are reserved for the core application and standard plugins;
    positive values are used for user-defined errors.
- **Error Message**: A human-readable C-string describing the failure.
- **Location**: The source file name, function name, and line number where the error was triggered.

To avoid the pitfalls of global state (like `errno`), Archipelago propagates errors
via a specific output parameter in supporting functions.

The error object should be initialized to the "unspecified error" code before being passed to a function.
The function is then responsible for updating that object to either "success" or a specific error code.
A set of helper macros is provided to simplify this process.

If an error with non-zero error code gets returned to the application, it is printed to the log.

## Module: Contexts

The context system is the central part of Archipelago.
The application is built around it, and the rest of the modules provide context interfaces
for the facilities they implement.

### Contexts and Slots

A *context* is an opaque object encapsulating a data structure, a resource,
or any entity that fits into the context model.
Encapsulated data is split into *public* and *private* parts.
*Public data* (or simply *context data*) can be fetched and used externally.
*Private data* is only available internally, to the *context interface* functions.

Contexts expose their functionality, components, and information through *context slots*.
A *context slot* is a structure consisting of:
- *slot name*: A C-string. While these can be arbitrary or empty,
they typically follow the format of C identifiers
separated by dots (e.g., `seq_node.array.element`).
- *slot indices*: An array of signed integers, can be empty.

The *empty slot* (a slot with an empty name and no indices)
specifically references the *context data* itself,
rather than a sub-resource provided via a slot.

There are different kinds of slots:
* *getter slots*: context outputs that return a generic pointer.
* *setter slots*: context inputs that accept a generic pointer.
* *call slots* (or *calls*): internal actions of a context,
    accept a key-value list of parameters and optionally return a generic pointer.

Certain setter slots may be unset; this operation does not require a value.

Getters and setters behave similarly to object properties in OOP languages.
Calls behave similarly to object methods in OOP languages.

The empty slot has special properties:
- The empty getter slot always returns the context data, this cannot be redefined.
- The empty slot cannot be a set or unset (the empty setter slot can't exist).
- The empty call slot can be defined.

### Example: File Context

* **Public Data**: file stream.
* **Getters**: `fd` (returns file descriptor), `offset` (returns current offset).
* **Setters**: `offset` (sets offset from beginning), `offset.end` (sets offset from end),
    `offset.shift` (relative shift).
* **Calls**: `read` (reads from file), `write` (writes to file),
    `sync` (synchronizes file with storage).

### Context Interfaces

A context type is defined by a *context interface*,
which contains pointers to the following lifecycle and operational functions:
* **Initialization**: Allocates and initializes public and private data of a context.
* **Finalization**: Finalizes and deallocates public and private data of a context.
* **Evaluation**: Implements *call* and *getter* slots.
* **Setter**: Implements *setter* slots.

Evaluation and setter functions can modify both public and private data,
allowing the set of available slots to change dynamically.
For example, a hashmap context provides getter/setter slots corresponding to
the keys of the values currently stored within it.

### Context Registry

The context registry is a specialized hashmap context used to store
all existing contexts, including itself.

* **Lifecycle Management**: When a context is created, it is inserted into the registry.
    When destroyed, it is removed.
* **Reference Counting**: Contexts are reference-counted.
    When destruction is requested, the context is immediately removed from the registry,
    but actual deallocation is deferred until all references are released.

The application manages context lifecycles and invocations
through the following registry *operations*:

| Category | Operation | Description | Context reference count |
| -------- | --------- | ----------- | ----------------------- |
| Deletion | `delete` | Removes a context. | Decremented |
| Creation/insertion | `alias` | Creates an alternative key for an existing context. | Incremented |
| | `insert` | Inserts an existing context into the registry. | Incremented |
| | `create` | Creates a new context using a specific *interface*. | Set to `1` |
| | `create_as` | Creates a new context using the *interface* of another. | Set to `1` |
| | `create_from` | Creates a context using an *interface* fetched from a slot. | Set to `1` |
| Invocation | `invoke` | Executes a context call. | Unchanged |
| Assignment | `unassign` | Unsets a specific context slot. | Unchanged |
| | `assign` | Directly assigns a value to a slot. | Unchanged |
| | `assign_slot` | Assigns a value from one slot/context to another. | Unchanged |
| | `assign_call` | Sets a value returned by a call to a context slot. | Unchanged |

## Module: Aggregate Objects

TODO

## Module: Directed Execution Graphs

TODO

## Module: Threads

TODO

## Module: Signal Management

TODO

## Module: Memory

TODO

## Module: Files

TODO

## Module: Shared Libraries

TODO

## Module: Hashmaps

TODO

## Module: Environment Variables

TODO

## Module: Parsers

TODO

## Module: Timers

TODO

## Application

TODO

# Included Plugins

## SDL

TODO

## OpenCL

TODO

# Technical Implementation Details

## Generic Pointer Attribute Format

The attribute field is 64 bits wide (though the format is adaptable to other widths).

The **Memory Type** is determined by the two most significant bits (63–62):

| Bits 63-62 | Memory Type                | Description |
| :--------: | -------------------------- | ----------- |
| `00`       | Short-lived, writable data | Stack memory (automatic storage duration) |
| `01`       | Long-lived, writable data  | Memory that can be written to (e.g., heap); may or may not be readable |
| `10`       | Long-lived, read-only data | Memory that can be read only (e.g., memory mapping) |
| `11`       | Function                   | Executable memory |

1. **Function Pointers**

    If the type is `11`, bits 62–0 store a function tag (unsigned integer ID).
    - Unequal tags indicate incompatible function types.
    - A tag of `0` acts as a wildcard, matching any tag and indicating an unspecified function type.

    The format is: `[11][function tag (62 bits)]`.

2. **Data Pointers**

    If the type is `00`, `01`, or `10`, the interpretation of bits 62–0 depends on the **Data Kind**
    (specified by bits 61–56, denoted as *N*, which define the split of bits 55-0 into two parts):

    * **Primitive Data** (0 ≤ *N* < 56):

        Used for copyable types that can form arrays (scalars, vectors, or simple structures).
        These are defined by length, stride (element size), and alignment (must be a power of two, and stride must be divisible by alignment).

        The format is: `[mem.type (2 bits)][N (6 bits)][stride / alignment - 1 (N bits)][length (56-N bits)]`.

        The alignment is calculated as `1 << M`, where *M* is the number of leading zeros in the stride/alignment field.

        * In case when *N* = 0, the format simplifies to: `[mem.type (2 bits)][000000][length (56 bits)]`.
            The alignment and the stride are both `1`.

    * **Complex Data** (*N* ≥ 56):

        Used for opaque objects, structures with flexible array members, and other types not intended to form arrays or be copyable.

        The format is: `[mem.type (2 bits)][data tag, bitwise inverted (62 bits)]`.
        Similar to function tags, these IDs identify the data type; a `0` tag is a wildcard.

### Constraints & Limits

The attribute format is exhaustive; every bit combination has a unique, valid meaning.

For 64-bit attributes:
- Max Primitive Size (length×stride): 64 PiB minus 1 byte
- Max Primitive Stride/Alignment: 32 PiB
- Max Data Tag: `0x07ffffffffffffff` (576,460,752,303,423,487)
- Max Function Tag: `0x3fffffffffffffff` (4,611,686,018,427,387,903)

### Compatibility & Matching

Pointer attributes are used to validate that a pointee matches a required specification.
Compatibility is determined by comparing the actual pointer attributes against the sample attributes.

Pointers are compatible if they meet one of the following criteria:

1. **Function Match**: both pointers are function types:
    * Function tags are equal, or at least one of them is `0` (function wildcard).

2. **Data Match**: both pointers are data types:
    * At least one pointer refers to complex data with a tag of `0` (primitive/complex data wildcard).
    * Both pointers are complex data types and their tags are equal.
    * Both pointers are primitive data types, provided that:
        - The strides are equal.
        - The alignment requirements are equal.
        - The length of the tested data is ≥ the length of the sample data.

## Generic Pointer Validness

Generic pointer is valid if it meets one of the following criteria:

1. Pointer is function or complex data type.
2. Pointer is non-null and aligned to the requirement specified in the attributes.
3. Pointer is null and the primitive data length specified in the attributes is zero.

# Quick Start

## Dependencies

* [glibc](https://www.gnu.org/software/libc/) -- POSIX; argument parsing with [argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html).

## Build Instructions

The library, the application, and the included plugins are built using the [Ninja](https://ninja-build.org/) build system.
To generate `build.ninja` files, use `configure.py` scripts located in the repository root directory and plugin subdirectories.
The `configure.py` scripts may be modified for customization (e.g. to change the `MODULES` array listing the built modules, the compilation flags, the output file names, etc).
The behavior of the scripts are also controlled by several accepted environment variables (search for `INPUT ENVIRONMENT VARIABLE` to find them).

## Documentation

Doxygen documentation is available. To build it, go to the `doxygen` subdirectory and run `make`.

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

