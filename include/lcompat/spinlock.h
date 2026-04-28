#pragma once

#include <libs/klibc.h>
#include <task/task.h>

#define spin_lock_init(lock) (*(lock) = SPIN_INIT)

static inline void lcompat_spin_lock(spinlock_t *lock) {
    if (current_task)
        current_task->preempt_count++;
    raw_spin_lock(lock);
}

static inline void lcompat_spin_unlock(spinlock_t *lock) {
    raw_spin_unlock(lock);
    if (current_task)
        current_task->preempt_count--;
}

static inline bool lcompat_spin_trylock(spinlock_t *lock) {
    if (!lock)
        return false;
    if (current_task)
        current_task->preempt_count++;
    if (__sync_bool_compare_and_swap(&lock->lock, 0, 1))
        return true;
    if (current_task)
        current_task->preempt_count--;
    return false;
}

#define spin_lock(lock) lcompat_spin_lock((lock))
#define spin_unlock(lock) lcompat_spin_unlock((lock))
#define spin_trylock(lock) lcompat_spin_trylock((lock))

static inline void spin_lock_irq(spinlock_t *lock) {
    arch_disable_interrupt();
    lcompat_spin_lock(lock);
}

static inline void spin_lock_bh(spinlock_t *lock) { lcompat_spin_lock(lock); }

static inline void spin_unlock_irq(spinlock_t *lock) {
    lcompat_spin_unlock(lock);
    arch_enable_interrupt();
}

static inline void spin_unlock_bh(spinlock_t *lock) {
    lcompat_spin_unlock(lock);
}

#define spin_lock_irqsave(lock, flags)                                         \
    do {                                                                       \
        (flags) = arch_interrupt_enabled();                                    \
        arch_disable_interrupt();                                              \
        lcompat_spin_lock((lock));                                             \
    } while (0)

#define spin_unlock_irqrestore(lock, flags)                                    \
    do {                                                                       \
        lcompat_spin_unlock((lock));                                           \
        if (flags)                                                             \
            arch_enable_interrupt();                                           \
    } while (0)

#define spin_lock_irqsave_nested(lock, flags, subclass)                        \
    do {                                                                       \
        (void)(subclass);                                                      \
        spin_lock_irqsave((lock), (flags));                                    \
    } while (0)

static inline bool spin_is_locked(spinlock_t *lock) {
    return lock ? __atomic_load_n(&lock->lock, __ATOMIC_ACQUIRE) != 0 : false;
}

#define assert_spin_locked(lock) ((void)(lock))
