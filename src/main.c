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
 * @brief Default main() implementation.
 */

#include "archi/exe/main.fun.h"
#include "archi/exe/args.fun.h"
#include "archi/exe/args.typ.h"
#include "archi/app/instance.typ.h"
#include "archi/app/interface.fun.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"

#include <stdlib.h> // for atexit()
#include <errno.h> // for error codes

static archi_application_t app;

static
void
exit_cleanup(void) // will be called on exit(), or if archi_main() returns
{
    archi_application_finalize(&app); // the function is idempotent,
                                      // so the possible second invokation here
                                      // after the one in archi_main() is not a problem
}

int
main(
        int argc,
        char *argv[])
{
    archi_log_set_start_time();

    archi_cmdline_args_t args;
    switch (archi_parse_cmdline_args(&args, argc, argv))
    {
        case 0: // success
            break;

        case EINVAL: // invalid arguments
            return ARCHI_ERROR_MISUSE;
        case ENOMEM: // memory allocation error
            return ARCHI_ERROR_ALLOC;
        default: // unknown error
            return ARCHI_ERROR_UNKNOWN;
    }

    archi_log_set_verbosity(args.verbosity_level);

    atexit(exit_cleanup);

    return archi_main(&app, &args);
}

