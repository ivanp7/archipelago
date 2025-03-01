 #############################################################################
 # Copyright (C) 2023-2025 by Ivan Podmazov                                  #
 #                                                                           #
 # This file is part of Archipelago.                                         #
 #                                                                           #
 #   Archipelago is free software: you can redistribute it and/or modify it  #
 #   under the terms of the GNU Lesser General Public License as published   #
 #   by the Free Software Foundation, either version 3 of the License, or    #
 #   (at your option) any later version.                                     #
 #                                                                           #
 #   Archipelago is distributed in the hope that it will be useful,          #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of          #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
 #   GNU Lesser General Public License for more details.                     #
 #                                                                           #
 #   You should have received a copy of the GNU Lesser General Public        #
 #   License along with Archipelago. If not, see                             #
 #   <http://www.gnu.org/licenses/>.                                         #
 #############################################################################

# @file
# @brief Python module: Archipelago application configuration generator.

import ctypes as c
import mmap
import os
import signal

###############################################################################

VALUE_NULL      = 0  # No value.
VALUE_FALSE     = 1  # Falsey boolean value.
VALUE_TRUE      = 2  # Truthy boolean value.
VALUE_UINT      = 3  # Unsigned integer.
VALUE_SINT      = 4  # Signed integer.
VALUE_FLOAT     = 5  # Floating-point number.
VALUE_STRING    = 6  # Null-terminated string.
VALUE_DATA      = 7  # Binary data.
VALUE_NESTED    = 8  # Nested node.
VALUE_LIST      = 9  # Nested list.
VALUE_FUNCTION  = 10 # Pointer to a function.


class Value:
    """High-level representation of a value_t object.
    """
    def __init__(self, object, size=None, num_of=1, type=VALUE_DATA):
        """Create a value wrapper.

        @param[in] object : a ctypes object
        @param[in] size   : size of the object (if None, ctypes.sizeof() is used)
        @param[in] num_of : number of array elements (1 if the object is singular)
        @param[in] type   : type ID of the object (one of VALUE_* constants)
        """
        if (size is not None and size < 0) or num_of < 0:
            ValueError(f'incorrect size ({size}) or number ({num_of})')

        self._object_ = object

        if object:
            self._size_ = size if size else c.sizeof(object)
            self._num_of_ = num_of
            self._type_ = type
        else:
            self._size_ = 0
            self._num_of_ = 0
            self._type_ = VALUE_NULL

###############################################################################

class ApplicationContextField:
    """Representation of a context slot in the application configuration.
    """
    def __init__(self, context, name: str):
        """Create a context field representation object.
        """
        if not isinstance(context, ApplicationContext):
            raise TypeError("expected ApplicationContext")

        self._context_ = context
        self._name_ = name


    def __call__(self, **params):
        """Invoke a context action.
        """
        self._context_._app_._steps_.append({'type': 'act',
                                             'key': self._context_._key_,
                                             'action': self._name_,
                                             'params': params})


class ApplicationContext:
    """Representation of a context in the application configuration.
    """
    def __init__(self, app, key: str, interface_key: str=None, **config):
        """Create a context.
        """
        if not isinstance(app, Application):
            raise TypeError("expected Application")

        self._app_ = app
        self._key_ = key
        self._interface_key_ = interface_key
        self._existing_ = interface_key is None

        if not self._existing_:
            app._steps_.append({'type': 'init',
                                'key': self._key_,
                                'interface_key': interface_key,
                                'config': config})


    def __del__(self):
        """Destroy a context.
        """
        if not self._existing_:
            self._app_._steps_.append({'type': 'final',
                                       'key': self._key_})


    def __getattr__(self, name):
        """Get a context slot or a context action.
        """
        if name[0] == '_':
            return self.__dict__[name]

        return ApplicationContextField(self, name)


    def __setattr__(self, name, value):
        """Set a context slot.
        """
        if name[0] == '_':
            self.__dict__[name] = value
            return

        if isinstance(value, ApplicationContext):
            self._app_._steps_.append({'type': 'assign',
                                       'key': self._key_,
                                       'slot': name,
                                       'source_key': value._key_})
        elif isinstance(value, ApplicationContextField):
            self._app_._steps_.append({'type': 'assign',
                                       'key': self._key_,
                                       'slot': name,
                                       'source_key': value._context_._key_,
                                       'source_slot': value._name_})
        elif isinstance(value, Value):
            self._app_._steps_.append({'type': 'set',
                                       'key': self._key_,
                                       'slot': name,
                                       'value': value})
        else:
            raise TypeError(f'expected ApplicationContext, ApplicationContextField, or Value')


