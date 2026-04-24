#pragma once

#include <linux/slab.h>

struct idr {
    void **slots;
    int size;
};

static inline void idr_init(struct idr *idr) {
    if (!idr)
        return;
    idr->slots = NULL;
    idr->size = 0;
}

static inline void idr_destroy(struct idr *idr) {
    if (!idr)
        return;
    kfree(idr->slots);
    idr->slots = NULL;
    idr->size = 0;
}

static inline bool idr_is_empty(struct idr *idr) {
    if (!idr || !idr->slots)
        return true;
    for (int i = 0; i < idr->size; i++) {
        if (idr->slots[i])
            return false;
    }
    return true;
}

static inline int idr_alloc(struct idr *idr, void *ptr, int start, int end,
                            gfp_t gfp) {
    int limit, i;
    void **new_slots;

    (void)gfp;
    if (!idr)
        return -EINVAL;
    limit = end > start ? end : start + 1;
    if (limit > idr->size) {
        new_slots = krealloc(idr->slots, sizeof(void *) * limit, GFP_KERNEL);
        if (!new_slots)
            return -ENOMEM;
        for (i = idr->size; i < limit; i++)
            new_slots[i] = NULL;
        idr->slots = new_slots;
        idr->size = limit;
    }

    for (i = start; i < limit; i++) {
        if (!idr->slots[i]) {
            idr->slots[i] = ptr;
            return i;
        }
    }

    return -ENOSPC;
}

static inline void *idr_remove(struct idr *idr, int id) {
    void *ptr;

    if (!idr || id < 0 || id >= idr->size || !idr->slots)
        return NULL;
    ptr = idr->slots[id];
    idr->slots[id] = NULL;
    return ptr;
}

static inline void *idr_find(struct idr *idr, int id) {
    if (!idr || id < 0 || id >= idr->size || !idr->slots)
        return NULL;
    return idr->slots[id];
}

#define idr_for_each_entry(idr, entry, id)                                     \
    for ((id) = 0; (idr) && (id) < (idr)->size; (id)++)                        \
        if (((entry) = (idr)->slots[(id)]) != NULL)
