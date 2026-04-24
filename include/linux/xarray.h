#pragma once

#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define XA_FLAGS_ALLOC 1U
#define XA_FLAGS_ALLOC1 2U
#define XA_FLAGS_LOCK_IRQ 4U

struct xa_limit {
    unsigned long min;
    unsigned long max;
};

static const struct xa_limit xa_limit_32b = {
    .min = 0,
    .max = 0xffffffffUL,
};

struct xarray {
    void **slots;
    unsigned long capacity;
    unsigned long next;
    spinlock_t xa_lock;
};

static inline void xa_init_flags(struct xarray *xa, unsigned int flags) {
    (void)flags;
    if (!xa)
        return;
    xa->slots = NULL;
    xa->capacity = 0;
    xa->next = 0;
    spin_lock_init(&xa->xa_lock);
}

static inline void xa_destroy(struct xarray *xa) {
    if (!xa)
        return;
    kfree(xa->slots);
    xa->slots = NULL;
    xa->capacity = 0;
    xa->next = 0;
}

static inline int xa_ensure_capacity(struct xarray *xa, unsigned long index) {
    if (index < xa->capacity)
        return 0;

    unsigned long new_capacity = xa->capacity ? xa->capacity : 16;
    while (new_capacity <= index)
        new_capacity *= 2;

    void **new_slots =
        krealloc(xa->slots, new_capacity * sizeof(*new_slots), GFP_KERNEL);
    if (!new_slots)
        return -1;

    for (unsigned long i = xa->capacity; i < new_capacity; i++)
        new_slots[i] = NULL;

    xa->slots = new_slots;
    xa->capacity = new_capacity;
    return 0;
}

static inline void *xa_load(struct xarray *xa, unsigned long index) {
    if (!xa || index >= xa->capacity)
        return NULL;
    return xa->slots[index];
}

static inline void *xa_store(struct xarray *xa, unsigned long index,
                             void *entry, gfp_t gfp) {
    (void)gfp;
    if (!xa || xa_ensure_capacity(xa, index))
        return (void *)-1L;
    void *old = xa->slots[index];
    xa->slots[index] = entry;
    return old;
}

static inline void *__xa_store(struct xarray *xa, unsigned long index,
                               void *entry, gfp_t gfp) {
    return xa_store(xa, index, entry, gfp);
}

static inline void *xa_erase(struct xarray *xa, unsigned long index) {
    if (!xa || index >= xa->capacity)
        return NULL;
    void *old = xa->slots[index];
    xa->slots[index] = NULL;
    return old;
}

static inline void *__xa_erase(struct xarray *xa, unsigned long index) {
    return xa_erase(xa, index);
}

static inline bool xa_is_err(const void *entry) { return (long)entry == -1L; }

static inline int xa_err(const void *entry) {
    return xa_is_err(entry) ? -1 : 0;
}

static inline int xa_alloc(struct xarray *xa, unsigned long *id, void *entry,
                           struct xa_limit limit, gfp_t gfp) {
    (void)gfp;
    if (!xa || !id)
        return -1;
    unsigned long idx = xa->next < limit.min ? limit.min : xa->next;
    while (idx <= limit.max && xa_load(xa, idx) != NULL)
        idx++;
    if (idx > limit.max || xa_ensure_capacity(xa, idx))
        return -1;
    xa->slots[idx] = entry;
    xa->next = idx + 1;
    *id = idx;
    return 0;
}

static inline int xa_alloc_cyclic_irq(struct xarray *xa, unsigned long *id,
                                      void *entry, struct xa_limit limit,
                                      unsigned long *next, gfp_t gfp) {
    if (next && xa)
        xa->next = *next;
    int ret = xa_alloc(xa, id, entry, limit, gfp);
    if (!ret && next)
        *next = xa->next;
    return ret;
}

static inline void xa_lock(struct xarray *xa) { (void)xa; }
static inline void xa_unlock(struct xarray *xa) { (void)xa; }
static inline void xa_lock_irq(struct xarray *xa) { (void)xa; }
static inline void xa_unlock_irq(struct xarray *xa) { (void)xa; }
static inline void xa_lock_irqsave(struct xarray *xa, unsigned long flags) {
    (void)xa;
    (void)flags;
}
static inline void xa_unlock_irqrestore(struct xarray *xa,
                                        unsigned long flags) {
    (void)xa;
    (void)flags;
}
static inline void xa_erase_irq(struct xarray *xa, unsigned long index) {
    (void)xa_erase(xa, index);
}

#define xa_mk_value(v) ((void *)((((unsigned long)(v)) << 1) | 1UL))
#define xa_to_value(v) (((unsigned long)(v)) >> 1)
#define xa_is_value(v) ((((unsigned long)(v)) & 1UL) != 0)

#define xa_for_each(xa, index, entry)                                          \
    for ((index) = 0; (xa) && (index) < (xa)->capacity; (index)++)             \
        if (((entry) = (xa)->slots[index]) != NULL)
