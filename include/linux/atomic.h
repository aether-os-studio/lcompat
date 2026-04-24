#pragma once

#include <libs/klibc.h>

static inline int atomic_add_return(int i, atomic_t *v) {
    return __sync_add_and_fetch(&v->counter, i);
}

static inline int atomic_sub_return(int i, atomic_t *v) {
    return __sync_sub_and_fetch(&v->counter, i);
}

static inline int atomic_inc_return(atomic_t *v) {
    return atomic_add_return(1, v);
}
static inline int atomic_dec_return(atomic_t *v) {
    return atomic_sub_return(1, v);
}

static inline int atomic_fetch_inc(atomic_t *v) {
    return __sync_fetch_and_add(&v->counter, 1);
}

static inline int atomic_fetch_dec(atomic_t *v) {
    return __sync_fetch_and_sub(&v->counter, 1);
}

static inline int atomic_cmpxchg(atomic_t *v, int old, int new_value) {
    return __sync_val_compare_and_swap(&v->counter, old, new_value);
}

static inline bool atomic_add_unless(atomic_t *v, int add, int unless) {
    int old;

    do {
        old = atomic_read(v);
        if (old == unless)
            return false;
    } while (atomic_cmpxchg(v, old, old + add) != old);

    return true;
}

static inline bool atomic_inc_not_zero(atomic_t *v) {
    return atomic_add_unless(v, 1, 0);
}

static inline void atomic_set_release(atomic_t *v, int i) {
    if (v)
        __atomic_store_n(&v->counter, i, __ATOMIC_RELEASE);
}
