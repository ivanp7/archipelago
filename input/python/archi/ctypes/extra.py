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

    def __init__(self, watch=set(), invert_normal=False, invert_realtime=False):
        default_normal = True
        default_realtime = True

        if invert_normal:
            default_normal = False
            for signal in archi_signal_watch_set_t.SIGNALS:
                setattr(self, f'f_{signal}', True)

        if invert_realtime:
            default_realtime = False
            for idx in range(archi_signal_watch_set_t.NUM_RT_SIGNALS):
                self.f_SIGRTMIN[idx] = True

        for signal in watch:
            if isinstance(signal, str):
                if signal not in archi_signal_watch_set_t.SIGNALS:
                    raise ValueError(f"Unknown signal '{signal}'")

                setattr(self, f'f_{signal}', default_normal)

            elif isinstance(signal, tuple) and len(signal) == 2 \
                    and isinstance(signal[0], str) and isinstance(signal[1], int):
                if signal[0] == 'SIGRTMIN':
                    if signal[1] < 0 or signal[1] >= archi_signal_watch_set_t.NUM_RT_SIGNALS:
                        raise ValueError("Real-time signal out of supported range")

                    self.f_SIGRTMIN[signal[1]] = default_realtime

                elif signal[0] == 'SIGRTMAX':
                    if signal[1] > 0 or signal[1] <= -archi_signal_watch_set_t.NUM_RT_SIGNALS:
                        raise ValueError("Real-time signal out of supported range")

                    self.f_SIGRTMIN[(archi_signal_watch_set_t.NUM_RT_SIGNALS-1) \
                            + signal[1]] = default_realtime

                else:
                    raise ValueError(f"Unknown signal '{signal[0]}'")

            else:
                raise ValueError("Unsupported signal signature")

