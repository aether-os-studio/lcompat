#pragma once

#include <linux/atomic.h>
#include <linux/spinlock.h>

typedef atomic_t refcount_t;

static inline void refcount_set(refcount_t *r, int n) { atomic_set(r, n); }
static inline unsigned int refcount_read(const refcount_t *r) {
    return (unsigned int)atomic_read((atomic_t *)r);
}
static inline void refcount_inc(refcount_t *r) { atomic_inc(r); }
static inline bool refcount_inc_not_zero(refcount_t *r) {
    return atomic_inc_not_zero(r);
}
static inline bool refcount_dec_and_test(refcount_t *r) {
    return atomic_dec_and_test(r);
}

static inline bool refcount_dec_and_lock_irqsave(refcount_t *r,
                                                 spinlock_t *lock,
                                                 unsigned long *flags) {
    if (!refcount_dec_and_test(r))
        return false;
    spin_lock_irqsave(lock, *flags);
    return true;
}
