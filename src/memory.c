/*
 * libpino - memory.c
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <pino.h>
#include <pino/handler.h>

#include "internal/common.h"

static inline bool glow_mm(mm_t *mm, size_t step)
{
    void **ptrs;
    size_t i;

    ptrs = (void **)prealloc(mm->ptrs, (mm->capacity + step) * sizeof(void *));
    if (!ptrs) {
        return false;
    }

    for (i = mm->capacity; i < mm->capacity + step; i++) {
        ptrs[i] = NULL;
    }

    mm->ptrs = ptrs;
    mm->capacity += step;

    return true;
}

extern bool pino_memory_manager_obj_init(mm_t *mm, size_t initialize_size)
{
    if (initialize_size == 0) {
        return false;
    }

    mm->usage = 0;
    mm->capacity = 0;
    mm->ptrs = (void **)pcalloc(initialize_size, sizeof(void *));
    if (!mm->ptrs) {
        return false;
    }

    mm->usage = 0;
    mm->capacity = initialize_size;

    return true;
}

extern void pino_memory_manager_obj_free(mm_t *mm)
{
    size_t i;

    if (!mm) {
        return;
    }

    for (i = 0; i < mm->capacity; i++) {
        if (mm->ptrs[i]) {
            pfree(mm->ptrs[i]);
            mm->ptrs[i] = NULL;
            --mm->usage;
        }
    }

    pfree(mm->ptrs);
    mm->ptrs = NULL;
    mm->usage = 0;
    mm->capacity = 0;
}

extern void *pino_memory_manager_malloc(/* handler_entry_t */ void *entry, size_t size)
{
    size_t i;

    if (!entry || size == 0) {
        return NULL;
    }

    if (((handler_entry_t *)entry)->mm.usage >= ((handler_entry_t *)entry)->mm.capacity) {
        if (!glow_mm(&((handler_entry_t *)entry)->mm, HANDLER_STEP)) {
            return NULL;
        }
    }

    for (i = 0; i < ((handler_entry_t *)entry)->mm.capacity; i++) {
        if (!((handler_entry_t *)entry)->mm.ptrs[i]) {
            ((handler_entry_t *)entry)->mm.ptrs[i] = pmalloc(size);
            if (!((handler_entry_t *)entry)->mm.ptrs[i]) {
                return NULL;
            }

            ++((handler_entry_t *)entry)->mm.usage;

            return ((handler_entry_t *)entry)->mm.ptrs[i];
        }
    }

    return NULL;
}

extern void *pino_memory_manager_calloc(/* handler_entry_t */ void *entry, size_t count, size_t size)
{
    void *ptr;

    ptr = pino_memory_manager_malloc(entry, count * size);
    if (!ptr) {
        return NULL;
    }

    memset(ptr, 0, count * size);

    return ptr;
}

extern void pino_memory_manager_free(/* handler_entry_t */ void *entry, void *ptr)
{
    size_t i;

    if (!entry || !ptr) {
        return;
    }

    for (i = 0; i < ((handler_entry_t *)entry)->mm.capacity; i++) {
        if (((handler_entry_t *)entry)->mm.ptrs[i] && ((handler_entry_t *)entry)->mm.ptrs[i] == ptr) {
            pfree(ptr);
            ((handler_entry_t *)entry)->mm.ptrs[i] = NULL;
            ptr = NULL;
            --((handler_entry_t *)entry)->mm.usage;

            return;
        }
    }
}
