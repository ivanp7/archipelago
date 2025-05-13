/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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
 * @brief Color constants for logging and printing formatted messages.
 */

#pragma once
#ifndef _ARCHI_LOG_COLOR_DEF_H_
#define _ARCHI_LOG_COLOR_DEF_H_

#define ARCHI_COLOR_RESET           "\033[0m" ///< Reset colors to default.

#define ARCHI_COLOR_FG_BLACK        "\033[30m" ///< Black foreground.
#define ARCHI_COLOR_FG_RED          "\033[31m" ///< Dark red foreground.
#define ARCHI_COLOR_FG_GREEN        "\033[32m" ///< Dark green foreground.
#define ARCHI_COLOR_FG_YELLOW       "\033[33m" ///< Dark yellow foreground.
#define ARCHI_COLOR_FG_BLUE         "\033[34m" ///< Dark blue foreground.
#define ARCHI_COLOR_FG_MAGENTA      "\033[35m" ///< Dark magenta foreground.
#define ARCHI_COLOR_FG_CYAN         "\033[36m" ///< Dark cyan foreground.
#define ARCHI_COLOR_FG_WHITE        "\033[37m" ///< Light gray foreground.

#define ARCHI_COLOR_FG_BRI_BLACK    "\033[90m" ///< Dark gray foreground.
#define ARCHI_COLOR_FG_BRI_RED      "\033[91m" ///< Bright red foreground.
#define ARCHI_COLOR_FG_BRI_GREEN    "\033[92m" ///< Bright green foreground.
#define ARCHI_COLOR_FG_BRI_YELLOW   "\033[93m" ///< Bright yellow foreground.
#define ARCHI_COLOR_FG_BRI_BLUE     "\033[94m" ///< Bright blue foreground.
#define ARCHI_COLOR_FG_BRI_MAGENTA  "\033[95m" ///< Bright magenta foreground.
#define ARCHI_COLOR_FG_BRI_CYAN     "\033[96m" ///< Bright cyan foreground.
#define ARCHI_COLOR_FG_BRI_WHITE    "\033[97m" ///< White foreground.

#define ARCHI_COLOR_BG_BLACK        "\033[40m" ///< Black background.
#define ARCHI_COLOR_BG_RED          "\033[41m" ///< Dark red background.
#define ARCHI_COLOR_BG_GREEN        "\033[42m" ///< Dark green background.
#define ARCHI_COLOR_BG_YELLOW       "\033[43m" ///< Dark yellow background.
#define ARCHI_COLOR_BG_BLUE         "\033[44m" ///< Dark blue background.
#define ARCHI_COLOR_BG_MAGENTA      "\033[45m" ///< Dark magenta background.
#define ARCHI_COLOR_BG_CYAN         "\033[46m" ///< Dark cyan background.
#define ARCHI_COLOR_BG_WHITE        "\033[47m" ///< Light gray background.

#define ARCHI_COLOR_BG_BRI_BLACK    "\033[100m" ///< Dark gray background.
#define ARCHI_COLOR_BG_BRI_RED      "\033[101m" ///< Bright red background.
#define ARCHI_COLOR_BG_BRI_GREEN    "\033[102m" ///< Bright green background.
#define ARCHI_COLOR_BG_BRI_YELLOW   "\033[103m" ///< Bright yellow background.
#define ARCHI_COLOR_BG_BRI_BLUE     "\033[104m" ///< Bright blue background.
#define ARCHI_COLOR_BG_BRI_MAGENTA  "\033[105m" ///< Bright magenta background.
#define ARCHI_COLOR_BG_BRI_CYAN     "\033[106m" ///< Bright cyan background.
#define ARCHI_COLOR_BG_BRI_WHITE    "\033[107m" ///< White background.

#define ARCHI_COLOR_FG(num) "\033[38;5;" #num "m" ///< Foreground color #num (0-255).
#define ARCHI_COLOR_BG(num) "\033[48;5;" #num "m" ///< Background color #num (0-255).

#define ARCHI_COLOR_FG_RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m" ///< Foreground color in RGB format.
#define ARCHI_COLOR_BG_RGB(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m" ///< Background color in RGB format.

#endif // _ARCHI_LOG_COLOR_DEF_H_

