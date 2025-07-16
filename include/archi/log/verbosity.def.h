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
 * @brief Log verbosity levels.
 */

#pragma once
#ifndef _ARCHI_LOG_VERBOSITY_DEF_H_
#define _ARCHI_LOG_VERBOSITY_DEF_H_

#define ARCHI_LOG_VERBOSITY_QUIET    0  ///< Disable all logging.
#define ARCHI_LOG_VERBOSITY_ERROR    1  ///< Print errors only.
#define ARCHI_LOG_VERBOSITY_WARNING  2  ///< Print errors and warnings only.
#define ARCHI_LOG_VERBOSITY_NOTICE   3  ///< Print errors, warnings, and notices only.
#define ARCHI_LOG_VERBOSITY_INFO     4  ///< Print errors, warnings, notices, and info messages.
#define ARCHI_LOG_VERBOSITY_DEBUG    5  ///< Print everything, including debug messages.

#define ARCHI_LOG_VERBOSITY_MAX ARCHI_LOG_VERBOSITY_DEBUG ///< The maximum verbosity level.

#endif // _ARCHI_LOG_VERBOSITY_DEF_H_

