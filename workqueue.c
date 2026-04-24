#include "lcompat_native.h"
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <task/task.h>

#define LCOMPAT_WORK_STATE_QUEUED BIT(0)
#define LCOMPAT_WORK_STATE_RUNNING BIT(1)

static void lcompat_delay_jiffies(unsigned long delay) {
    unsigned long deadline = jiffies + delay;

    while (time_before(jiffies, deadline))
        schedule(SCHED_FLAG_YIELD);
}

static void lcompat_work_worker(uint64_t arg) {
    struct work_struct *work = (struct work_struct *)arg;
    unsigned int expected = LCOMPAT_WORK_STATE_QUEUED;

    if (!work || !work->func)
        return;

    if (!__atomic_compare_exchange_n(&work->state, &expected,
                                     LCOMPAT_WORK_STATE_RUNNING, false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return;

    work->func(work);
    __atomic_store_n(&work->state, 0, __ATOMIC_RELEASE);
}

static void lcompat_delayed_work_worker(uint64_t arg) {
    struct delayed_work *dwork = (struct delayed_work *)arg;
    unsigned int generation;
    unsigned long expires;
    unsigned int expected = LCOMPAT_WORK_STATE_QUEUED;

    if (!dwork || !dwork->work.func)
        return;

    generation = __atomic_load_n(&dwork->generation, __ATOMIC_ACQUIRE);
    expires = __atomic_load_n(&dwork->expires, __ATOMIC_ACQUIRE);

    while (__atomic_load_n(&dwork->active, __ATOMIC_ACQUIRE) &&
           __atomic_load_n(&dwork->generation, __ATOMIC_ACQUIRE) ==
               generation &&
           time_before(jiffies, expires)) {
        schedule(SCHED_FLAG_YIELD);
    }

    if (!__atomic_load_n(&dwork->active, __ATOMIC_ACQUIRE) ||
        __atomic_load_n(&dwork->generation, __ATOMIC_ACQUIRE) != generation) {
        __atomic_store_n(&dwork->work.state, 0, __ATOMIC_RELEASE);
        return;
    }

    if (!__atomic_compare_exchange_n(&dwork->work.state, &expected,
                                     LCOMPAT_WORK_STATE_RUNNING, false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return;

    __atomic_store_n(&dwork->running, true, __ATOMIC_RELEASE);
    __atomic_store_n(&dwork->active, false, __ATOMIC_RELEASE);
    dwork->work.func(&dwork->work);
    __atomic_store_n(&dwork->running, false, __ATOMIC_RELEASE);
    __atomic_store_n(&dwork->work.state, 0, __ATOMIC_RELEASE);
}

void lcompat_init_work(struct work_struct *work, work_func_t func) {
    if (work) {
        work->func = func;
        work->state = 0;
    }
}

void lcompat_init_delayed_work(struct delayed_work *work, work_func_t func) {
    if (work) {
        work->work.func = func;
        work->work.state = 0;
        work->generation = 0;
        work->active = false;
        work->running = false;
        work->expires = 0;
    }
}

struct workqueue_struct *
lcompat_alloc_workqueue(const char *name, unsigned int flags, int max_active) {
    struct workqueue_struct *wq = kzalloc(sizeof(*wq), GFP_KERNEL);
    if (!wq)
        return NULL;
    wq->name = name;
    wq->flags = flags;
    wq->max_active = max_active;
    return wq;
}

void lcompat_destroy_workqueue(struct workqueue_struct *wq) { kfree(wq); }

bool lcompat_queue_work(struct workqueue_struct *wq, struct work_struct *work) {
    unsigned int expected = 0;

    (void)wq;
    if (!work || !work->func)
        return false;
    if (!__atomic_compare_exchange_n(&work->state, &expected,
                                     LCOMPAT_WORK_STATE_QUEUED, false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return false;
    if (!task_create("lcompat_work", lcompat_work_worker, (uint64_t)work,
                     KTHREAD_PRIORITY)) {
        __atomic_store_n(&work->state, 0, __ATOMIC_RELEASE);
        return false;
    }
    return true;
}

bool lcompat_queue_delayed_work(struct workqueue_struct *wq,
                                struct delayed_work *work,
                                unsigned long delay) {
    unsigned int expected = 0;

    (void)wq;
    if (!work || !work->work.func)
        return false;
    if (!__atomic_compare_exchange_n(&work->work.state, &expected,
                                     LCOMPAT_WORK_STATE_QUEUED, false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return false;
    __atomic_store_n(&work->expires, jiffies + delay, __ATOMIC_RELEASE);
    __atomic_store_n(&work->active, true, __ATOMIC_RELEASE);
    __atomic_add_fetch(&work->generation, 1, __ATOMIC_ACQ_REL);
    if (!task_create("lcompat_dwork", lcompat_delayed_work_worker,
                     (uint64_t)work, KTHREAD_PRIORITY)) {
        __atomic_store_n(&work->active, false, __ATOMIC_RELEASE);
        __atomic_store_n(&work->work.state, 0, __ATOMIC_RELEASE);
        __atomic_add_fetch(&work->generation, 1, __ATOMIC_ACQ_REL);
        return false;
    }
    return true;
}

void lcompat_flush_workqueue(struct workqueue_struct *wq) {
    (void)wq;
    schedule(SCHED_FLAG_YIELD);
}

bool lcompat_cancel_delayed_work_sync(struct delayed_work *work) {
    bool pending;

    if (!work)
        return false;

    pending = __atomic_load_n(&work->active, __ATOMIC_ACQUIRE) ||
              (__atomic_load_n(&work->work.state, __ATOMIC_ACQUIRE) ==
               LCOMPAT_WORK_STATE_QUEUED) ||
              __atomic_load_n(&work->running, __ATOMIC_ACQUIRE);
    __atomic_store_n(&work->active, false, __ATOMIC_RELEASE);
    __atomic_add_fetch(&work->generation, 1, __ATOMIC_ACQ_REL);
    if (__atomic_load_n(&work->work.state, __ATOMIC_ACQUIRE) ==
        LCOMPAT_WORK_STATE_QUEUED) {
        __atomic_store_n(&work->work.state, 0, __ATOMIC_RELEASE);
    }
    while (__atomic_load_n(&work->running, __ATOMIC_ACQUIRE))
        schedule(SCHED_FLAG_YIELD);
    return pending;
}