NUM_RT_SIGNALS = signal.SIGRTMAX - signal.SIGRTMIN + 1

class Application:
    """Constructor of Archipelago application configurations.
    """
    def __init__(self):
        """Initialize an application configuration instance.
        """
        self._signals_ = set()
        self._libraries_ = {}
        self._interfaces_ = {}
        self._steps_ = []


    def watch_signal(self, signame: str, n: int=0):
        """Add a signal to the watch set.

        @param[in] signame : signal name (such as 'SIGINT', 'SIGTERM', 'SIGRTMIN', etc)
        @param[in] n       : signal number (for real-time signals SIGRTMIN+n and SIGRTMAX-n)
        """
        if not signame:
            raise ValueError("null signal name")
        elif signame[0:3] != 'SIG':
            raise ValueError(f"invalid signal name '{signame}'")
        elif n < 0:
            raise ValueError(f"invalid signal number '{n}'")
        elif signame == 'SIGRTMIN' and n > signal.SIGRTMAX - signal.SIGRTMIN:
            raise ValueError(f"invalid signal SIGRTMIN+{n}")
        elif signame == 'SIGRTMAX' and n > signal.SIGRTMAX - signal.SIGRTMIN:
            raise ValueError(f"invalid signal SIGRTMAX-{n}")

        self._signals_.add((signame, n))


    def load_library(self, key: str, pathname: str,
                     flag_lazy: bool=False, flag_global: bool=False, flags: int=0):
        """Load a shared library and add it to the application.

        @param[in] key         : alias of the loaded library
        @param[in] pathname    : pathname of the library file
        @param[in] flag_lazy   : whether is lazy binding performed
        @param[in] flag_global : whether are the symbols made available to subsequent loaded libraries
        @param[in] flags       : other flags for dlopen()
        """
        if not key:
            raise ValueError("null key")
        elif key in self._libraries_:
            raise KeyError("library '{key}' is added already")

        self._libraries_[key] = {'pathname': pathname,
                                 'lazy': flag_lazy,
                                 'global': flag_global,
                                 'flags': flags}


    def register_interface(self, key: str, symbol_name: str, library_key: str):
        """Get a context interface symbol and add it to the application.

        @param[in] key         : alias of the context interface
        @param[in] symbol_name : name of the symbol defining the context interface
        @param[in] library_key : alias of the library from which the symbol is obtained
        """
        if not key:
            raise ValueError("null key")
        elif not symbol_name:
            raise ValueError("null symbol name")
        elif key in self._interfaces_:
            raise KeyError("interface '{key}' has been added already")
        elif library_key not in self._libraries_:
            raise KeyError(f"unknown library '{library_key}'")

        self._interfaces_[key] = {'symbol_name': symbol_name,
                                  'library_key': library_key}


    def new_context(self, key: str, interface_key: str, **config) -> ApplicationContext:
        """Return a new context.

        @param[in] key           : alias of the created context
        @param[in] interface_key : alias of the context interface to use for the created context
        @param[in] **config      : configuration of the created context
        """
        return ApplicationContext(self, key, interface_key, **config)


    def context(self, key: str) -> ApplicationContext:
        """Return an existing context.

        @param[in] key : alias of the existing context
        """
        return ApplicationContext(self, key)


