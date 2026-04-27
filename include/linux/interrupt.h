#pragma once

#include <linux/atomic.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <task/task.h>

#define TASKLET_STATE_SCHED 0
#define TASKLET_STATE_RUN 1
typedef int irqreturn_t;
#define IRQ_NONE 0
#define IRQ_HANDLED 1

struct tasklet_struct;
typedef void (*tasklet_callback_t)(struct tasklet_struct *t);

struct tasklet_struct {
    unsigned long state;
    atomic_t count;
    tasklet_callback_t callback;
    void (*callback_old)(unsigned long);
    unsigned long data;
    bool use_callback_data;
};

#define from_tasklet(var, callback_tasklet, tasklet_field)                     \
    container_of(callback_tasklet, typeof(*(var)), tasklet_field)

static inline void lcompat_tasklet_invoke(struct tasklet_struct *t) {
    if (!t)
        return;

    if (t->use_callback_data) {
        if (t->callback_old)
            t->callback_old(t->data);
        return;
    }

    if (t->callback)
        t->callback(t);
}

static void lcompat_tasklet_worker(uint64_t arg) {
    struct tasklet_struct *t = (struct tasklet_struct *)arg;

    if (!t)
        return;

    for (;;) {
        while (atomic_read(&t->count) > 0) {
            if (!test_bit(TASKLET_STATE_SCHED, &t->state))
                return;
            schedule(SCHED_FLAG_YIELD);
        }

        if (test_and_set_bit(TASKLET_STATE_RUN, &t->state)) {
            schedule(SCHED_FLAG_YIELD);
            continue;
        }

        if (!test_and_clear_bit(TASKLET_STATE_SCHED, &t->state)) {
            clear_bit(TASKLET_STATE_RUN, &t->state);
            return;
        }

        lcompat_tasklet_invoke(t);
        clear_bit(TASKLET_STATE_RUN, &t->state);

        if (!test_bit(TASKLET_STATE_SCHED, &t->state))
            return;
    }
}

static inline void tasklet_setup(struct tasklet_struct *t,
                                 tasklet_callback_t cb) {
    if (!t)
        return;
    t->state = 0;
    atomic_set(&t->count, 0);
    t->callback = cb;
    t->callback_old = NULL;
    t->data = 0;
    t->use_callback_data = false;
}

static inline void tasklet_init(struct tasklet_struct *t,
                                void (*func)(unsigned long),
                                unsigned long data) {
    if ((t)) {
        (t)->state = 0;
        atomic_set(&(t)->count, 0);
        (t)->callback = NULL;
        (t)->callback_old = func;
        (t)->data = data;
        (t)->use_callback_data = true;
    }
}

static inline bool tasklet_trylock(struct tasklet_struct *t) {
    if (!t)
        return false;
    return !test_and_set_bit(TASKLET_STATE_RUN, &t->state);
}

static inline void tasklet_unlock(struct tasklet_struct *t) {
    if (!t)
        return;
    clear_bit(TASKLET_STATE_RUN, &t->state);
}

static inline void tasklet_unlock_spin_wait(struct tasklet_struct *t) {
    while (t && test_bit(TASKLET_STATE_RUN, &t->state))
        barrier();
}

static inline void tasklet_schedule(struct tasklet_struct *t) {
    if (!t)
        return;
    if (!t->use_callback_data && !t->callback)
        return;
    if (t->use_callback_data && !t->callback_old)
        return;
    if (test_and_set_bit(TASKLET_STATE_SCHED, &t->state))
        return;
    if (!task_create("lcompat_tasklet", lcompat_tasklet_worker, (uint64_t)t,
                     KTHREAD_PRIORITY)) {
        clear_bit(TASKLET_STATE_SCHED, &t->state);
    }
}

static inline void tasklet_disable(struct tasklet_struct *t) {
    if (!t)
        return;
    atomic_inc(&t->count);
    tasklet_unlock_spin_wait(t);
}

static inline void tasklet_enable(struct tasklet_struct *t) {
    if (t)
        atomic_dec_return(&t->count);
}

static inline void tasklet_disable_nosync(struct tasklet_struct *t) {
    if (t)
        atomic_inc(&t->count);
}

static inline void tasklet_kill(struct tasklet_struct *t) {
    if (!t)
        return;

    atomic_inc(&t->count);
    clear_bit(TASKLET_STATE_SCHED, &t->state);
    tasklet_unlock_spin_wait(t);
    atomic_dec_return(&t->count);
}

#define tasklet_hi_schedule(t) tasklet_schedule((t))
#define tasklet_hi_enable(t) tasklet_enable((t))
#define tasklet_hi_disable(t) tasklet_disable((t))

static inline void local_bh_disable(void) {}
static inline void local_bh_enable(void) {}
static inline void cpu_relax(void) { barrier(); }
