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
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"

#include <fcntl.h> // for open()
#include <unistd.h> // for close(), ftruncate(), lseek(), read(), write(), fsync()

archi_file_descriptor_t
archi_file_open(
        const char *pathname,
        archi_file_open_params_t params,
        ARCHI_ERROR_PARAMETER_DECL)
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
    if (params.truncate && (params.size > 0))
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
        ARCHI_ERROR_PARAMETER_DECL)
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
        ARCHI_ERROR_PARAMETER_DECL)
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

ptrdiff_t
archi_file_seek(
        archi_file_descriptor_t fd,
        ptrdiff_t offset,
        int whence,
        ARCHI_ERROR_PARAMETER_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return -1;
    }

    if (sizeof(off_t) < sizeof(ptrdiff_t))
    {
#define OFF_MSB ((ptrdiff_t)1 << (sizeof(off_t)*CHAR_BIT - 2))
#define OFF_MAX (OFF_MSB | (OFF_MSB - 1))
#define OFF_MIN (-OFF_MAX - 1)

        if ((offset < OFF_MIN) || (offset > OFF_MAX))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file offset (%ti) out of bounds (min = %ti, max = %ti)",
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

    if (sizeof(off_t) > sizeof(ptrdiff_t))
    {
        if (tell > (off_t)PTRDIFF_MAX)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file offset (%lli) out of bounds (max = %ti)",
                    (long long)tell, PTRDIFF_MAX);
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
        ARCHI_ERROR_PARAMETER_DECL)
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
    if (!archi_pointer_attr_parse__transp_data(destination.attr, &length, &stride, NULL, NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination data type is not transparent");
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

    ssize_t num_read = read(fd, destination.ptr, size);
    if (num_read < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "couldn't read data from file descriptor (%i)", fd);
        return 0;
    }

    ARCHI_ERROR_RESET();
    return num_read;
}

size_t
archi_file_write(
        archi_file_descriptor_t fd,
        archi_pointer_t source,
        ARCHI_ERROR_PARAMETER_DECL)
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
    if (!archi_pointer_attr_parse__transp_data(source.attr, &length, &stride, NULL, NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source data type is not transparent");
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

    ssize_t num_written = write(fd, source.ptr, size);
    if (num_written < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "couldn't write data to file descriptor (%i)", fd);
        return 0;
    }

    ARCHI_ERROR_RESET();
    return num_written;
}

void
archi_file_sync(
        archi_file_descriptor_t fd,
        ARCHI_ERROR_PARAMETER_DECL)
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

