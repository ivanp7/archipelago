/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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
 * @brief The environment context type.
 */

#pragma once
#ifndef _SRC_ARCHI_ENV_API_CONTEXT_TYP_H_
#define _SRC_ARCHI_ENV_API_CONTEXT_TYP_H_

#ifndef __STDC_NO_THREADS__
#  include <threads.h> // for mtx_t
#endif
#include <stdbool.h>


struct archi_env_context {
#ifndef __STDC_NO_THREADS__
    mtx_t *getenv_lock; ///< getenv() protection mutex.
#endif

    bool initialized; ///< Whether the context was initialized.
};

#endif // _SRC_ARCHI_ENV_API_CONTEXT_TYP_H_

