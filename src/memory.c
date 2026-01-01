/*
 * libpino - memory.c
 * 
 */

#include <pino.h>
#include <pino/handler.h>

#include <pino_internal.h>

static inline bool glow_mm(mm_t *mm, size_t step)
{
    void **ptrs;
    size_t i;

    ptrs = (void **)prealloc(mm->ptrs, (mm->capacity + step) * sizeof(void *));
    /* LCOV_EXCL_START */
    if (!ptrs) {
        PINO_SUPRTF("prealloc failed");
        return false;
    }
    /* LCOV_EXCL_STOP */

    for (i = mm->capacity; i < mm->capacity + step; i++) {
        ptrs[i] = NULL;
    }

    mm->ptrs = ptrs;
    mm->capacity += step;

    PINO_SUPRTF("glowed capacity: %zu, usage: %zu", mm->capacity, mm->usage);

    return true;
}

extern bool pino_memory_manager_obj_init(mm_t *mm, size_t initialize_size)
{
    /* LCOV_EXCL_START */
    if (initialize_size == 0) {
        return false;
    }
    /* LCOV_EXCL_STOP */

    mm->usage = 0;
    mm->capacity = 0;
    mm->ptrs = (void **)pcalloc(initialize_size, sizeof(void *));
    /* LCOV_EXCL_START */
    if (!mm->ptrs) {
        return false;
    }
    /* LCOV_EXCL_STOP */

    mm->usage = 0;
    mm->capacity = initialize_size;

    PINO_SUPRTF("usage: %zu, capacity: %zu", mm->usage, mm->capacity);

    return mm;
}

extern void pino_memory_manager_obj_free(mm_t *mm)
{
    size_t i;

    if (!mm) {
        return; /* LCOV_EXCL_LINE */
    }

    for (i = 0; i < mm->capacity; i++) {
        if (mm->ptrs[i]) {
            pfree(mm->ptrs[i]);
            mm->ptrs[i] = NULL;
            --mm->usage;

            PINO_SUPRTF("freeing: %zu, usage: %zu, capacity: %zu", i, mm->usage, mm->capacity);
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
        PINO_SUPRTF("entry or size is NULL");
        return NULL;
    }

    if (((handler_entry_t *)entry)->mm.usage >= ((handler_entry_t *)entry)->mm.capacity) {
        /* LCOV_EXCL_START */
        if (!glow_mm(&((handler_entry_t *)entry)->mm, HANDLER_STEP)) {
            PINO_SUPRTF("glow_mm failed");
            return NULL;
        }
        /* LCOV_EXCL_STOP */
    }

    for (i = 0; i < ((handler_entry_t *)entry)->mm.capacity; i++) {
        if (!((handler_entry_t *)entry)->mm.ptrs[i]) {
            ((handler_entry_t *)entry)->mm.ptrs[i] = pmalloc(size);
            /* LCOV_EXCL_START */
            if (!((handler_entry_t *)entry)->mm.ptrs[i]) {
                PINO_SUPRTF("pmalloc failed");
                return NULL;
            }
            /* LCOV_EXCL_STOP */

            ++((handler_entry_t *)entry)->mm.usage;

            PINO_SUPRTF("magic: %.4s, using: %zu, usage: %zu, capacity: %zu",
                ((handler_entry_t *)entry)->magic,
                i,
                ((handler_entry_t *)entry)->mm.usage,
                ((handler_entry_t *)entry)->mm.capacity
            );

            return ((handler_entry_t *)entry)->mm.ptrs[i];
        }
    }
    /* LCOV_EXCL_START */
    PINO_SUPUNREACH();
    return NULL;
    /* LCOV_EXCL_STOP */
}

extern void *pino_memory_manager_calloc(/* handler_entry_t */ void *entry, size_t count, size_t size)
{
    void *ptr;

    ptr = pino_memory_manager_malloc(entry, count * size);
    /* LCOV_EXCL_START */
    if (!ptr) {
        PINO_SUPRTF("pino_memory_manager_malloc failed");
        return NULL;
    }
    /* LCOV_EXCL_STOP */

    memset(ptr, 0, count * size);

    return ptr;
}

extern void pino_memory_manager_free(/* handler_entry_t */ void *entry, void *ptr)
{
    size_t i;

    if (!entry || !ptr) {
        PINO_SUPRTF("mm or ptr is NULL");
        return;
    }

    for (i = 0; i < ((handler_entry_t *)entry)->mm.capacity; i++) {
        if (((handler_entry_t *)entry)->mm.ptrs[i] && ((handler_entry_t *)entry)->mm.ptrs[i] == ptr) {
            pfree(ptr);
            ((handler_entry_t *)entry)->mm.ptrs[i] = NULL;
            ptr = NULL;
            --((handler_entry_t *)entry)->mm.usage;

            PINO_SUPRTF("freeing: %zu, usage: %zu, capacity: %zu",
                i, ((handler_entry_t *)entry)->mm.usage, ((handler_entry_t *)entry)->mm.capacity);

            return;
        }
    }

    /* LCOV_EXCL_START */
    PINO_SUPUNREACH();
    return;
    /* LCOV_EXCL_STOP */
}
