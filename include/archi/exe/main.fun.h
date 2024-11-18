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
 * @brief Executable entry function.
 */

#pragma once
#ifndef _ARCHI_EXE_MAIN_FUN_H_
#define _ARCHI_EXE_MAIN_FUN_H_

struct archi_application;
struct archi_cmdline_args;

/**
 * @brief Application entry function.
 *
 * This function reads the application configuration from the specified stream,
 * initializes the application, and runs the finite state machine.
 * If the FSM returns normally, the application is finalized.
 *
 * @return Application exit code.
 */
int
archi_main(
        struct archi_application *app, ///< [out] Application object.
        const struct archi_cmdline_args *args ///< [in] Parsed values of command line arguments.
);

#endif // _ARCHI_EXE_MAIN_FUN_H_

