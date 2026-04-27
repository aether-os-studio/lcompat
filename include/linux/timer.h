#pragma once

#include <linux/completion.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/seqlock.h>
#include <task/task.h>

#define timer_container_of(var, timer, member)                                 \
    container_of(timer, typeof(*var), member)

struct timer_list;
typedef void (*timer_func_t)(struct timer_list *timer);

struct timer_list {
    timer_func_t function;
    unsigned long expires;
    unsigned int generation;
    bool active;
    bool running;
};

static void lcompat_timer_worker(uint64_t arg) {
    struct timer_list *timer = (struct timer_list *)arg;
    unsigned int generation;
    unsigned long expires;

    if (!timer)
        return;

    generation = __atomic_load_n(&timer->generation, __ATOMIC_ACQUIRE);
    expires = __atomic_load_n(&timer->expires, __ATOMIC_ACQUIRE);

    while (__atomic_load_n(&timer->active, __ATOMIC_ACQUIRE) &&
           __atomic_load_n(&timer->generation, __ATOMIC_ACQUIRE) ==
               generation &&
           time_before(lcompat_jiffies_refresh(), expires)) {
        schedule(SCHED_FLAG_YIELD);
    }

    if (!__atomic_load_n(&timer->active, __ATOMIC_ACQUIRE) ||
        __atomic_load_n(&timer->generation, __ATOMIC_ACQUIRE) != generation) {
        return;
    }

    __atomic_store_n(&timer->running, true, __ATOMIC_RELEASE);
    __atomic_store_n(&timer->active, false, __ATOMIC_RELEASE);

    if (__atomic_load_n(&timer->generation, __ATOMIC_ACQUIRE) == generation &&
        timer->function) {
        timer->function(timer);
    }

    __atomic_store_n(&timer->running, false, __ATOMIC_RELEASE);
}

static inline void timer_setup(struct timer_list *timer, timer_func_t func,
                               unsigned int flags) {
    (void)flags;
    if (!timer)
        return;
    timer->function = func;
    timer->expires = 0;
    timer->generation = 0;
    timer->active = false;
    timer->running = false;
}

static inline int mod_timer(struct timer_list *timer, unsigned long expires) {
    unsigned int generation;
    bool pending;

    if (!timer)
        return 0;

    pending = __atomic_load_n(&timer->active, __ATOMIC_ACQUIRE);
    __atomic_store_n(&timer->expires, expires, __ATOMIC_RELEASE);
    __atomic_store_n(&timer->active, true, __ATOMIC_RELEASE);
    generation = __atomic_add_fetch(&timer->generation, 1, __ATOMIC_ACQ_REL);

    if (!task_create("lcompat_timer", lcompat_timer_worker, (uint64_t)timer,
                     KTHREAD_PRIORITY)) {
        __atomic_store_n(&timer->active, false, __ATOMIC_RELEASE);
        __atomic_add_fetch(&timer->generation, 1, __ATOMIC_ACQ_REL);
        return pending ? 1 : 0;
    }

    return pending ? 1 : 0;
}

static inline void add_timer(struct timer_list *timer) {
    if (timer)
        mod_timer(timer, timer->expires);
}

static inline int del_timer_sync(struct timer_list *timer) {
    bool pending;

    if (!timer)
        return 0;

    pending = __atomic_load_n(&timer->active, __ATOMIC_ACQUIRE) ||
              __atomic_load_n(&timer->running, __ATOMIC_ACQUIRE);
    __atomic_store_n(&timer->active, false, __ATOMIC_RELEASE);
    __atomic_add_fetch(&timer->generation, 1, __ATOMIC_ACQ_REL);

    while (__atomic_load_n(&timer->running, __ATOMIC_ACQUIRE))
        schedule(SCHED_FLAG_YIELD);

    __atomic_store_n(&timer->expires, 0, __ATOMIC_RELEASE);
    return pending ? 1 : 0;
}

static inline int timer_delete_sync(struct timer_list *timer) {
    return del_timer_sync(timer);
}