APP_SIGNAL_CONTEXT_KEY = "archi_app_signal"
APP_FSM_CONTEXT_KEY = "archi_app_fsm"

###############################################################################
###############################################################################

def fossilize(app: Application, pathname: str):
    """Create a memory-mapped file and write the configuration into it.

    @param[in] app      : an application object
    @param[in] pathname : pathname of the created memory-mapped configuration file
    """
    if not isinstance(app, Application):
        raise TypeError("expected Application")

    address = None    # address of the mapped memory
    total_size = 0    # total size of the configuration
    total_padding = 0 # total number of padding bytes used

    def allocate(ctype): # allocate a new object
        nonlocal total_size
        nonlocal total_padding

        size = c.sizeof(ctype)
        alignment = c.alignment(ctype)

        # fulfill the alignment requirement
        new_total_size = (total_size + (alignment - 1)) & ~(alignment - 1)
        total_padding += new_total_size - total_size
        total_size = new_total_size

        object = ctype.from_address(address + total_size) if address else None
        total_size += size

        return object


    def allocate_object(obj): # allocate an object copy
        object = allocate(type(obj))
        if address:
            try:
                c.memmove(object, obj, c.sizeof(obj)) # buffer-based object
            except:
                object.value = obj.value # value-based object

        return object


    all_strings = None # set of strings

    def collect_strings(): # collect all strings of the configuration
        nonlocal all_strings

        all_strings = set()

        all_strings.update(app._libraries_.keys())
        all_strings.update(value['pathname'] for value in app._libraries_.values())

        all_strings.update(app._interfaces_.keys())
        all_strings.update(value['symbol_name'] for value in app._interfaces_.values())
        all_strings.update(value['library_key'] for value in app._interfaces_.values())

        all_strings.update(step['key'] for step in app._steps_)

        for step in app._steps_:
            if step['type'] == 'init':
                all_strings.add(step['interface_key'])
                all_strings.update(step['config'].keys())
            elif step['type'] == 'set':
                all_strings.add(step['slot'])
            elif step['type'] == 'assign':
                all_strings.add(step['slot'])
                all_strings.add(step['source_key'])
                if 'source_slot' in step:
                    all_strings.add(step['source_slot'])
            elif step['type'] == 'act':
                all_strings.add(step['action'])
                all_strings.update(step['params'].keys())


    string_objects = {} # dictionary of string objects

    def allocate_string(bstr): # allocate a string copy
        if bstr in string_objects:
            return string_objects[bstr]

        object = allocate_object(c.create_string_buffer(bstr.encode()))
        string_objects[bstr] = object

        return object


    def allocate_value(v): # allocate a value wrapper
        if not isinstance(v, Value):
            raise TypeError("expected Value")

        value = allocate(value_t)
        object = allocate_object(v._object_) if v._object_ else None

        if address:
            if object:
                value.ptr = c.addressof(object)
                value.size = v._size_
                value.num_of = v._num_of_
            value.type = v._type_

        return value


    def allocate_named_value_list(values): # allocate a list of nodes with names and values
        if len(values) == 0:
            return None

        nodes = allocate(list_node_named_value_t * len(values))

        for i, (key, value) in enumerate(values.items()):
            if not isinstance(value, Value):
                raise TypeError("expected Value")

            name = allocate_string(key)
            object = allocate_object(value._object_) if value._object_ else None

            if address:
                if i > 0:
                    nodes[i].base.link.prev = c.cast(c.pointer(nodes[i - 1]), c.POINTER(list_node_t))
                if i < len(values) - 1:
                    nodes[i].base.link.next = c.cast(c.pointer(nodes[i + 1]), c.POINTER(list_node_t))

                nodes[i].base.name = c.addressof(name)

                if object:
                    nodes[i].value.ptr = c.addressof(object)
                    nodes[i].value.size = value._size_
                    nodes[i].value.num_of = value._num_of_
                nodes[i].value.type = value._type_

        return nodes


    def construct_configuration(): # construct the whole configuration
        nonlocal total_size
        nonlocal total_padding
        nonlocal string_objects

        total_size = 0
        total_padding = 0
        string_objects = {}

        # Allocate the main structure
        shm_config = allocate(configuration_t)

        # Preallocate all strings at once
        for s in all_strings:
            allocate_string(s)

        # Allocate the signal watch set
        if len(app._signals_) > 0:
            signal_watch_set = allocate(signal_watch_set_t)
            if address:
                shm_config.signal_watch_set = c.pointer(signal_watch_set)

        if address:
            for signame, n in app._signals_:
                if signame == 'SIGRTMIN':
                    signal_watch_set.f_SIGRTMIN[n] = True
                elif signame == 'SIGRTMAX':
                    signal_watch_set.f_SIGRTMIN[(signal.SIGRTMAX - signal.SIGRTMIN) - n] = True
                else:
                    setattr(signal_watch_set, f'f_{signame}', True)

        # Allocate the configuration arrays
        if len(app._libraries_) > 0:
            libraries = allocate(app_loader_library_t * len(app._libraries_))
            if address:
                shm_config.app_config.libraries = c.pointer(libraries[0])

        if len(app._interfaces_) > 0:
            interfaces = allocate(app_loader_library_symbol_t * len(app._interfaces_))
            if address:
                shm_config.app_config.interfaces = c.pointer(interfaces[0])

        if len(app._steps_):
            steps = allocate(app_config_step_t * len(app._steps_))
            if address:
                shm_config.app_config.steps = c.pointer(steps[0])

        if address:
            shm_config.app_config.num_libraries = len(app._libraries_)
            shm_config.app_config.num_interfaces = len(app._interfaces_)
            shm_config.app_config.num_steps = len(app._steps_)

        # Construct the configuration of libraries
        for i, (key, value) in enumerate(app._libraries_.items()):
            library_key = allocate_string(key)
            pathname = allocate_string(value['pathname'])

            if address:
                libraries[i].key = c.addressof(library_key)
                libraries[i].param.pathname = c.addressof(pathname)
                libraries[i].param.lazy = value['lazy']
                setattr(libraries[i].param, 'global', value['global'])
                libraries[i].param.flags = value['flags']

        # Construct the configuration of interfaces
        for i, (key, value) in enumerate(app._interfaces_.items()):
            interface_key = allocate_string(key)
            symbol_name = allocate_string(value['symbol_name'])
            library_key = allocate_string(value['library_key'])

            if address:
                interfaces[i].key = c.addressof(interface_key)
                interfaces[i].symbol_name = c.addressof(symbol_name)
                interfaces[i].library_key = c.addressof(library_key)

        # Construct the configuration steps
        for i, step in enumerate(app._steps_):
            context_key = allocate_string(step['key'])

            if step['type'] == 'init':
                interface_key = allocate_string(step['interface_key'])
                config = allocate_named_value_list(step['config'])

                if address:
                    steps[i].type = APP_CONFIG_STEP_INIT
                    steps[i].key = c.addressof(context_key)
                    steps[i].as_init.interface_key = c.addressof(interface_key)
                    if config:
                        steps[i].as_init.config = c.pointer(config[0])

            elif step['type'] == 'final':
                if address:
                    steps[i].type = APP_CONFIG_STEP_FINAL
                    steps[i].key = c.addressof(context_key)

            elif step['type'] == 'set':
                slot = allocate_string(step['slot'])
                value = allocate_value(step['value'])

                if address:
                    steps[i].type = APP_CONFIG_STEP_SET
                    steps[i].key = c.addressof(context_key)
                    steps[i].as_set.slot = c.addressof(slot)
                    steps[i].as_set.value = c.pointer(value)

            elif step['type'] == 'assign':
                slot = allocate_string(step['slot'])
                source_key = allocate_string(step['source_key'])
                source_slot = allocate_string(step['source_slot']) if 'source_slot' in step else None

                if address:
                    steps[i].type = APP_CONFIG_STEP_ASSIGN
                    steps[i].key = c.addressof(context_key)
                    steps[i].as_assign.slot = c.addressof(slot)
                    steps[i].as_assign.source_key = c.addressof(source_key)
                    if source_slot:
                        steps[i].as_assign.source_slot = c.addressof(source_slot)

            elif step['type'] == 'act':
                action = allocate_string(step['action'])
                params = allocate_named_value_list(step['params'])

                if address:
                    steps[i].type = APP_CONFIG_STEP_ACT
                    steps[i].key = c.addressof(context_key)
                    steps[i].as_act.action = c.addressof(action)
                    if params:
                        steps[i].as_act.params = c.pointer(params[0])

    ### Do the job ###

    # Create the set of all strings in the configuration
    collect_strings()

    # Count total used bytes
    construct_configuration()

    # Create the memory-mapped configuration file
    mm = create_mmap_file(pathname, total_size)

    # Get memory address
    address = c.addressof(c.c_char.from_buffer(mm))

    # Write configuration to memory
    construct_configuration()

    # Initialize the header of pointers-containing data
    init_mmap_file_header(mm, total_size)

    # Unmap the memory
    mm.close()

    print(f"Wrote {total_size} bytes to '{pathname}',")
    print(f"including {total_padding} padding bytes")

