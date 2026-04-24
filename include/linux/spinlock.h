#pragma once

#include <libs/klibc.h>

#define spin_lock_init(lock) (*(lock) = SPIN_INIT)

static inline void spin_lock_irq(spinlock_t *lock) { spin_lock(lock); }
static inline void spin_lock_bh(spinlock_t *lock) { spin_lock(lock); }

static inline void spin_unlock_irq(spinlock_t *lock) { spin_unlock(lock); }
static inline void spin_unlock_bh(spinlock_t *lock) { spin_unlock(lock); }

static inline void spin_lock_irqsave(spinlock_t *lock, unsigned long flags) {
    (void)flags;
    spin_lock(lock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock,
                                          unsigned long flags) {
    (void)flags;
    spin_unlock(lock);
}

static inline void spin_lock_irqsave_nested(spinlock_t *lock,
                                            unsigned long flags, int subclass) {
    (void)subclass;
    spin_lock_irqsave(lock, flags);
}

static inline bool spin_is_locked(spinlock_t *lock) {
    return lock ? __atomic_load_n(&lock->lock, __ATOMIC_ACQUIRE) != 0 : false;
}

#define assert_spin_locked(lock) ((void)(lock))
