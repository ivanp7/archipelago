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

#include "archi/res/file/api.fun.h"
#include "archi/res/file/header.typ.h"

#include <fcntl.h> // for open()
#include <unistd.h> // for close(), sysconf(), ftruncate()
#include <sys/mman.h> // for mmap(), munmap()
#include <sys/stat.h> // for fstat()
#include <stdint.h> // for uintptr_t

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

void*
archi_file_map(
        int fd,
        archi_file_map_params_t params,
        size_t *size)
{
    if (fd < 0)
        return NULL;

    int prot = (params.readable ? PROT_READ : 0) | (params.writable ? PROT_WRITE : 0);
    int all_flags = (params.shared ? MAP_SHARED_VALIDATE : MAP_PRIVATE) | params.flags;

    if (!params.has_header)
    {
        if (params.size == 0)
        {
            struct stat statbuf;
            if (fstat(fd, &statbuf) != 0)
                return NULL;

            if (params.offset >= (size_t)statbuf.st_size)
                return NULL;

            params.size = statbuf.st_size - params.offset;
        }

        void *mm = mmap(NULL, params.size, prot, all_flags, fd, params.offset);
        if (mm == MAP_FAILED)
            return NULL;

        if (size != NULL)
            *size = params.size;

        return mm;
    }
    else
    {
        archi_file_header_t *mm;
        archi_file_header_t header;

        // Map the memory the initial time to extract its header
        mm = mmap(NULL, sizeof(header), prot, all_flags, fd, params.offset);
        if (mm == MAP_FAILED)
            return NULL;

        header = *mm;

        if (params.size == 0)
        {
            uintptr_t addr = (uintptr_t)header.addr;
            uintptr_t end  = (uintptr_t)header.end;

            if (addr > end)
            {
                munmap(mm, sizeof(header));
                return NULL;
            }

            params.size = end - addr;
        }

        if (params.size < sizeof(header))
        {
            munmap(mm, sizeof(header));
            return NULL;
        }

        // Remap the memory of the correct size at the correct address
        munmap(mm, sizeof(header));

        mm = mmap(header.addr, params.size, prot, all_flags | MAP_FIXED_NOREPLACE, fd, params.offset);
        if (mm == MAP_FAILED)
            return NULL;

        if (mm != header.addr)
        {
            munmap(mm, params.size);
            return NULL;
        }

        if (size != NULL)
            *size = params.size;

        return mm;
    }
}

bool
archi_file_unmap(
        void *mm,
        size_t size)
{
    if (mm == NULL)
        return false;

    return munmap(mm, size) == 0;
}