###############################################################################

def create_mmap_file(pathname: str, size: int):
    """Create a memory-mapped file.

    @param[in] pathname : pathname of a created memory-mapped file
    @param[in] size     : size in bytes of the created memory-mapped file
    """
    if size < c.sizeof(mmap_header_t):
        raise ValueError("File size is too small -- can't fit the header.")

    # Create a memory-mapped file
    fd = os.open(pathname, os.O_CREAT | os.O_TRUNC | os.O_RDWR)

    # Allocate space
    os.write(fd, b'\x00' * size)

    # Map the file into memory
    mm = mmap.mmap(fd, size, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)

    # Close the file descriptor
    os.close(fd)

    # Return the memory
    return mm


def init_mmap_file_header(mm, size: int):
    """Initialize the header of a pointer-aware memory-mapped file.

    @param[in] mm   : a memory-mapped file
    @param[in] size : size in bytes of the memory-mapped file
    """
    mmap_header = mmap_header_t.from_buffer(mm)

    address = c.addressof(c.c_char.from_buffer(mm))

    mmap_header.addr = address
    mmap_header.end = address + size

###############################################################################
###############################################################################

class value_union_t(c.Union):
    """Union of a pointer to data and a pointer to function.
    """
    _fields_ = [('ptr', c.c_void_p),
                ('fptr', c.CFUNCTYPE(None))]


