/*
 * libpino - handler.c
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <pino.h>
#include <pino/handler.h>

#include "internal/common.h"

static struct {
    bool initialized;
    size_t capacity;
    size_t usage;
    handler_entry_t **entries;
} g_handlers;

static void *g_handler_context_entry;

static inline handler_entry_t *find_replacement_entry(handler_entry_t *entry)
{
    size_t i;

    if (!entry || !g_handlers.initialized) {
        return NULL;
    }

    for (i = 0; i < g_handlers.capacity; i++) {
        if (g_handlers.entries[i] && g_handlers.entries[i] != entry &&
            g_handlers.entries[i]->handler == entry->handler) {
            return g_handlers.entries[i];
        }
    }

    return NULL;
}

static inline void free_entry(handler_entry_t *entry)
{
    handler_entry_t *replacement;

    if (!entry) {
        return;
    }

    replacement = find_replacement_entry(entry);
    if (entry->handler && entry->handler->entry == entry) {
        entry->handler->entry = replacement;
    }

    pino_memory_manager_obj_free(&entry->mm);
    pfree(entry);
}

static inline bool glow_handlers(size_t step)
{
    handler_entry_t **entries;
    size_t i, new_capacity;

    if (step > SIZE_MAX - g_handlers.capacity) {
        return false;
    }
    new_capacity = g_handlers.capacity + step;

    if (new_capacity > SIZE_MAX / sizeof(handler_entry_t *)) {
        return false;
    }

    entries = (handler_entry_t **)prealloc(g_handlers.entries, new_capacity * sizeof(handler_entry_t *));
    if (!entries) {
        return false;
    }

    for (i = g_handlers.capacity; i < new_capacity; i++) {
        entries[i] = NULL;
    }

    g_handlers.entries = entries;
    g_handlers.capacity = new_capacity;

    return true;
}

extern bool pino_handler_init(size_t initialize_size)
{
    handler_entry_t **ents;
    size_t i;

    if (g_handlers.initialized) {
        return true;
    }

    g_handlers.initialized = false;
    g_handlers.capacity = initialize_size;
    g_handlers.usage = 0;

    ents = (handler_entry_t **)pcalloc(initialize_size, sizeof(handler_entry_t *));
    if (!ents) {
        return false;
    }

    for (i = 0; i < initialize_size; i++) {
        ents[i] = NULL;
    }

    g_handlers.entries = ents;

    return g_handlers.initialized = true;
}

extern void *pino_handler_context_set(void *entry)
{
    void *previous;

    previous = g_handler_context_entry;
    g_handler_context_entry = entry;

    return previous;
}

extern void *pino_handler_context_resolve(void *entry)
{
    return g_handler_context_entry ? g_handler_context_entry : entry;
}

extern void pino_handler_free(void)
{
    size_t i;
    handler_entry_t *entry;

    if (!g_handlers.initialized) {
        return;
    }

    for (i = 0; i < g_handlers.capacity; i++) {
        entry = g_handlers.entries[i];
        if (entry) {
            entry->unregistered = true;
            g_handlers.entries[i] = NULL;
            --g_handlers.usage;
            if (entry->refcount == 0) {
                free_entry(entry);
            }
        }
    }

    pfree(g_handlers.entries);

    g_handlers.entries = NULL;
    g_handlers.capacity = 0;
    g_handlers.usage = 0;
    g_handlers.initialized = false;
}

extern bool pino_handler_register(pino_magic_safe_t magic, pino_handler_t *handler)
{
    handler_entry_t *entry;
    size_t i;

    if (!g_handlers.initialized || !handler) {
        return false;
    }

    if (!handler->create || !handler->destroy || !handler->pack || !handler->unserialize || !handler->serialize_size ||
        !handler->serialize || !handler->unpack_size || !handler->unpack) {
        return false;
    }

    if (!validate_magic(magic)) {
        return false;
    }

    if (pino_handler_find_entry(magic)) {
        return false;
    }

    if (g_handlers.usage >= g_handlers.capacity) {
        if (g_handlers.capacity > SIZE_MAX - HANDLER_STEP) {
            return false;
        }

        if (!glow_handlers(HANDLER_STEP)) {
            return false;
        }
    }

    entry = (handler_entry_t *)pmalloc(sizeof(handler_entry_t));
    if (!entry) {
        return false;
    }

    if (!pino_memory_manager_obj_init(&entry->mm, MM_STEP)) {
        pfree(entry);
        return false;
    }

    pmemcpy(entry->magic, magic, sizeof(pino_magic_t));
    entry->handler = handler;
    entry->refcount = 0;
    entry->unregistered = false;
    handler->entry = entry;

    for (i = 0; i < g_handlers.capacity; i++) {
        if (!g_handlers.entries[i]) {
            g_handlers.entries[i] = entry;
            ++g_handlers.usage;

            return true;
        }
    }

    pino_memory_manager_obj_free(&entry->mm);
    handler->entry = NULL;
    pfree(entry);

    return false;
}

extern bool pino_handler_unregister(pino_magic_safe_t magic)
{
    size_t i;
    handler_entry_t *entry;

    if (!g_handlers.initialized) {
        return false;
    }

    if (!validate_magic(magic)) {
        return false;
    }

    for (i = 0; i < g_handlers.capacity; i++) {
        if (g_handlers.entries[i] && magic_equal(g_handlers.entries[i]->magic, magic)) {
            entry = g_handlers.entries[i];
            entry->unregistered = true;
            g_handlers.entries[i] = NULL;
            --g_handlers.usage;
            if (entry->refcount == 0) {
                free_entry(entry);
            }

            return true;
        }
    }

    return false;
}

extern handler_entry_t *pino_handler_find_entry(pino_magic_safe_t magic)
{
    size_t i;

    if (!g_handlers.initialized) {
        return NULL;
    }

    for (i = 0; i < g_handlers.capacity; i++) {
        if (g_handlers.entries[i] && magic_equal(g_handlers.entries[i]->magic, magic)) {
            return g_handlers.entries[i];
        }
    }

    return NULL;
}
