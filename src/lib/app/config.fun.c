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
 * @brief Application configuration functions.
 */

#include "archi/app/config.fun.h"
#include "archi/app/context.fun.h"
#include "archi/app/instance.typ.h"
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h>

archi_status_t
archi_app_add_context(
        archi_application_t *app,
        const char *key,
        const archi_context_interface_t *interface,
        const struct archi_list_node_named_value *config)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;
    else if ((key == NULL) || (interface == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *context = malloc(sizeof(*context));
    if (context == NULL)
        return ARCHI_ERROR_ALLOC;

    *context = (archi_context_t){.interface = interface};

    code = archi_context_initialize(context, config);
    if (code != 0)
    {
        free(context);
        return code;
    }

    code = archi_container_insert(app->contexts, key, context);
    if (code != 0)
    {
        archi_context_finalize(context);
        free(context);
        return code;
    }

    return 0;
}

archi_status_t
archi_app_remove_context(
        archi_application_t *app,
        const char *key)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (key == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *context = NULL;

    code = archi_container_remove(app->contexts, key, (void**)&context);
    if (code != 0)
        return code;

    if (context == NULL)
        return ARCHI_ERROR_CONFIG;

    archi_context_finalize(context);
    free(context);

    return 0;
}

/*****************************************************************************/

static
archi_status_t
archi_app_do_config_step_init(
        archi_application_t *app,

        const char *context_key,
        archi_app_config_step_init_t step_init)
{
    if (context_key == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    const archi_context_interface_t *interface = NULL;

    code = archi_container_extract(app->interfaces,
            step_init.interface_key, (void**)&interface);
    if (code != 0)
        return code;

    return archi_app_add_context(app, context_key, interface, step_init.config);
}

static
archi_status_t
archi_app_do_config_step_final(
        archi_application_t *app,

        const char *context_key)
{
    if (context_key == NULL)
        return ARCHI_ERROR_MISUSE;

    return archi_app_remove_context(app, context_key);
}

static
archi_status_t
archi_app_do_config_step_set(
        archi_application_t *app,

        const char *context_key,
        archi_app_config_step_set_t step_set)
{
    if ((context_key == NULL) || (step_set.slot == NULL) || (step_set.value == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *context = NULL;

    code = archi_container_extract(app->contexts, context_key, (void**)&context);
    if (code != 0)
        return code;

    if (context == NULL)
        return ARCHI_ERROR_CONFIG;

    code = archi_context_set(context, step_set.slot, step_set.value);
    if (code != 0)
        return code;

    return 0;
}

static
archi_status_t
archi_app_do_config_step_assign(
        archi_application_t *app,

        const char *context_key,
        archi_app_config_step_assign_t step_assign)
{
    if ((context_key == NULL) || (step_assign.slot == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (step_assign.source_key == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *dest_context = NULL, *src_context = NULL;

    code = archi_container_extract(app->contexts, context_key, (void**)&dest_context);
    if (code != 0)
        return code;

    code = archi_container_extract(app->contexts, step_assign.source_key, (void**)&src_context);
    if (code != 0)
        return code;

    if ((dest_context == NULL) || (src_context == NULL))
        return ARCHI_ERROR_CONFIG;

    code = archi_context_assign(dest_context, step_assign.slot,
            src_context, step_assign.source_slot);
    if (code != 0)
        return code;

    return 0;
}

static
archi_status_t
archi_app_do_config_step_act(
        archi_application_t *app,

        const char *context_key,
        archi_app_config_step_act_t step_act)
{
    if ((context_key == NULL) || (step_act.action == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *context = NULL;

    code = archi_container_extract(app->contexts, context_key, (void**)&context);
    if (code != 0)
        return code;

    if (context == NULL)
        return ARCHI_ERROR_CONFIG;

    code = archi_context_act(context, step_act.action, step_act.params);
    if (code != 0)
        return code;

    return 0;
}

archi_status_t
archi_app_do_config_step(
        archi_application_t *app,
        archi_app_config_step_t step)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;

    switch (step.type)
    {
        case ARCHI_APP_CONFIG_STEP_INIT:
            return archi_app_do_config_step_init(app, step.key, step.as_init);

        case ARCHI_APP_CONFIG_STEP_FINAL:
            return archi_app_do_config_step_final(app, step.key);

        case ARCHI_APP_CONFIG_STEP_SET:
            return archi_app_do_config_step_set(app, step.key, step.as_set);

        case ARCHI_APP_CONFIG_STEP_ASSIGN:
            return archi_app_do_config_step_assign(app, step.key, step.as_assign);

        case ARCHI_APP_CONFIG_STEP_ACT:
            return archi_app_do_config_step_act(app, step.key, step.as_act);

        default:
            return ARCHI_ERROR_CONFIG;
    }
}

archi_status_t
archi_app_undo_config_step(
        archi_application_t *app,
        archi_app_config_step_t step)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;

    switch (step.type)
    {
        case ARCHI_APP_CONFIG_STEP_INIT:
            return archi_app_do_config_step_final(app, step.key);

        default:
            return 0;
    }
}

bool
archi_app_config_step_undoable(
        archi_app_config_step_type_t type)
{
    switch (type)
    {
        case ARCHI_APP_CONFIG_STEP_INIT:
            return true;

        default:
            return false;
    }
}

