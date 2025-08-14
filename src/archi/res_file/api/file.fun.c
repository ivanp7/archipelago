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
 * @brief File operations.
 */

#include "archi/res_file/api/file.fun.h"

#include <fcntl.h> // for open()
#include <unistd.h> // for close(), sysconf(), ftruncate()

size_t
archi_file_page_size(void)
{
    return sysconf(_SC_PAGE_SIZE);
}

int
archi_file_open(
        archi_file_open_params_t params)
{
    if (params.pathname == NULL)
        return -1;

    int flags = params.flags;

    if (params.create)
        flags |= O_CREAT;

    if (params.exclusive)
        flags |= O_EXCL;

    if (params.truncate)
        flags |= O_TRUNC;

    if (params.readable && params.writable)
        flags |= O_RDWR;
    else if (params.readable)
        flags |= O_RDONLY;
    else
        flags |= O_WRONLY;

    if (params.nonblock)
        flags |= O_NONBLOCK;

    mode_t mode = params.mode;

    int fd = open(params.pathname, flags, mode);

    if ((fd >= 0) && params.truncate && (params.size > 0))
    {
        int ret = ftruncate(fd, params.size);
        if (ret < 0)
        {
            close(fd);
            fd = -1;
        }
    }

    return fd;
}

bool
archi_file_close(
        int fd)
{
    if (fd < 0)
        return false;

    return close(fd) == 0;
}

