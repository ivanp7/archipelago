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
 * @brief File operations.
 */

#include "archi/file/api/file.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"

#include <fcntl.h> // for open()
#include <unistd.h> // for close(), ftruncate(), lseek(), read(), write(), fsync()
#include <limits.h> // for LLONG_MAX


archi_file_descriptor_t
archi_file_open(
        const char *pathname,
        archi_file_open_params_t params,
        ARCHI_ERROR_PARAM_DECL)
{
    if (pathname == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file pathname is NULL");
        return -1;
    }
    else if (!params.readable && !params.writable)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file is neither readable nor writable");
        return -1;
    }

    // Prepare file creation flags
    int flags = params.flags;

    if (params.readable && params.writable)
        flags |= O_RDWR;
    else if (params.writable)
        flags |= O_WRONLY;
    else if (params.readable)
        flags |= O_RDONLY;

    if (params.create)
        flags |= O_CREAT;

    if (params.exclusive)
        flags |= O_EXCL;

    if (params.truncate)
        flags |= O_TRUNC;

    if (params.append)
        flags |= O_APPEND;

    // Create file descriptor
    archi_file_descriptor_t fd = open(pathname, flags, (mode_t)params.mode);
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't open file '%s'", pathname);
        return -1;
    }

    // Truncate file if necessary
    if (params.truncate && (params.size != 0))
    {
        int ret = ftruncate(fd, params.size);
        if (ret != 0)
        {
            close(fd);
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't truncate file '%s'", pathname);
            return -1;
        }
    }

    ARCHI_ERROR_RESET();
    return fd;
}

void
archi_file_close(
        archi_file_descriptor_t fd,
        ARCHI_ERROR_PARAM_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return;
    }

    int ret = close(fd);
    if (ret != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't close file descriptor (%i)", fd);
        return;
    }

    ARCHI_ERROR_RESET();
}

archi_file_descriptor_t
archi_file_duplicate(
        archi_file_descriptor_t fd,
        ARCHI_ERROR_PARAM_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return -1;
    }

    archi_file_descriptor_t new_fd = dup(fd);
    if (new_fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't duplicate file descriptor (%i)", fd);
        return new_fd;
    }

    ARCHI_ERROR_RESET();
    return new_fd;
}

long long
archi_file_seek(
        archi_file_descriptor_t fd,
        long long offset,
        int whence,
        ARCHI_ERROR_PARAM_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return -1;
    }

    if (sizeof(off_t) < sizeof(long long))
    {
#define OFF_MSB ((long long)1 << (sizeof(off_t)*CHAR_BIT - 2))
#define OFF_MAX (OFF_MSB | (OFF_MSB - 1))
#define OFF_MIN (-OFF_MAX - 1)

        if ((offset < OFF_MIN) || (offset > OFF_MAX))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file offset (%lli) out of range [%lli; %lli]",
                    offset, OFF_MIN, OFF_MAX);
            return -1;
        }

#undef OFF_MIN
#undef OFF_MAX
#undef OFF_MSB
    }

    off_t tell = lseek(fd, offset, whence);
    if (tell < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't seek file descriptor (%i)", fd);
        return -1;
    }

    if (sizeof(off_t) > sizeof(long long))
    {
        if (tell > (off_t)LLONG_MAX)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file offset (%lli) out of range [0; %lli]",
                    (long long)tell, LLONG_MAX);
            return -1;
        }
    }

    ARCHI_ERROR_RESET();
    return tell;
}

size_t
archi_file_read(
        archi_file_descriptor_t fd,
        archi_pointer_t destination,
        ARCHI_ERROR_PARAM_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return 0;
    }
    else if (!ARCHI_POINTER_TO_WRITABLE_DATA(destination.attr) && !ARCHI_POINTER_TO_STACK(destination.attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination is not a pointer to writable memory");
        return 0;
    }

    size_t length, stride;
    if (!archi_pointer_attr_unpk__pdata(destination.attr, &length, &stride, NULL, NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination data type is not primitive");
        return 0;
    }

    size_t size = length * stride;

    if (size == 0)
    {
        ARCHI_ERROR_RESET();
        return 0;
    }
    else if (size > SSIZE_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file read size (%zu) is greater than SSIZE_MAX", size);
        return 0;
    }

    size_t total_read = 0;
    char *dest = destination.ptr;

    while (total_read < size)
    {
        ssize_t num_read = read(fd, dest, size - total_read);
        if (num_read < 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "couldn't read data from file descriptor (%i)", fd);
            return total_read;
        }

        total_read += num_read;
        dest += num_read;
    }

    ARCHI_ERROR_RESET();
    return total_read;
}

size_t
archi_file_write(
        archi_file_descriptor_t fd,
        archi_pointer_t source,
        ARCHI_ERROR_PARAM_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return 0;
    }
    else if (ARCHI_POINTER_TO_FUNCTION(source.attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source is not a pointer to readable memory");
        return 0;
    }

    size_t length, stride;
    if (!archi_pointer_attr_unpk__pdata(source.attr, &length, &stride, NULL, NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source data type is not primitive");
        return 0;
    }

    size_t size = length * stride;

    if (size == 0)
    {
        ARCHI_ERROR_RESET();
        return 0;
    }
    else if (size > SSIZE_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file write size (%zu) is greater than SSIZE_MAX", size);
        return 0;
    }

    size_t total_written = 0;
    const char *src = source.cptr;

    while (total_written < size)
    {
        ssize_t num_written = write(fd, src, size - total_written);
        if (num_written < 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "couldn't write data to file descriptor (%i)", fd);
            return total_written;
        }

        total_written += num_written;
        src += num_written;
    }

    ARCHI_ERROR_RESET();
    return total_written;
}

void
archi_file_sync(
        archi_file_descriptor_t fd,
        ARCHI_ERROR_PARAM_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return;
    }

    int ret = fsync(fd);
    if (ret != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't sync file descriptor (%i)", fd);
        return;
    }

    ARCHI_ERROR_RESET();
}