class value_t(c.Structure):
    """Value pointer with metadata.
    """
    _fields_ = [('as', value_union_t),
                ('size', c.c_size_t),
                ('num_of', c.c_size_t),
                ('own_memory', c.c_bool),
                ('type', c.c_int)]
    _anonymous_ = ['as']


class list_node_t(c.Structure):
    """Linked list node base.
    """
    pass

list_node_t._fields_ = [('prev', c.POINTER(list_node_t)),
                        ('next', c.POINTER(list_node_t))]


class list_node_named_t(c.Structure):
    """Named list node.
    """
    _fields_ = [('link', list_node_t),
                ('name', c.c_char_p)]


class list_node_named_value_t(c.Structure):
    """Named value list node.
    """
    _fields_ = [('base', list_node_named_t),
                ('value', value_t)]

###############################################################################

class app_config_step_init_t(c.Structure):
    """Specific configuration step data for context initialization.
    """
    _fields_ = [('interface_key', c.c_char_p),
                ('config', c.POINTER(list_node_named_value_t))]


class app_config_step_set_t(c.Structure):
    """Specific configuration step data for context slot value setting.
    """
    _fields_ = [('slot', c.c_char_p),
                ('value', c.POINTER(value_t))]


class app_config_step_assign_t(c.Structure):
    """Specific configuration step data for context assignment.
    """
    _fields_ = [('slot', c.c_char_p),
                ('source_key', c.c_char_p),
                ('source_slot', c.c_char_p)]


