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
 * @brief Constants and macros for printing formatted messages and logging.
 */

#pragma once
#ifndef _ARCHI_UTIL_PRINT_DEF_H_
#define _ARCHI_UTIL_PRINT_DEF_H_

#ifndef ARCHI_FEATURE_COLORLESS

#  define ARCHI_COLOR_RESET           "\033[0m" ///< Reset colors to default.

#  define ARCHI_COLOR_FG_BLACK        "\033[30m" ///< Black foreground.
#  define ARCHI_COLOR_FG_RED          "\033[31m" ///< Dark red foreground.
#  define ARCHI_COLOR_FG_GREEN        "\033[32m" ///< Dark green foreground.
#  define ARCHI_COLOR_FG_YELLOW       "\033[33m" ///< Dark yellow foreground.
#  define ARCHI_COLOR_FG_BLUE         "\033[34m" ///< Dark blue foreground.
#  define ARCHI_COLOR_FG_MAGENTA      "\033[35m" ///< Dark magenta foreground.
#  define ARCHI_COLOR_FG_CYAN         "\033[36m" ///< Dark cyan foreground.
#  define ARCHI_COLOR_FG_WHITE        "\033[37m" ///< Light gray foreground.

#  define ARCHI_COLOR_FG_BRI_BLACK    "\033[90m" ///< Dark gray foreground.
#  define ARCHI_COLOR_FG_BRI_RED      "\033[91m" ///< Bright red foreground.
#  define ARCHI_COLOR_FG_BRI_GREEN    "\033[92m" ///< Bright green foreground.
#  define ARCHI_COLOR_FG_BRI_YELLOW   "\033[93m" ///< Bright yellow foreground.
#  define ARCHI_COLOR_FG_BRI_BLUE     "\033[94m" ///< Bright blue foreground.
#  define ARCHI_COLOR_FG_BRI_MAGENTA  "\033[95m" ///< Bright magenta foreground.
#  define ARCHI_COLOR_FG_BRI_CYAN     "\033[96m" ///< Bright cyan foreground.
#  define ARCHI_COLOR_FG_BRI_WHITE    "\033[97m" ///< White foreground.

#  define ARCHI_COLOR_BG_BLACK        "\033[40m" ///< Black background.
#  define ARCHI_COLOR_BG_RED          "\033[41m" ///< Dark red background.
#  define ARCHI_COLOR_BG_GREEN        "\033[42m" ///< Dark green background.
#  define ARCHI_COLOR_BG_YELLOW       "\033[43m" ///< Dark yellow background.
#  define ARCHI_COLOR_BG_BLUE         "\033[44m" ///< Dark blue background.
#  define ARCHI_COLOR_BG_MAGENTA      "\033[45m" ///< Dark magenta background.
#  define ARCHI_COLOR_BG_CYAN         "\033[46m" ///< Dark cyan background.
#  define ARCHI_COLOR_BG_WHITE        "\033[47m" ///< Light gray background.

#  define ARCHI_COLOR_BG_BRI_BLACK    "\033[100m" ///< Dark gray background.
#  define ARCHI_COLOR_BG_BRI_RED      "\033[101m" ///< Bright red background.
#  define ARCHI_COLOR_BG_BRI_GREEN    "\033[102m" ///< Bright green background.
#  define ARCHI_COLOR_BG_BRI_YELLOW   "\033[103m" ///< Bright yellow background.
#  define ARCHI_COLOR_BG_BRI_BLUE     "\033[104m" ///< Bright blue background.
#  define ARCHI_COLOR_BG_BRI_MAGENTA  "\033[105m" ///< Bright magenta background.
#  define ARCHI_COLOR_BG_BRI_CYAN     "\033[106m" ///< Bright cyan background.
#  define ARCHI_COLOR_BG_BRI_WHITE    "\033[107m" ///< White background.

#else // ARCHI_FEATURE_COLORLESS

#  define ARCHI_COLOR_RESET           "" ///< Reset colors to default.

#  define ARCHI_COLOR_FG_BLACK        "" ///< Black foreground.
#  define ARCHI_COLOR_FG_RED          "" ///< Dark red foreground.
#  define ARCHI_COLOR_FG_GREEN        "" ///< Dark green foreground.
#  define ARCHI_COLOR_FG_YELLOW       "" ///< Dark yellow foreground.
#  define ARCHI_COLOR_FG_BLUE         "" ///< Dark blue foreground.
#  define ARCHI_COLOR_FG_MAGENTA      "" ///< Dark magenta foreground.
#  define ARCHI_COLOR_FG_CYAN         "" ///< Dark cyan foreground.
#  define ARCHI_COLOR_FG_WHITE        "" ///< Light gray foreground.

