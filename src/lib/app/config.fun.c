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
#include "archi/app/config.typ.h"
#include "archi/app/context.fun.h"
#include "archi/util/container.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h>

static
archi_status_t
archi_app_configure_step_init(
        archi_container_t application,

        const archi_app_config_step_init_t *step_init,
        archi_container_t context_interfaces)
{
    if ((step_init->key == NULL) || (((const char*)step_init->key)[0] == '\0'))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *context = malloc(sizeof(*context));
    if (context == NULL)
        return ARCHI_ERROR_ALLOC;

    code = archi_container_extract(context_interfaces,
            step_init->interface_key, (void**)&context->interface);
    if (code != 0)
        goto failure;

    code = archi_context_initialize(context, (archi_container_t){
            .data = (void*)step_init->config,
            .interface = &archi_list_container_interface});
    if (code != 0)
        goto failure;

    code = archi_container_insert(application, step_init->key, context);
    if (code != 0)
    {
        archi_context_finalize(context);
        goto failure;
    }

    return 0;

failure:
    free(context);
    return code;
}

static
archi_status_t
archi_app_configure_step_final(
        archi_container_t application,

        const archi_app_config_step_final_t *step_final)
{
    if ((step_final->key == NULL) || (((const char*)step_final->key)[0] == '\0'))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *context = NULL;

    code = archi_container_remove(application, step_final->key, (void**)&context);
    if (code != 0)
        return code;

    if (context == NULL)
        return ARCHI_ERROR_CONFIG;

    archi_context_finalize(context);
    free(context);

    return 0;
}

static
archi_status_t
archi_app_configure_step_set(
        archi_container_t application,

        const archi_app_config_step_set_t *step_set)
{
    if ((step_set->key == NULL) || (step_set->slot == NULL) || (step_set->value == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *context = NULL;

    code = archi_container_extract(application, step_set->key, (void**)&context);
    if (code != 0)
        return code;

    if (context == NULL)
        return ARCHI_ERROR_CONFIG;

    code = archi_context_set(*context, step_set->slot, step_set->value);
    if (code != 0)
        return code;

    return 0;
}

static
archi_status_t
archi_app_configure_step_assign(
        archi_container_t application,

        const archi_app_config_step_assign_t *step_assign)
{
    if ((step_assign->destination.key == NULL) || (step_assign->destination.slot == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (step_assign->source.key == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *dest_context = NULL, *src_context = NULL;

    code = archi_container_extract(application, step_assign->destination.key, (void**)&dest_context);
    if (code != 0)
        return code;

    code = archi_container_extract(application, step_assign->source.key, (void**)&src_context);
    if (code != 0)
        return code;

    if ((dest_context == NULL) || (src_context == NULL))
        return ARCHI_ERROR_CONFIG;

    code = archi_context_assign(*dest_context, step_assign->destination.slot,
            *src_context, step_assign->source.slot);
    if (code != 0)
        return code;

    return 0;
}

static
archi_status_t
archi_app_configure_step_act(
        archi_container_t application,

        const archi_app_config_step_act_t *step_act)
{
    if ((step_act->key == NULL) || (step_act->action == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_context_t *context = NULL;

    code = archi_container_extract(application, step_act->key, (void**)&context);
    if (code != 0)
        return code;

    if (context == NULL)
        return ARCHI_ERROR_CONFIG;

    code = archi_context_act(*context, step_act->action, (archi_container_t){
            .data = (void*)step_act->params,
            .interface = &archi_list_container_interface});
    if (code != 0)
        return code;

    return 0;
}

ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_apply_config_step)
{
    (void) key;

    if ((element == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    const archi_app_config_step_t *step = element;
    archi_app_apply_config_step_data_t *func_data = data;

    switch (step->type)
    {
        case ARCHI_APP_CONFIG_STEP_INIT:
            return archi_app_configure_step_init(func_data->contexts,
                    &step->as_init, func_data->context_interfaces);

        case ARCHI_APP_CONFIG_STEP_FINAL:
            return archi_app_configure_step_final(func_data->contexts, &step->as_final);

        case ARCHI_APP_CONFIG_STEP_SET:
            return archi_app_configure_step_set(func_data->contexts, &step->as_set);

        case ARCHI_APP_CONFIG_STEP_ASSIGN:
            return archi_app_configure_step_assign(func_data->contexts, &step->as_assign);

        case ARCHI_APP_CONFIG_STEP_ACT:
            return archi_app_configure_step_act(func_data->contexts, &step->as_act);

        default:
            return ARCHI_ERROR_MISUSE;
    }
}

