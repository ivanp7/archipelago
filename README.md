# Contents

* [Introduction](#introduction)
    * [The Conceptual Model](#the-conceptual-model)
    * [System Lifecycle](#system-lifecycle)
    * [System Architecture](#system-architecture)
* [Component Breakdown](#component-breakdown)
    * [Logging](#logging)
    * [Base](#base)
        * [Reference Counters](#reference-counters)
        * [Generic Pointers](#generic-pointers)
        * [Key-Value Lists](#key-value-lists)
        * [Error Propagation](#error-propagation)
    * [Built-In Modules](#built-in-modules)
        * [Contexts](#contexts)
            * [Contexts and Slots](#contexts-and-slots)
            * [Context Interfaces](#context-interfaces)
            * [Context Registry](#context-registry)
        * [Aggregate Objects](#aggregate-objects)
        * [Directed Execution Graphs](#directed-execution-graphs)
        * [Threads](#threads)
        * [Signal Management](#signal-management)
        * [Memory](#memory)
        * [Files](#files)
        * [Shared Libraries](#shared-libraries)
        * [Hashmaps](#hashmaps)
        * [Environment Variables](#environment-variables)
        * [Parsers](#parsers)
        * [Timers](#timers)
    * [Application](#application)
* [Included Plugins](#included-plugins)
    * [SDL](#sdl)
    * [OpenCL](#opencl)
* [Technical Implementation Details](#technical-implementation-details)
    * [Project Directory Structure](#project-directory-structure)
        * [Submodule Directory Naming Convention](#submodule-directory-naming-convention)
    * [Source File Naming Convention](#source-file-naming-convention)
    * [Generic Pointer Attribute Format](#generic-pointer-attribute-format)
        * [Constraints & Limits](#constraints-limits)
        * [Compatibility & Matching](#compatibility-matching)
    * [Generic Pointer Validness](#generic-pointer-validness)
    * [Memory-Mapped File Header for Pointer Support](#memory-mapped-file-header-for-pointer-support)
    * [Application Input File Header](#application-input-file-header)
* [Quick Start](#quick-start)
    * [Dependencies](#dependencies)
    * [Build Instructions](#build-instructions)
    * [Making Input Files Executable](#making-input-files-executable)
    * [Documentation](#documentation)
* [License](#license)

# Introduction

```
No AI was used during the development of this project (excluding proofreading of documentation).
```

**Archipelago** is a modular framework designed to decouple system initialization
from core application logic.

Originally developed for the rapid prototyping of OpenCL pipelines,
the framework allows developers to modify the sequence and composition of
computational steps -- and the resources they depend on -- without recompiling the application.
By utilizing a string-indexed registry of contexts driven by memory-mapped configuration files,
Archipelago enables a flexible, data-driven approach to component composition.

## The Conceptual Model

To understand how Archipelago operates, it is necessary to define
the relationship between its primary entities:

* **The Application**: The core binary that manages the process lifecycle,
    provides built-in resources, and interprets input configuration files.
* **Plugins**: External, dynamically loaded libraries
    that extend the system's functionality and provide custom resources.
* **Input Files**: Binary memory images provided as command-line arguments.
    These images contain the data structures and specific instructions
    used to manipulate the state of the process.
* **Contexts**: Opaque objects that encapsulate data or resources.
    Contexts are not isolated; they can interact directly by exchanging pointers,
    or indirectly through common resources.
* **The Context Registry**: A string-indexed dictionary of all existing contexts.
    This registry serves as the global state of the running process.

## System Lifecycle

The execution of an Archipelago application follows a sequence of steps:

### 0. Preparation

Before execution, memory images containing data and context registry instructions
are generated using external tools (a Python-based generator is provided in this repository).

### 1. Mapping

Upon launch, the application maps memory images specified on the command line
to fixed virtual addresses specified in their headers.
All images are mapped at the same time, therefore their memory ranges must not overlap.

### 2. Registry Operations, Initialization & Execution

The application processes the mapped memory images sequentially,
executing a series of registry operations.

During this stage, the "heavy lifting" of initialization occurs:
libraries are loaded, threads are spawned, external files are opened,
and other types of resources are initialized.
This is the primary stage where the application's logic is executed:
* **Lifecycle Management**: Contexts are created or deleted within the registry.
* **Resource Exchange**: Pointers to resources are exchanged between contexts.
* **Invocation**: Specific context actions are triggered.

### 3. Cleanup

Once the instructions within the memory images have been fully interpreted,
the application performs a cleanup phase, automatically destroying any remaining contexts
in the registry that were not explicitly deleted.

## System Architecture

Archipelago is designed as a layered dependency model.
Each layer provides a set of features that the layers above it consume,
ensuring a strict hierarchy of dependencies.

```
+-------------------+--------------------+
|    application    |      plugins       |
+===================+====================+ -\---\
| threads, memory, files, libraries, ... | m
+----------------------------------------+ o   l
|        directed execution graphs       | d   i
+----------------------------------------+ u   b
|             aggregate types            | l   r
+----------------------------------------+ e   a
|                contexts                | s   r
+========================+===============+ -/  y
|    base & utilities    |    logging    |
+------------------------+---------------+ -----/
```

### The Library

The foundation of the system consists of the logging subsystem and the library base & utilities.
The library base provides core features utilized throughout the system,
except for the logging subsystem which is an independent component.

Together, the foundational layer and the modules built upon it constitute
the **Archipelago Library** (`libarchi`).
These modules provide the essential functionality required by both
the **Archipelago application** and external plugins.

### Modular Design & Coupling

To maintain flexibility and minimize binary overhead,
the system employs a loosely coupled modular architecture:

* **Granular Dependency**: Each module is partitioned into submodules
    with explicit dependency requirements.
* **Conditional Compilation**: This granularity allows the build system
    to selectively exclude specific modules or submodules,
    optimizing the resulting binaries for different deployment targets.

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

## Built-In Modules

### Contexts

The context system is the central part of Archipelago.
The application is built around it, and the rest of the modules provide context interfaces
for the facilities they implement.

#### Contexts and Slots

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

#### Example: File Context

* **Public Data**: file stream.
* **Getters**: `fd` (returns file descriptor), `offset` (returns current offset).
* **Setters**: `offset` (sets offset from beginning), `offset.end` (sets offset from end),
    `offset.shift` (relative shift).
* **Calls**: `read` (reads from file), `write` (writes to file),
    `sync` (synchronizes file with storage).

#### Context Interfaces

A context type is defined by a *context interface*,
which is a structure containing pointers to the following lifecycle and operational functions:
* **Initialization**: Allocates and initializes public and private data of a context.
* **Finalization**: Finalizes and deallocates public and private data of a context.
* **Evaluation**: Implements *call* and *getter* slots.
* **Setter**: Implements *setter* slots.

Evaluation and setter functions can modify both public and private data,
allowing the set of available slots to change dynamically.
For example, a hashmap context provides getter/setter slots corresponding to
the keys of the values currently stored within it.

#### Context Registry

The context registry is a specialized hashmap context used to store
all existing contexts, including itself.

* **Lifecycle Management**: When a context is created, it is inserted into the registry.
    When destroyed, it is removed.
* **Reference Counting**: Contexts are reference-counted.
    When destruction is requested, the context is immediately removed from the registry,
    but actual deallocation is deferred until all references are released.

The application manages context lifecycles and invocations
through the following registry *operations*.

* Operations on the context registry:

| Category | Operation | Description | Context reference count |
| -------- | --------- | ----------- | ----------------------- |
| Deletion | `delete` | Removes a context. | Decremented |
| Insertion | `alias` | Creates an alternative key for an existing context. | Incremented |
| | `insert` | Inserts an existing context into the registry. | Incremented |
| Creation | `create` | Creates a new context using a specific *interface*. | Set to `1` |
| | `create_as` | Creates a new context using the *interface* of another. | Set to `1` |
| | `create_from` | Creates a context using an *interface* fetched from a slot. | Set to `1` |

* Operations on a context slot:

| Category | Operation | Description |
| -------- | --------- | ----------- |
| Invocation | `invoke` | Executes a context call. |
| Assignment | `unassign` | Unsets a specific context slot. |
| | `assign` | Directly assigns a value to a context slot. |
| | `assign_slot` | Assigns a value from one context slot to another. |
| | `assign_call` | Sets a context call invocation result to a context slot. |

### Aggregate Objects

The module implements a dynamic mechanism for accessing members of structures and unions.
Instead of using `.` or `->` operators, members are accessed via their names (represented as C-strings).

A specialized context interface provides a unified way of interacting with aggregate objects,
eliminating the need to create separate context interfaces for every individual structure or union type.

#### Aggregate Interfaces

An aggregate type (a structure, union, or an array of either) is described by an *aggregate interface*.
This is a structure containing the following fields:

* Values:
    - **Aggregate Type Metadata Tag**: The data tag of the metadata type accepted by the aggregate interface functions.
* Function Pointers:
    - **Tag**: Returns the data tag of the described aggregate type.
    - **Layout**: Returns the layout of the aggregate type (size, alignment, and the stride of the flexible array member).
    - **Number of References**: Returns the total number of reference-counted members.
    - **Reference Index**: Returns the index of a specified reference-counted member.
    - **Initialization**: Initializes an aggregate object.
    - **Finalization**: Finalizes an aggregate object.
    - **Getter**: Returns a generic pointer to a specified member.
    - **Setter**: Sets the value behind a generic pointer to a specified member.

Each *aggregate interface* has its own type of metadata accepted by its functions.

A *member specifier* is a structure containing:

* **Name**: A C-string representing the name of the member.
* **Index**: An index into the array. Non-array members are treated as arrays of length 1 (index always 0).
    Multidimensional arrays are treated as one-dimensional arrays.

Nested members are specified by a sequence of *member specifiers*, ordered from the top level down to the nested levels.

##### The Generic Aggregate Interface

The generic aggregate interface describes all types of structures
(including those with optional flexible array members), unions, and arrays thereof.

A description of the aggregate type is provided to the interface functions as metadata.

Aggregate members are categorized as follows:

* **Value**: A scalar type (non-pointer).
    Optionally described by a data tag.
* **Pointer**: A data or function pointer.
    Described by either a pointee type tag or a pointee type layout (size + alignment),
    depending on the pointer type.
* **Aggregate**: A nested aggregate type, described by an array of nested members.

Each aggregate member is described by:

- Size
- Alignment requirement
- Offset
- Number of elements (`1` for non-array members, `0` for flexible array members)
- Name

The top level of an aggregate type is additionally described by:

- Data tag
- Object initialization value
- Flexible array element initialization value (optional)

### Directed Execution Graphs

TODO

### Threads

TODO

### Signal Management

TODO

### Memory

TODO

### Files

TODO

### Shared Libraries

TODO

### Hashmaps

TODO

### Environment Variables

TODO

### Parsers

TODO

### Timers

TODO

## Application

TODO

# Included Plugins

## SDL

TODO

## OpenCL

TODO

# Technical Implementation Details

## Project Directory Structure

The project directory structure mirrors the system architecture,
with source and header files organized into the following directories:

* `archi_log`: The logging subsystem implementation.
* `archi_base`: The library base and utilities.
* `archi`: Implementation of library/plugin modules.
* `archi_app`: The Archipelago application implementation.

Both the core library and external plugins store their implemented modules within the `archi` directory.
Each subdirectory represents a distinct module, which may be composed of one or more submodules.

**Example Hierarchy**:

* `archi`
    * `archi/module1`
        * `archi/module1/submodule1`
        * `archi/module1/submodule2`
        * `archi/module1/submodule3`
    * `archi/module2`
        * `archi/module2/submodule1`
    * `archi/module3`
        * `archi/module3/submodule1`
        * `archi/module3/submodule2`

Submodule directory naming reflects the entities implemented within them
and their corresponding dependencies.

### Submodule Directory Naming Convention

**Module Implementation**: The `i` submodule contains the primary logic and functionality of the module.

**Auxiliary Submodules**: Other submodules extend the module implementation
by integrating facilities provided by other modules.
The Archipelago library utilizes the following names for auxiliary submodules:

- `ctx`: Context interfaces.
- `agg`: Aggregate type interfaces.
- `mem`: Memory interfaces.
- `exe`: Directed execution graph functions.
- `thr`: Threading functions.
- `sig`: Signal handling functions.

**Composite Dependencies**: When a submodule depends on multiple modules,
the tags are joined by hyphens (e.g., `sig-ctx`).

## Source File Naming Convention

C headers and source files follow the pattern `*.XXX.h` / `*.XXX.c`,
where `XXX` identifies the primary declaration type implemented in that file.
While a file may contain other types of declarations, the suffix reflects the file's main purpose:

- `def`: Constants and macros (`#define`) -- headers only.
- `typ`: Custom type definitions (`typedef`) -- headers only.
- `fun`: Function declarations (in headers), definitions (in sources).
- `var`: Global variable declarations (in headers), definitions (in sources).

## Generic Pointer Attribute Format

Pointer attributes are used to validate that the pointer target type is compatible with the required specification.
Compatibility is determined by matching the actual (tested) pointer attributes against the sample (required) attributes.

The attribute format is 64 bits wide (though the format is adaptable to other widths).

The **memory type** is determined by bits 63–62:

| Bits 63-62 | Memory Type                | Description |
| ---------- | -------------------------- | ----------- |
| `00`       | Short-lived, writable data | Stack memory (automatic storage duration) |
| `01`       | Long-lived, writable data  | Writable memory (e.g., heap); **may or may not be readable** |
| `10`       | Long-lived, read-only data | Read-only memory (e.g., memory mapping) |
| `11`       | Function                   | Executable memory |

> Readability for writable memory is unspecified, which mirrors standard C semantics.
> In C language, pointers to `const`-qualified types denote read-only memory (memory that shouldn't be written to),
> but there is no guarantee that pointers to non-`const` types denote memory that can be read.

### Function Pointers

If the memory type is `11`, bits 61–0 store a **function tag**, an unsigned integer ID identifying a function type.
Function tags are used for matching: unequal tags correspond to incompatible functions.

**Format**:

| Bit Range | Field Width | Field Value |
| --------- | ----------- | ----------- |
| (64; 62]  | 2           | Memory type (`11`) |
| (62; 0]   | 62          | Function tag |

### Data Pointers

If the type is `00`, `01`, or `10`, the interpretation of bits 61–0 depends on the value of bits 61-59:
- if the value is `111`, bits 61-0 encode a *complex data* type;
- otherwise, it's a *primitive data* type.

#### Complex Data

Opaque objects, structures with flexible array members, and other types not intended to form arrays or be copyable.

Bits 58-0 store a **data tag**.
Similar to function tags, data tags identify data types,
but the matching rules are more complex to support extension of data types (inheritance).
Data tags allow to encode base and derived types,
so that derived types are compatible with base types and not vice versa.

**Format**:

| Bit Range  | Field Width | Field Value |
| ---------- | ----------- | ----------- |
| (64; 62]   | 2           | Memory type (`00`, `01`, or `10`) |
| (62; 59]   | 3           | `111` |
| (59; 0]    | 59          | Data tag |

#### Primitive Data

Copyable types that can form arrays (scalars, vectors, or simple structures).

These are defined by length, stride (element size), and alignment requirement.
- Alignment must be a power of two.
- Stride must be positive.
- Stride must be divisible by alignment.

**Format**:

| Bit Range  | Field Width | Field Value |
| ---------- | ----------- | ----------- |
| (64; 62]   | 2           | Memory type (`00`, `01`, or `10`) |
| (62; 56]   | 6           | *N*: bit width of the next field |
| (56; 56-N] | N           | *q* = (stride / alignment) - 1 |
| (56-N; 0]  | 56-N        | Array length |

> **Note**: *N* is always < 56 because bits 61-59 are never `111` for primitive data.
> Therefore, the length field always contains at least one bit,
> unlike the stride/alignment field, which can be absent if *N* = 0.

* Alignment is calculated as `1 << M`, where *M* is the number of leading zeros
in the stride/alignment field (0 ≤ *M* ≤ *N*).
* Stride is calculated as `(q + 1) * alignment`.

**Byte Array Case**: When *N* = 0, alignment and stride are both `1`, and the format simplifies to:

| Bit Range  | Field Width | Field Value |
| ---------- | ----------- | ----------- |
| (64; 62]   | 2           | Memory type (`00`, `01`, or `10`) |
| (62; 56]   | 6           | *N* (`000000`) |
| (56; 0]    | 56          | Array length |

### Constraints & Limits

The attribute format is exhaustive; every representation has a unique, valid meaning.

For 64-bit attributes:
- Max primitive size (length×stride): 64 PiB minus 1 byte
- Max primitive stride & alignment: 32 PiB
- Number of distinct data tags: 576,460,752,303,423,487 (`0x07ffffffffffffff`)
- Number of distinct function tags: 4,611,686,018,427,387,903 (`0x3fffffffffffffff`)

### Compatibility & Matching

Function types and data types are mutually incompatible.
Primitive data types and complex data types are mutually incompatible
(except for the wildcard, which is compatible with any data type).

#### Matching Function Types

Function types match if their tags meet one of the following criteria:
* At least one of the tags is a wildcard.
* **OR** The tags are equal.

The function wildcard tag is defined as a tag with all bits clear (`0`).

#### Matching Data Types

Data types match if they meet one of the following criteria:
* At least one of the types is complex and its tag is a wildcard.
* **OR** Both types are primitive, and:
    - The strides are equal.
    - The alignment requirements are equal.
    - The length of the tested array is ≥ the sample array length.
* **OR** Both types are complex, and:
    - The tested tag is an *extension* of the sample tag.

> **Note**: memory types of both attributes are irrelevant for matching and ignored.

The data wildcard tag is defined as a tag with all bits clear (`0`).

Tag A is an *extension* of a pattern B, if:
1. A matches B in all bits of B.
2. Any differing bits (where A xor B has set bits) are located to the left of B's most significant bit
    and are separated from it by a gap of at least 4 zeros.

Examples:
* `100001101` is an extension of `1101` (Differing bit is at pos 8; MSB of B is at pos 3. There are 4 zeros between them).
* `110001101` is not an extension of `1101` (Differing bits are at pos 8 and 7; only 3 zeros separate the bit at pos 7 from the MSB of B).
* `1100` is not an extension of `1101` (The tags differ in the bits to the right of B's MSB).

### Examples

> Note: In the *Binary Attribute* column, `...` denotes omitted bits that follow the specified pattern.
>
> E.g., `0...0` means "all zeros", `1...1` means "all ones", and `(10)...(10)` means "alternating bits".

#### Functions

| Scenario    | Tag (hex)            | Tag (bin)         | Binary Attribute (64-bit) |
| ----------- | -------------------- | ----------------- | ------------------------- |
| zero tag    | `0x0`                | `000...000`       | `11 000...000` |
| small tag   | `0x38`               | `000...000111000` | `11 000...000111000` |
| large tag   | `0x2aaaaaaaaaaaaaaa` | `(10)...(10)`     | `11 (10)...(10)`  |
| maximum tag | `0x3fffffffffffffff` | `111...111`       | `11 111...111` |

#### Data

##### Complex Data

| Scenario    | Memory Type | Tag (hex)           | Tag (bin)            | Binary Attribute (64-bit) |
| ----------- | ----------- | ------------------- | -------------------- | ------------------------- |
| zero tag    | stack       | `0x0`               | `000...000`          | `00 111 000...000` |
| small tag   | writable    | `0x38`              | `000...000111000`    | `01 111 000...000111000` |
| large tag   | writable    | `0x566666666666666` | `101(0110)...(0110)` | `01 111 101(0110)...(0110)` |
| maximum tag | read-only   | `0x7ffffffffffffff` | `111...111`          | `10 111 111...111` |

##### Primitive Data

| Scenario           | Memory Type | Array Length      | Array Stride                            | Alignment Requirement | *M* (# bits) | *N* (# bits) | Binary Attribute (64-bit) |
| ------------------ | ----------- | ----------------- | --------------------------------------- | --------------------- | ------------ | ------------ | ------------------------- |
| all bits zero      | stack       | 0                 | 1 B ((0+1) * 1 bytes)                   | 1 B (1<<0 bytes)      | 0            | 0            | `00 000000  000...000` |
| byte array (N = 0) | stack       | 255               | 1 B ((0+1) * 1 bytes)                   | 1 B (1<<0 bytes)      | 0            | 0            | `00 000000  000...00011111111` |
| prime stride       | writable    | 13                | 7 B ((6+1) * 1 bytes)                   | 1 B (1<<0 bytes)      | 0            | 3            | `01 000011 110 000...0001101` |
| composite stride   | writable    | 25                | 832 B ((51+1) * 16 bytes)               | 16 B (1<<4 bytes)     | 4            | 10           | `01 001010 0000110011 000...00011001` |
| N = 1, M = 1       | writable    | 128               | 2 B ((0+1) * 2 bytes)                   | 2 B (1<<1 bytes)      | 1            | 1            | `01 000001 0 000...00010000000` |
| N = 1, M = 0       | writable    | 128               | 2 B ((1+1) * 1 bytes)                   | 1 B (1<<0 bytes)      | 0            | 1            | `01 000001 1 000...00010000000` |
| N = 56/2, M = N/2  | read-only   | 43                | 256 MiB ((16383+1) * (1<<14) bytes)     | 16 KiB (1<<14 bytes)  | 14           | 28           | `10 011100 000...000111...111 000...000101011` |
| maximum N, M = N   | read-only   | 1                 | 32 PiB ((0+1) * (1<<55) bytes)          | 32 PiB (1<<55 bytes)  | 55           | 55           | `10 110111 000...000 1` |
| maximum N, M = 0   | read-only   | 1                 | 32 PiB ((0x7fffffffffffff+1) * 1 bytes) | 1 B (1<<0 bytes)      | 0            | 55           | `10 110111 111...111 1` |
| maximum data size  | read-only   | (1<<56) - 1       | 1 B ((0+1) * 1 bytes)                   | 1 B (1<<0 bytes)      | 0            | 0            | `10 000000  111...111` |

## Generic Pointer Validness

Generic pointer is valid if it meets one of the following criteria:

1. The target is function or complex data type.
2. Pointer is non-null and aligned to the requirement specified in the attributes.
3. Pointer is null and the primitive data length specified in the attributes is zero.

## Memory-Mapped File Header for Pointer Support

Memory-mapped files can begin with a special header that enables the use of pointers within the file.
This header contains the following fields:

1. **Required Mapping Address**: The specific memory address where the header (and the subsequent file) must be located.
2. **Payload Size**: The size of the file in bytes, excluding the header.

Mapping a file to its specified address ensures that the memory region is predictable
and that all internal pointers remain valid.

When pointer support is enabled, the file is processed in two steps:

1. **Probe**: The header is mapped to a temporary address to retrieve the required mapping address and the file size.
    The remainder of the file is not yet mapped.
2. **Placement**: The temporary mapping is removed, and the entire file is mapped at the required address.

## Application Input File Header

Every input file provided to the application must begin with a header containing the following fields:

1. **Memory-Mapped File Header**: As described in the [Memory-Mapped File Header for Pointer Support](#memory-mapped-file-header-for-pointer-support) section.
2. **Magic Bytes**: An 8-byte C-string `[archi]` (including the null-terminator).
3. **Contents Pointer**: A pointer to a key-value list of the file's contents (non-reference-counted).
    Keys within this list designate the types of values; duplicate keys are permitted.

# Quick Start

## Dependencies

* [glibc](https://www.gnu.org/software/libc/) -- POSIX; argument parsing with [argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html).

## Build Instructions

The library, the application, and the included plugins are built using the [Ninja](https://ninja-build.org/) build system.

To generate `build.ninja` files, use `configure.py` scripts located in the repository root directory and plugin subdirectories.
The `configure.py` scripts may be modified for customization (e.g. to change the `MODULES` array listing the built modules, the compilation flags, the output file names, etc).
The behavior of the scripts are also controlled by several accepted environment variables (search for `INPUT ENVIRONMENT VARIABLE` to find them).

## Making Input Files Executable

Archipelago memory image files can be made directly executable on Linux systems.
This allows you to run `.archi` files directly, without explicitly calling the Archipelago application.

To register the file format with the kernel via `binfmt_misc`, run `register-binfmt.sh` as root.
The executable (`archi`) must be installed in a system directory for binaries (listed in your `PATH`).

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

