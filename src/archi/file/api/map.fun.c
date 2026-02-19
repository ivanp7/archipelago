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
 * @brief File mapping operations.
 */

#include "archi/file/api/map.fun.h"
#include "archi/file/api/header.typ.h"

#include <sys/mman.h> // for mmap(), munmap()
#include <sys/stat.h> // for fstat()
#include <unistd.h> // for sysconf()
#include <stdint.h> // for uintptr_t


size_t
archi_file_page_size(void)
{
    return sysconf(_SC_PAGE_SIZE);
}

void*
archi_file_map(
        archi_file_descriptor_t fd,
        archi_file_map_params_t params,
        size_t *out_size,
        ARCHI_ERROR_PARAM_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return NULL;
    }

    int prot = (params.readable ? PROT_READ : 0) | (params.writable ? PROT_WRITE : 0);
    int all_flags = (params.shared ? MAP_SHARED_VALIDATE : MAP_PRIVATE) | params.flags;

    struct stat statbuf;
    if (fstat(fd, &statbuf) != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't get status of file descriptor (%i)", fd);
        return NULL;
    }

    if (!params.ptr_support)
    {
        if (params.size == 0)
        {
            if (params.offset >= (size_t)statbuf.st_size)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "offset (%zu) is greater or equal to file size (%zu)",
                        params.offset, (size_t)statbuf.st_size);
                return NULL;
            }

            params.size = (size_t)statbuf.st_size - params.offset;
        }

        void *mm = mmap(NULL, params.size, prot, all_flags, fd, params.offset);
        if (mm == MAP_FAILED)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't map file descriptor (%i), offset = %zu, size = %zu, flags = %#x, prot = %#x",
                    fd, params.offset, params.size, all_flags, prot);
            return NULL;
        }

        if (out_size != NULL)
            *out_size = params.size;

        ARCHI_ERROR_RESET();
        return mm;
    }
    else
    {
        archi_file_ptr_support_header_t *mm;
        archi_file_ptr_support_header_t header;

        if ((size_t)statbuf.st_size < sizeof(header))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file size (%zu) is less than header size (%zu)",
                    (size_t)statbuf.st_size, sizeof(header));
            return NULL;
        }
        else if ((params.size != 0) && (params.size < sizeof(header)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "size of mapped file region (%zu) is less than header size (%zu)",
                    params.size, sizeof(header));
            return NULL;
        }

        // Map the memory initially to extract the header
        mm = mmap(NULL, sizeof(header), prot, all_flags, fd, params.offset);
        if (mm == MAP_FAILED)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't map file descriptor (%i), offset = %zu, size = %zu, flags = %#x, prot = %#x",
                    fd, params.offset, sizeof(header), all_flags, prot);
            return NULL;
        }

        header = *mm;

        if (params.size == 0)
        {
            // Use the size from the header
            if (header.size > SIZE_MAX - sizeof(header))
            {
                munmap(mm, sizeof(header));

                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file size is greater than SIZE_MAX");
                return NULL;
            }

            params.size = sizeof(header) + header.size;
        }

        // Remap the memory of the correct size at the correct address
        munmap(mm, sizeof(header));

        mm = mmap(header.addr, params.size, prot, all_flags | MAP_FIXED_NOREPLACE, fd, params.offset);
        if (mm == MAP_FAILED)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't map file descriptor (%i) at %p, offset = %zu, size = %zu, flags = %#x, prot = %#x",
                    fd, header.addr, params.offset, sizeof(header), all_flags, prot);
            return NULL;
        }

        if (mm != header.addr)
        {
            munmap(mm, params.size);

            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "file is mapped at the wrong address (%p), expected %p",
                    (void*)mm, header.addr);
            return NULL;
        }

        if (out_size != NULL)
            *out_size = params.size;

        ARCHI_ERROR_RESET();
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

