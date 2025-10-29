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
 * @brief File stream operations.
 */

#include "archi/file/api/stream.fun.h"

#include <fcntl.h> // for O_*

archi_file_stream_t
archi_file_stream_open(
        archi_file_descriptor_t fd,
        archi_file_open_params_t params,
        ARCHI_ERROR_PARAMETER_DECL)
{
    if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return NULL;
    }
    else if (!params.readable && !params.writable)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file is opened for neither reading nor writing");
        return NULL;
    }

    // Choose file stream mode
    const char *mode;
    {
        bool flag_RDWR = (params.flags & O_RDWR) || (params.readable && params.writable);
        bool flag_WRONLY = (params.flags & O_WRONLY) || params.writable;
        bool flag_RDONLY = (!(params.flags & O_RDWR) && !(params.flags & O_WRONLY)) || params.readable;

        if ((int)flag_RDWR + (int)flag_WRONLY + (int)flag_RDONLY != 1)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unsupported combination of fdopen() flags (RDWR + WRONLY + RDONLY != 1)");
            return NULL;
        }

        bool flag_CREAT = (params.flags & O_CREAT) || params.create;
        bool flag_TRUNC = (params.flags & O_TRUNC) || params.truncate;
        bool flag_APPEND = (params.flags & O_APPEND) || params.append;

        if (flag_RDONLY && !flag_CREAT && !flag_TRUNC && !flag_APPEND)
            mode = "rb";
        else if (flag_WRONLY && flag_CREAT && flag_TRUNC && !flag_APPEND)
            mode = "wb";
        else if (flag_WRONLY && flag_CREAT && !flag_TRUNC && flag_APPEND)
            mode = "ab";
        else if (flag_RDWR && !flag_CREAT && !flag_TRUNC && !flag_APPEND)
            mode = "r+b";
        else if (flag_RDWR && flag_CREAT && flag_TRUNC && !flag_APPEND)
            mode = "w+b";
        else if (flag_RDWR && flag_CREAT && !flag_TRUNC && flag_APPEND)
            mode = "a+b";
        else
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unsupported combination of fdopen() flags (CREAT/TRUNC/APPEND)");
            return NULL;
        }
    }

    // Create file stream
    archi_file_stream_t file = fdopen(fd, mode);
    if (file == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create file stream from file descriptor (%i)", fd);
        return NULL;
    }

    ARCHI_ERROR_RESET();
    return file;
}

void
archi_file_stream_close(
        archi_file_stream_t file,
        ARCHI_ERROR_PARAMETER_DECL)
{
    if (file == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file stream is NULL");
        return;
    }

    int ret = fclose(file);
    if (ret != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't close file stream");
        return;
    }

    ARCHI_ERROR_RESET();
}

archi_file_descriptor_t
archi_file_stream_descriptor(
        archi_file_stream_t file)
{
    return fileno(file);
}

