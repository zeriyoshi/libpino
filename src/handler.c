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

static inline bool glow_handlers(size_t step)
{
    handler_entry_t **entries;
    size_t i;

    entries =
        (handler_entry_t **)prealloc(g_handlers.entries, (g_handlers.capacity + step) * sizeof(handler_entry_t *));
    if (!entries) {
        return false;
    }

    for (i = g_handlers.capacity; i < g_handlers.capacity + step; i++) {
        entries[i] = NULL;
    }

    g_handlers.entries = entries;
    g_handlers.capacity += step;

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

extern void pino_handler_free(void)
{
    size_t i, j;

    if (!g_handlers.initialized) {
        return;
    }

    for (i = 0; i < g_handlers.capacity; i++) {
        if (g_handlers.entries[i]) {
            pino_memory_manager_obj_free(&g_handlers.entries[i]->mm);
            pfree(g_handlers.entries[i]);
            g_handlers.entries[i] = NULL;
            --g_handlers.usage;
        }
    }

    pfree(g_handlers.entries);

    g_handlers.initialized = false;
}

extern bool pino_handler_register(pino_magic_safe_t magic, pino_handler_t *handler)
{
    handler_entry_t *entry;
    size_t i;

    if (!handler) {
        return false;
    }

    if (!validate_magic(magic)) {
        return false;
    }

    if (pino_handler_find(magic)) {
        return false;
    }

    if (g_handlers.usage >= g_handlers.capacity) {
        if (!glow_handlers(g_handlers.capacity + HANDLER_STEP)) {
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
    handler->entry = entry;

    for (i = 0; i < g_handlers.capacity; i++) {
        if (!g_handlers.entries[i]) {
            g_handlers.entries[i] = entry;
            ++g_handlers.usage;

            return true;
        }
    }

    return false;
}

extern bool pino_handler_unregister(pino_magic_safe_t magic)
{
    size_t i;

    if (!g_handlers.initialized) {
        return false;
    }

    if (!validate_magic(magic)) {
        return false;
    }

    for (i = 0; i < g_handlers.capacity; i++) {
        if (g_handlers.entries[i] && magic_equal(g_handlers.entries[i]->magic, magic)) {
            pino_memory_manager_obj_free(&g_handlers.entries[i]->mm);
            pfree(g_handlers.entries[i]);
            g_handlers.entries[i] = NULL;
            --g_handlers.usage;

            return true;
        }
    }

    return false;
}

extern pino_handler_t *pino_handler_find(pino_magic_safe_t magic)
{
    size_t i;

    if (!g_handlers.initialized) {
        return NULL;
    }

    for (i = 0; i < g_handlers.capacity; i++) {
        if (g_handlers.entries[i] && magic_equal(g_handlers.entries[i]->magic, magic)) {
            return g_handlers.entries[i]->handler;
        }
    }

    return NULL;
}
