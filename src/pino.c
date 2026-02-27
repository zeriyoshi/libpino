/*
 * libpino - pino.c
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <pino.h>
#include <pino/handler.h>

#include "internal/common.h"

static inline pino_t *pino_create(pino_magic_safe_t magic, pino_handler_t *handler, size_t size)
{
    pino_t *pino;

    pino = pmalloc(sizeof(pino_t));
    if (!pino) {
        return NULL;
    }

    pmemcpy(pino->magic, magic, sizeof(pino_magic_t));
    pino->magic[sizeof(pino_magic_t)] = '\0';
    pino->static_fields_size = handler->static_fields_size;
    pino->static_fields = pcalloc(1, pino->static_fields_size);

    if (!pino->static_fields) {
        pfree(pino);

        return NULL;
    }

    pino->handler = handler;
    pino->this = handler->create(size, pino->static_fields);
    if (!pino->this) {
        pfree(pino->static_fields);
        pfree(pino);
        return NULL;
    }

    ((handler_entry_t *)handler->entry)->refcount++;

    return pino;
}

extern bool pino_init(void)
{
    return pino_handler_init(HANDLER_STEP);
}

extern void pino_free(void)
{
    pino_handler_free();
}

extern size_t pino_serialize_size(const pino_t *pino)
{
    size_t handler_size, total_size;

    if (!pino || !pino->handler || !pino->handler->serialize_size) {
        return 0;
    }

    handler_size = pino->handler->serialize_size(pino->this, pino->static_fields);
    if (handler_size > SIZE_MAX - sizeof(pino_magic_t) - sizeof(pino_static_fields_size_t) - pino->static_fields_size) {
        return 0;
    }

    total_size = handler_size + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t) + pino->static_fields_size;

    return total_size;
}

extern bool pino_serialize(const pino_t *pino, void *dest)
{
    if (!pino || !dest || !pino->handler || !pino->handler->serialize) {
        return false;
    }

    pmemcpy(dest, pino->magic, sizeof(pino_magic_t));
    pmemcpy_n2l(((char *)dest) + sizeof(pino_magic_t), &pino->static_fields_size, sizeof(pino_static_fields_size_t));

    /* fields always use LE */
    pmemcpy(((char *)dest) + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t), pino->static_fields,
            pino->static_fields_size);

    return pino->handler->serialize(pino->this, pino->static_fields,
                                    ((char *)dest) + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t) +
                                        pino->handler->static_fields_size);
}

extern pino_t *pino_unserialize(const void *src, size_t size)
{
    pino_t *pino;
    pino_handler_t *handler;
    pino_magic_safe_t magic;
    pino_static_fields_size_t fields_size;

    if (!src) {
        return NULL;
    }

    if (size < sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t)) {
        return NULL;
    }

    pmemcpy(magic, src, sizeof(pino_magic_t));
    pmemcpy_l2n(&fields_size, ((char *)src) + sizeof(pino_magic_t), sizeof(pino_static_fields_size_t));

    if (fields_size > size - sizeof(pino_magic_t) - sizeof(pino_static_fields_size_t)) {
        return NULL;
    }

    handler = pino_handler_find(magic);
    if (!handler) {
        return NULL;
    }

    if (fields_size != handler->static_fields_size) {
        return NULL;
    }

    pino = pino_create(magic, handler, size - sizeof(pino_magic_t) - sizeof(pino_static_fields_size_t) - fields_size);
    if (!pino) {
        return NULL;
    }

    /* always LE */
    pmemcpy(pino->static_fields, ((char *)src) + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t), fields_size);
    if (!handler->unserialize(pino->this, pino->static_fields,
                              ((char *)src) + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t) + fields_size,
                              size - sizeof(pino_magic_t) - sizeof(pino_static_fields_size_t) - fields_size)) {
        pino_destroy(pino);
        return NULL;
    }

    return pino;
}

extern pino_t *pino_pack(pino_magic_safe_t magic, const void *src, size_t size)
{
    pino_t *pino;
    pino_handler_t *handler;

    handler = pino_handler_find(magic);
    if (!handler) {
        return NULL;
    }

    pino = pino_create(magic, handler, size);
    if (!pino) {
        return NULL;
    }

    if (!handler->pack(pino->this, pino->static_fields, src, size)) {
        pino_destroy(pino);
        return NULL;
    }

    return pino;
}

extern size_t pino_unpack_size(const pino_t *pino)
{
    if (!pino || !pino->handler || !pino->handler->unpack_size) {
        return 0;
    }

    return pino->handler->unpack_size(pino->this, pino->static_fields);
}

extern bool pino_unpack(const pino_t *pino, void *dest)
{
    if (!pino || !dest || !pino->handler || !pino->handler->unpack) {
        return false;
    }

    return pino->handler->unpack(pino->this, pino->static_fields, dest);
}

extern void pino_destroy(pino_t *pino)
{
    handler_entry_t *entry;

    if (!pino) {
        return;
    }

    entry = (handler_entry_t *)pino->handler->entry;

    if (pino->this) {
        pino->handler->destroy(pino->this, pino->static_fields);
    }

    if (pino->static_fields) {
        pfree(pino->static_fields);
    }

    pfree(pino);

    if (entry) {
        entry->refcount--;
        if (entry->refcount == 0 && entry->unregistered) {
            pino_memory_manager_obj_free(&entry->mm);
            entry->handler->entry = NULL;
            pfree(entry);
        }
    }
}

extern uint32_t pino_version_id()
{
    return (uint32_t)PINO_VERSION_ID;
}

extern pino_buildtime_t pino_buildtime()
{
    return (pino_buildtime_t)PINO_BUILDTIME;
}
