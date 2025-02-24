##########################################################################
# Python module: shared memory utilities for the Archipelago application #
##########################################################################

import ctypes as c
import mmap
import os
import signal

###############################################################################

NUM_RT_SIGNALS = signal.SIGRTMAX - signal.SIGRTMIN + 1

###############################################################################

class value_union_t(c.Union):
    """Union of a pointer to data and a pointer to function.
    """
    _fields_ = [('ptr', c.c_void_p),
                ('fptr', c.CFUNCTYPE(None))]

VALUE_NULL      = 0  # No value.
VALUE_FALSE     = 1 # Falsey boolean value.
VALUE_TRUE      = 2 # Truthy boolean value.
VALUE_UINT      = 3 # Unsigned integer.
VALUE_SINT      = 4 # Signed integer.
VALUE_FLOAT     = 5 # Floating-point number.
VALUE_STRING    = 6 # Null-terminated string.
VALUE_DATA      = 7 # Binary data.
VALUE_NESTED    = 8 # Nested node.
VALUE_LIST      = 9 # Nested list.
VALUE_FUNCTION  = 10 # Pointer to a function.

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
    _fields_ = [('interface_key', c.c_void_p),
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
                ('source_key', c.c_void_p),
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
                ('key', c.c_void_p),
                ('as', app_config_step_union_t)]
    _anonymous_ = ['as']

###############################################################################

class signal_watch_set_t(c.Structure):
    """Flags designating which signals need to be watched.
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

class app_loader_library_t(c.Structure):
    """Shared library loader configuration.
    """
    _fields_ = [('key', c.c_void_p),
                ('pathname', c.c_char_p),
                ('lazy', c.c_bool),
                ('global', c.c_bool)]

class app_loader_library_symbol_t(c.Structure):
    """Shared library symbol getter configuration.
    """
    _fields_ = [('key', c.c_void_p),
                ('symbol_name', c.c_char_p),
                ('library_key', c.c_void_p)]

class app_config_t(c.Structure):
    """Application configuration.
    """
    _fields_ = [('libraries', c.POINTER(app_loader_library_t)),
                ('interfaces', c.POINTER(app_loader_library_symbol_t)),
                ('steps', c.POINTER(app_config_step_t)),
                ('num_libraries', c.c_size_t),
                ('num_interfaces', c.c_size_t),
                ('num_steps', c.c_size_t)]

class shm_header_t(c.Structure):
    """Shared memory header.
    """
    _fields_ = [('shmaddr', c.c_void_p),
                ('shmend', c.c_void_p)]

class process_config_shm_t(c.Structure):
    """Configuration of the process in shared memory.
    """
    _fields_ = [('shm_header', shm_header_t),
                ('signal_watch_set', c.POINTER(signal_watch_set_t)),
                ('app_config', app_config_t)]

###############################################################################
###############################################################################

def create_mmap_file(pathname: str, size: int):
    """Create a memory-mapped file.
    """
    if size < c.sizeof(shm_header_t):
        raise ValueError("File size is too small -- can't fit the header.")

    # Create a memory-mapped file
    fd = os.open(pathname, os.O_CREAT | os.O_TRUNC | os.O_RDWR)

    # Allocate space
    os.write(fd, b'\x00' * size)

    # Map the file into memory
    mm = mmap.mmap(fd, size, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)

    # Close the file descriptor
    os.close(fd)

    # Return the shared memory
    return mm


def init_mmap_file_header(mm, size: int):
    """Initialize the header of a pointer-aware memory-mapped file.
    """
    shm_header = shm_header_t.from_buffer(mm)

    address = c.addressof(c.c_char.from_buffer(mm))

    shm_header.shmaddr = address
    shm_header.shmend = address + size

###############################################################################

class memory_manager:
    """Count number of used bytes and allocate memory in memory-mapped file.
    """
    def __init__(self, mm=None):
        self._size = 0

        if mm:
            self._address = c.addressof(c.c_char.from_buffer(mm))
        else:
            self._address = None

    def total_used_size(self):
        return self._size

    def allocate(self, typ):
        size = c.sizeof(typ)
        alignment = c.alignment(typ)

        self._size = (self._size + (alignment - 1)) & ~(alignment - 1) # fulfill the alignment requirement

        if self._address:
            obj = typ.from_address(self._address + self._size)
        else:
            obj = None

        self._size += size
        return obj

    def allocate_string(self, value):
        size = len(value) + 1

        if self._address:
            buf = c.create_string_buffer(value)
            obj = self.allocate(type(buf))
            c.memmove(obj, buf, size)
        else:
            obj = None
            self._size += size

        return obj

###############################################################################

def create_mmap_config_file(pathname: str, constr_func):
    """Create a pointer-aware memory-mapped file and
    form the configuration in it using the provided constructor function.
    """
    # Compute total size of the configuration
    manager_dummy = memory_manager()
    constr_func(manager_dummy)
    total_size = manager_dummy.total_used_size()

    # Create the memory-mapped file
    mm = create_mmap_file(pathname, total_size)

    # Form the configuration objects in the memory-mapped file
    manager_shm = memory_manager(mm)
    constr_func(manager_shm)
    init_mmap_file_header(mm, total_size)