class app_config_step_act_t(c.Structure):
    """Specific configuration step data for context action.
    """
    _fields_ = [('action', c.c_char_p),
                ('params', c.POINTER(list_node_named_value_t))]


class app_config_step_union_t(c.Union):
    """Union of specific step data in application configuration step.
    """
    _fields_ = [('as_init', app_config_step_init_t),
                ('as_set', app_config_step_set_t),
                ('as_assign', app_config_step_assign_t),
                ('as_act', app_config_step_act_t)]


APP_CONFIG_STEP_INIT    = 0 # Initialize a context.
APP_CONFIG_STEP_FINAL   = 1 # Finalize a context.
APP_CONFIG_STEP_SET     = 2 # Set a value to context slot.
APP_CONFIG_STEP_ASSIGN  = 3 # Assign a value to context slot (get -> set).
APP_CONFIG_STEP_ACT     = 4 # Perform a context action.


class app_config_step_t(c.Structure):
    """Application configuration step.
    """
    _fields_ = [('type', c.c_int),
                ('key', c.c_char_p),
                ('as', app_config_step_union_t)]
    _anonymous_ = ['as']

###############################################################################

class signal_watch_set_t(c.Structure):
    """Set of signals to be watched.
    """
    _fields_ = [('f_SIGINT', c.c_bool), ('f_SIGQUIT', c.c_bool),
                ('f_SIGTERM', c.c_bool), ('f_SIGCHLD', c.c_bool),
                ('f_SIGCONT', c.c_bool), ('f_SIGTSTP', c.c_bool),
                ('f_SIGXCPU', c.c_bool), ('f_SIGXFSZ', c.c_bool),
                ('f_SIGPIPE', c.c_bool), ('f_SIGPOLL', c.c_bool),
                ('f_SIGURG', c.c_bool), ('f_SIGALRM', c.c_bool),
                ('f_SIGVTALRM', c.c_bool), ('f_SIGPROF', c.c_bool),
                ('f_SIGHUP', c.c_bool), ('f_SIGTTIN', c.c_bool),
                ('f_SIGTTOU', c.c_bool), ('f_SIGWINCH', c.c_bool),
                ('f_SIGUSR1', c.c_bool), ('f_SIGUSR2', c.c_bool),
                ('f_SIGRTMIN', c.c_bool * int(NUM_RT_SIGNALS))]


class library_load_config_t(c.Structure):
    """Parameters for archi_library_load().
    """
    _fields_ = [('pathname', c.c_char_p),
                ('lazy', c.c_bool),
                ('global', c.c_bool),
                ('flags', c.c_int)]


class app_loader_library_t(c.Structure):
    """Shared library loader configuration.
    """
    _fields_ = [('key', c.c_char_p),
                ('param', library_load_config_t)]


class app_loader_library_symbol_t(c.Structure):
    """Shared library symbol getter configuration.
    """
    _fields_ = [('key', c.c_char_p),
                ('symbol_name', c.c_char_p),
                ('library_key', c.c_char_p)]


class app_config_t(c.Structure):
    """Application configuration.
    """
    _fields_ = [('libraries', c.POINTER(app_loader_library_t)),
                ('interfaces', c.POINTER(app_loader_library_symbol_t)),
                ('steps', c.POINTER(app_config_step_t)),
                ('num_libraries', c.c_size_t),
                ('num_interfaces', c.c_size_t),
                ('num_steps', c.c_size_t)]


class mmap_header_t(c.Structure):
    """Mapped memory header.
    """
    _fields_ = [('addr', c.c_void_p),
                ('end', c.c_void_p)]


class configuration_t(c.Structure):
    """Full application configuration in memory.
    """
    _fields_ = [('mmap_header', mmap_header_t),
                ('signal_watch_set', c.POINTER(signal_watch_set_t)),
                ('app_config', app_config_t)]

