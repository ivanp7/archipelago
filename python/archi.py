##########################################################################
# Python module: shared memory utilities for the Archipelago application #
##########################################################################

import ctypes as c
import mmap
import os

###############################################################################

class archi_value_union_t(c.Union):
    """Union of a pointer to data and a pointer to function.
    """
    _fields_ = [('ptr', c.c_void_p),
                ('fptr', c.CFUNCTYPE(None))]

class archi_value_t(c.Structure):
    """Value pointer with metadata.
    """
    _fields_ = [('as', archi_value_union_t),
                ('size', c.c_size_t),
                ('num_of', c.c_size_t),
                ('own_memory', c.c_bool),
                ('type', c.c_int)]
    _anonymous_ = ['as']

class archi_list_node_t(c.Structure):
    """Linked list node base.
    """
    pass

archi_list_node_t._fields_ = [('prev', c.POINTER(archi_list_node_t)),
                              ('next', c.POINTER(archi_list_node_t))]

class archi_list_node_named_t(c.Structure):
    """Named list node.
    """
    _fields_ = [('link', archi_list_node_t),
                ('name', c.c_char_p)]

class archi_list_node_named_value_t(c.Structure):
    """Named value list node.
    """
    _fields_ = [('base', archi_list_node_named_t),
                ('value', archi_value_t)]

###############################################################################

class archi_app_config_step_init_t(c.Structure):
    """Specific configuration step data for context initialization.
    """
    _fields_ = [('interface_key', c.c_void_p),
                ('config', c.POINTER(archi_list_node_named_value_t))]

class archi_app_config_step_set_t(c.Structure):
    """Specific configuration step data for context slot value setting.
    """
    _fields_ = [('slot', c.c_char_p),
                ('value', c.POINTER(archi_value_t))]

class archi_app_config_step_assign_t(c.Structure):
    """Specific configuration step data for context assignment.
    """
    _fields_ = [('slot', c.c_char_p),
                ('source_key', c.c_void_p),
                ('source_slot', c.c_char_p)]

class archi_app_config_step_act_t(c.Structure):
    """Specific configuration step data for context action.
    """
    _fields_ = [('action', c.c_char_p),
                ('params', c.POINTER(archi_list_node_named_value_t))]

class archi_app_config_step_union_t(c.Union):
    """Union of specific step data in application configuration step.
    """
    _fields_ = [('as_init', archi_app_config_step_init_t),
                ('as_set', archi_app_config_step_set_t),
                ('as_assign', archi_app_config_step_assign_t),
                ('as_act', archi_app_config_step_act_t)]

class archi_app_config_step_t(c.Structure):
    """Application configuration step.
    """
    _fields_ = [('type', c.c_int),
                ('key', c.c_void_p),
                ('as', archi_app_config_step_union_t)]
    _anonymous_ = ['as']

###############################################################################

class archi_signal_watch_set_t(c.Structure):
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
                ('f_SIGRTMIN', c.c_bool * int(0))]

class archi_app_loader_library_t(c.Structure):
    """Shared library loader configuration.
    """
    _fields_ = [('key', c.c_void_p),
                ('pathname', c.c_char_p),
                ('lazy', c.c_bool),
                ('global', c.c_bool)]

class archi_app_loader_library_symbol_t(c.Structure):
    """Shared library symbol getter configuration.
    """
    _fields_ = [('key', c.c_void_p),
                ('symbol_name', c.c_char_p),
                ('library_key', c.c_void_p)]

class archi_app_config_t(c.Structure):
    """Application configuration.
    """
    _fields_ = [('libraries', c.POINTER(archi_app_loader_library_t)),
                ('interfaces', c.POINTER(archi_app_loader_library_symbol_t)),
                ('steps', c.POINTER(archi_app_config_step_t)),
                ('num_libraries', c.c_size_t),
                ('num_interfaces', c.c_size_t),
                ('num_steps', c.c_size_t)]

class archi_shm_header_t(c.Structure):
    """Shared memory header.
    """
    _fields_ = [('shmaddr', c.c_void_p),
                ('shmend', c.c_void_p)]

class archi_process_config_shm_t(c.Structure):
    """Configuration of the process in shared memory.
    """
    _fields_ = [('shm_header', archi_shm_header_t),
                ('signal_watch_set', c.POINTER(archi_signal_watch_set_t)),
                ('app_config', archi_app_config_t)]

###############################################################################
###############################################################################

def create_mmap_file(file: str, size: int):
    """Create a pointer-aware memory-mapped file.
    """
    # Create a memory-mapped file
    fd = os.open(file, os.O_CREAT | os.O_TRUNC | os.O_RDWR)

    # Allocate space
    os.write(fd, b'\x00' * size)

    # Map the file into memory
    mm = mmap.mmap(fd, size, mmap.MAP_SHARED, mmap.PROT_WRITE | mmap.PROT_READ)

    # Close the file descriptor
    os.close(fd)

    # Initialize the shared memory header
    shm_header = archi_shm_header_t.from_buffer(mm)

    address = c.addressof(ctypes.c_char.from_buffer(mm))

    shm_header.shmaddr = ctypes.c_void_p.from_address(address)
    shm_header.shmend = ctypes.c_void_p.from_address(address + size)

    # Return the shared memory
    return mm