#  define ARCHI_COLOR_FG_BRI_BLACK    "" ///< Dark gray foreground.
#  define ARCHI_COLOR_FG_BRI_RED      "" ///< Bright red foreground.
#  define ARCHI_COLOR_FG_BRI_GREEN    "" ///< Bright green foreground.
#  define ARCHI_COLOR_FG_BRI_YELLOW   "" ///< Bright yellow foreground.
#  define ARCHI_COLOR_FG_BRI_BLUE     "" ///< Bright blue foreground.
#  define ARCHI_COLOR_FG_BRI_MAGENTA  "" ///< Bright magenta foreground.
#  define ARCHI_COLOR_FG_BRI_CYAN     "" ///< Bright cyan foreground.
#  define ARCHI_COLOR_FG_BRI_WHITE    "" ///< White foreground.

#  define ARCHI_COLOR_BG_BLACK        "" ///< Black background.
#  define ARCHI_COLOR_BG_RED          "" ///< Dark red background.
#  define ARCHI_COLOR_BG_GREEN        "" ///< Dark green background.
#  define ARCHI_COLOR_BG_YELLOW       "" ///< Dark yellow background.
#  define ARCHI_COLOR_BG_BLUE         "" ///< Dark blue background.
#  define ARCHI_COLOR_BG_MAGENTA      "" ///< Dark magenta background.
#  define ARCHI_COLOR_BG_CYAN         "" ///< Dark cyan background.
#  define ARCHI_COLOR_BG_WHITE        "" ///< Light gray background.

#  define ARCHI_COLOR_BG_BRI_BLACK    "" ///< Dark gray background.
#  define ARCHI_COLOR_BG_BRI_RED      "" ///< Bright red background.
#  define ARCHI_COLOR_BG_BRI_GREEN    "" ///< Bright green background.
#  define ARCHI_COLOR_BG_BRI_YELLOW   "" ///< Bright yellow background.
#  define ARCHI_COLOR_BG_BRI_BLUE     "" ///< Bright blue background.
#  define ARCHI_COLOR_BG_BRI_MAGENTA  "" ///< Bright magenta background.
#  define ARCHI_COLOR_BG_BRI_CYAN     "" ///< Bright cyan background.
#  define ARCHI_COLOR_BG_BRI_WHITE    "" ///< White background.

#endif // ARCHI_FEATURE_COLORLESS

/*****************************************************************************/

#define ARCHI_LOG_COLOR_ERROR ARCHI_COLOR_FG_BRI_RED        ///< Color of error messages.
#define ARCHI_LOG_COLOR_WARNING ARCHI_COLOR_FG_BRI_YELLOW   ///< Color of warning messages.
#define ARCHI_LOG_COLOR_NOTICE ARCHI_COLOR_FG_BRI_BLUE      ///< Color of notice messages.
#define ARCHI_LOG_COLOR_INFO "" /* default color */         ///< Color of info messages.
#define ARCHI_LOG_COLOR_DEBUG ARCHI_COLOR_FG_BRI_BLACK      ///< Color of debug messages.

#define ARCHI_LOG_TYPESTR_ERROR     "ERR" ///< Type string designating error messages.
#define ARCHI_LOG_TYPESTR_WARNING   "WRN" ///< Type string designating warning messages.
#define ARCHI_LOG_TYPESTR_NOTICE    "NOT" ///< Type string designating warning messages.
#define ARCHI_LOG_TYPESTR_INFO      "INF" ///< Type string designating info messages.
#define ARCHI_LOG_TYPESTR_DEBUG     "DBG" ///< Type string designating debug messages.

#define ARCHI_LOG_VERBOSITY_QUIET 0     ///< Disable logging.
#define ARCHI_LOG_VERBOSITY_ERROR 1     ///< Print errors only.
#define ARCHI_LOG_VERBOSITY_WARNING 2   ///< Print errors and warnings only.
#define ARCHI_LOG_VERBOSITY_NOTICE 3    ///< Print errors, warnings, notices.
#define ARCHI_LOG_VERBOSITY_INFO 4      ///< Print errors, warnings, notices, information.
#define ARCHI_LOG_VERBOSITY_DEBUG 5     ///< Print everything including debug messages.

#define ARCHI_LOG_VERBOSITY_MAX ARCHI_LOG_VERBOSITY_DEBUG ///< Print everything.

#define ARCHI_LOG_VERBOSITY_DEFAULT_LOWER ARCHI_LOG_VERBOSITY_NOTICE ///< Lower default verbosity level.
#define ARCHI_LOG_VERBOSITY_DEFAULT_HIGHER ARCHI_LOG_VERBOSITY_INFO  ///< Higher default verbosity level.

#endif // _ARCHI_UTIL_PRINT_DEF_H_

