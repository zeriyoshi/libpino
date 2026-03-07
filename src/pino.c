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

static inline pino_t *pino_create(pino_magic_safe_t magic, handler_entry_t *entry, size_t size)
{
    pino_t *pino;
    pino_handler_t *handler;
    void *previous_entry;

    if (!entry || !entry->handler) {
        return NULL;
    }

    handler = entry->handler;

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
    pino->entry = entry;
    previous_entry = pino_handler_context_set(entry);
    pino->this = handler->create(size, pino->static_fields);
    pino_handler_context_set(previous_entry);
    if (!pino->this) {
        pfree(pino->static_fields);
        pfree(pino);
        return NULL;
    }

    entry->refcount++;

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
    void *previous_entry;

    if (!pino || !pino->handler || !pino->handler->serialize_size) {
        return 0;
    }

    previous_entry = pino_handler_context_set(pino->entry);
    handler_size = pino->handler->serialize_size(pino->this, pino->static_fields);
    pino_handler_context_set(previous_entry);
    if (handler_size > SIZE_MAX - sizeof(pino_magic_t) - sizeof(pino_static_fields_size_t) - pino->static_fields_size) {
        return 0;
    }

    total_size = handler_size + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t) + pino->static_fields_size;

    return total_size;
}

extern bool pino_serialize(const pino_t *pino, void *dest)
{
    bool result;
    void *previous_entry;

    if (!pino || !dest || !pino->handler || !pino->handler->serialize) {
        return false;
    }

    pmemcpy(dest, pino->magic, sizeof(pino_magic_t));
    pmemcpy_n2l(((char *)dest) + sizeof(pino_magic_t), &pino->static_fields_size, sizeof(pino_static_fields_size_t));

    /* fields always use LE */
    pmemcpy(((char *)dest) + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t), pino->static_fields,
            pino->static_fields_size);

    previous_entry = pino_handler_context_set(pino->entry);
    result = pino->handler->serialize(pino->this, pino->static_fields,
                                      ((char *)dest) + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t) +
                                          pino->handler->static_fields_size);
    pino_handler_context_set(previous_entry);

    return result;
}

extern pino_t *pino_unserialize(const void *src, size_t size)
{
    pino_t *pino;
    handler_entry_t *entry;
    pino_handler_t *handler;
    pino_magic_safe_t magic;
    pino_static_fields_size_t fields_size;
    bool result;
    void *previous_entry;

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

    entry = pino_handler_find_entry(magic);
    if (!entry || !entry->handler) {
        return NULL;
    }

    handler = entry->handler;

    if (fields_size != handler->static_fields_size) {
        return NULL;
    }

    pino = pino_create(magic, entry, size - sizeof(pino_magic_t) - sizeof(pino_static_fields_size_t) - fields_size);
    if (!pino) {
        return NULL;
    }

    /* always LE */
    pmemcpy(pino->static_fields, ((char *)src) + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t), fields_size);
    previous_entry = pino_handler_context_set(pino->entry);
    result =
        handler->unserialize(pino->this, pino->static_fields,
                             ((char *)src) + sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t) + fields_size,
                             size - sizeof(pino_magic_t) - sizeof(pino_static_fields_size_t) - fields_size);
    pino_handler_context_set(previous_entry);
    if (!result) {
        pino_destroy(pino);
        return NULL;
    }

    return pino;
}

extern pino_t *pino_pack(pino_magic_safe_t magic, const void *src, size_t size)
{
    pino_t *pino;
    handler_entry_t *entry;
    pino_handler_t *handler;
    bool result;
    void *previous_entry;

    entry = pino_handler_find_entry(magic);
    if (!entry || !entry->handler) {
        return NULL;
    }

    handler = entry->handler;

    pino = pino_create(magic, entry, size);
    if (!pino) {
        return NULL;
    }

    previous_entry = pino_handler_context_set(pino->entry);
    result = handler->pack(pino->this, pino->static_fields, src, size);
    pino_handler_context_set(previous_entry);
    if (!result) {
        pino_destroy(pino);
        return NULL;
    }

    return pino;
}

extern size_t pino_unpack_size(const pino_t *pino)
{
    size_t size;
    void *previous_entry;

    if (!pino || !pino->handler || !pino->handler->unpack_size) {
        return 0;
    }

    previous_entry = pino_handler_context_set(pino->entry);
    size = pino->handler->unpack_size(pino->this, pino->static_fields);
    pino_handler_context_set(previous_entry);

    return size;
}

extern bool pino_unpack(const pino_t *pino, void *dest)
{
    bool result;
    void *previous_entry;

    if (!pino || !dest || !pino->handler || !pino->handler->unpack) {
        return false;
    }

    previous_entry = pino_handler_context_set(pino->entry);
    result = pino->handler->unpack(pino->this, pino->static_fields, dest);
    pino_handler_context_set(previous_entry);

    return result;
}

extern void pino_destroy(pino_t *pino)
{
    handler_entry_t *entry;
    void *previous_entry;

    if (!pino) {
        return;
    }

    entry = (handler_entry_t *)pino->entry;

    if (pino->this) {
        previous_entry = pino_handler_context_set(entry);
        pino->handler->destroy(pino->this, pino->static_fields);
        pino_handler_context_set(previous_entry);
    }

    if (pino->static_fields) {
        pfree(pino->static_fields);
    }

    pfree(pino);

    if (entry) {
        entry->refcount--;
        if (entry->refcount == 0 && entry->unregistered) {
            pino_memory_manager_obj_free(&entry->mm);
            if (entry->handler && entry->handler->entry == entry) {
                entry->handler->entry = NULL;
            }
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
