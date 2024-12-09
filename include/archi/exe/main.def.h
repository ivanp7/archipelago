/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Constants for the implementation of executable entry function.
 */

#pragma once
#ifndef _ARCHI_EXE_MAIN_DEF_H_
#define _ARCHI_EXE_MAIN_DEF_H_

/**
 * @brief Index of application configuration pointer in shared memory header.
 */
#define ARCHI_SHM_APP_CONFIG_INDEX 1

/**
 * @brief Index of signal management configuration pointer in shared memory header.
 */
#define ARCHI_SHM_SIGNALS_CONFIG_INDEX 2

/*****************************************************************************/

/**
 * @brief Application global context alias name.
 */
#define ARCHI_APP_CONTEXT_ALIAS "" // empty string is forbidden
                                   // and is unlikely to be used elsewere

/**
 * @brief Port of application global context: entry state function.
 */
#define ARCHI_APP_CONTEXT_PORT_ENTRY_STATE_FUNC "entry_state.function"
/**
 * @brief Port of application global context: entry state data.
 */
#define ARCHI_APP_CONTEXT_PORT_ENTRY_STATE_DATA "entry_state.data"

/**
 * @brief Port of application global context: state transition function.
 */
#define ARCHI_APP_CONTEXT_PORT_STATE_TRANS_FUNC "state_transition.function"
/**
 * @brief Port of application global context: state transition data.
 */
#define ARCHI_APP_CONTEXT_PORT_STATE_TRANS_DATA "state_transition.data"

/**
 * @brief Port of application global context: signal flags.
 */
#define ARCHI_APP_CONTEXT_PORT_SIGNAL_SET "signal.flags"

#endif // _ARCHI_EXE_MAIN_DEF_H_

