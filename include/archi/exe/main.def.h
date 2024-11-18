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
 * @brief Application global context alias name.
 */
#define ARCHI_APP_CONTEXT_ALIAS "" // empty string is forbidden
                                   // and is unlikely to be used elsewere

/**
 * @brief Port name of entry state function of application global context.
 */
#define ARCHI_APP_CONTEXT_PORT_ENTRY_STATE_FUNC "entry_state.function"
/**
 * @brief Port name of entry state data of application global context.
 */
#define ARCHI_APP_CONTEXT_PORT_ENTRY_STATE_DATA "entry_state.data"

/**
 * @brief Port name of state transition function of application global context.
 */
#define ARCHI_APP_CONTEXT_PORT_STATE_TRANS_FUNC "state_transition.function"
/**
 * @brief Port name of state transition data of application global context.
 */
#define ARCHI_APP_CONTEXT_PORT_STATE_TRANS_DATA "state_transition.data"

#endif // _ARCHI_EXE_MAIN_DEF_H_

