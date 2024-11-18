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
 * @brief OS-specific features.
 */

#include "archi/util/os.fun.h"

#include <sys/shm.h> // for ftok()
#include <sys/ipc.h> // for shmget(), shmat(), shmdt()

#include <dlfcn.h> // for dlopen(), dlclose(), dlsym()

void**
archi_shared_memory_create(
        const char *pathname,
        int proj_id,

        size_t size)
{
    if (pathname == NULL)
        return NULL;
    else if (size <= sizeof(void*))
        return NULL;

    key_t key = ftok(pathname, proj_id);
    if (key == -1)
        return NULL;

    int shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0640);
    if (shmid == -1)
        return NULL;

    void **shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void*)-1)
        goto failure;

    shmaddr[0] = shmaddr;

    return shmaddr;

failure:
    archi_shared_memory_destroy(pathname, proj_id);
    return NULL;
}

bool
archi_shared_memory_destroy(
        const char *pathname,
        int proj_id)
{
    if (pathname == NULL)
        return false;

    key_t key = ftok(pathname, proj_id);
    if (key == -1)
        return false;

    int shmid = shmget(key, 0, 0);
    if (shmid == -1)
        return false;

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        return false;

    return true;
}

void**
archi_shared_memory_attach(
        const char *pathname,
        const int proj_id,

        bool writable)
{
    if (pathname == NULL)
        return NULL;

    key_t key = ftok(pathname, proj_id);
    if (key == -1)
        return NULL;

    int shmid = shmget(key, 0, 0);
    if (shmid == -1)
        return NULL;

    int shmflg = writable ? 0 : SHM_RDONLY;

    void **current_shmaddr = shmat(shmid, NULL, shmflg);
    if (current_shmaddr == (void*)-1)
        return NULL;

    void *shmaddr = current_shmaddr[0];

    if (shmaddr != current_shmaddr)
    {
        if (shmdt(current_shmaddr) == -1)
            return NULL;

        current_shmaddr = shmat(shmid, shmaddr, SHM_RND | shmflg);
        if (current_shmaddr == (void*)-1)
            return NULL;

        if (current_shmaddr != shmaddr)
        {
            shmdt(current_shmaddr);
            return NULL;
        }
    }

    return shmaddr;
}

bool
archi_shared_memory_detach(
        const void *shmaddr)
{
    return shmdt(shmaddr) == 0;
}

/*****************************************************************************/

void*
archi_library_load(
        const char *file)
{
    if (file == NULL)
        return NULL;

    return dlopen(file, RTLD_LAZY | RTLD_GLOBAL);
}

void
archi_library_unload(
        void *handle)
{
    if (handle != NULL)
        dlclose(handle);
}

void*
archi_library_get_symbol(
        void *restrict handle,
        const char *restrict symbol)
{
    if ((handle == NULL) || (symbol == NULL))
        return NULL;

    return dlsym(handle, symbol);
}

