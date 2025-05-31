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
# @brief Extra C types.

import ctypes as c


class archi_signal_watch_set_t(c.Structure):
    """Mask of POSIX signals to watch and handle.
    """
    import signal

    SIGNALS = ['SIGINT', 'SIGQUIT', 'SIGTERM',              # interruption events
               'SIGCHLD', 'SIGCONT', 'SIGTSTP',             # process events
               'SIGXCPU', 'SIGXFSZ',                        # limit exceeding events
               'SIGPIPE', 'SIGPOLL', 'SIGURG',              # input/output events
               'SIGALRM', 'SIGVTALRM', 'SIGPROF',           # timer events
               'SIGHUP', 'SIGTTIN', 'SIGTTOU', 'SIGWINCH',  # terminal events
               'SIGUSR1', 'SIGUSR2']                        # user-defined events
    NUM_RT_SIGNALS = signal.SIGRTMAX - signal.SIGRTMIN + 1

    _fields_ = [(f'f_{signal}', c.c_bool) for signal in SIGNALS] \
            + [('f_SIGRTMIN', c.c_bool * NUM_RT_SIGNALS)]

