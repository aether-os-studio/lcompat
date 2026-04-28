#include "lcompat_native.h"
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <task/task.h>

#define LCOMPAT_WORK_STATE_QUEUED BIT(0)
#define LCOMPAT_WORK_STATE_RUNNING BIT(1)

static struct workqueue_struct lcompat_system_wq_obj = {
    .name = "system_wq",
    .flags = 0,
    .max_active = 1,
    .lock = SPIN_INIT,
};

static struct workqueue_struct lcompat_system_highpri_wq_obj = {
    .name = "system_highpri_wq",
    .flags = WQ_HIGHPRI,
    .max_active = 1,
    .lock = SPIN_INIT,
};

struct workqueue_struct *system_wq = &lcompat_system_wq_obj;
struct workqueue_struct *system_highpri_wq = &lcompat_system_highpri_wq_obj;

static void lcompat_delay_jiffies(unsigned long delay) {
    unsigned long deadline = lcompat_jiffies_refresh() + delay;

    while (time_before(lcompat_jiffies_refresh(), deadline))
        schedule(SCHED_FLAG_YIELD);
}

static struct work_struct *
lcompat_workqueue_pop_locked(struct workqueue_struct *wq) {
    struct work_struct *work = wq->head;

    if (!work)
        return NULL;

    wq->head = work->next;
    if (!wq->head)
        wq->tail = NULL;

    work->next = NULL;
    return work;
}

static bool lcompat_workqueue_remove_locked(struct workqueue_struct *wq,
                                            struct work_struct *work) {
    struct work_struct *prev = NULL;
    struct work_struct *pos;

    for (pos = wq->head; pos; pos = pos->next) {
        if (pos != work) {
            prev = pos;
            continue;
        }

        if (prev)
            prev->next = pos->next;
        else
            wq->head = pos->next;

        if (wq->tail == pos)
            wq->tail = prev;

        pos->next = NULL;
        pos->wq = NULL;
        return true;
    }

    return false;
}

static void lcompat_workqueue_worker(uint64_t arg) {
    struct workqueue_struct *wq = (struct workqueue_struct *)arg;

    if (!wq)
        return;

    for (;;) {
        struct delayed_work *dwork;
        struct work_struct *work;
        bool should_run = false;

        spin_lock(&wq->lock);
        work = lcompat_workqueue_pop_locked(wq);
        if (!work) {
            wq->worker_waiting = true;
            spin_unlock(&wq->lock);
            task_block(current_task, TASK_BLOCKING, -1, "lcompat_workqueue");
            continue;
        }

        wq->worker_waiting = false;
        wq->active++;
        spin_unlock(&wq->lock);

        dwork = work->delayed_owner;
        for (;;) {
            unsigned int state =
                __atomic_load_n(&work->state, __ATOMIC_ACQUIRE);
            unsigned int next = (state & ~LCOMPAT_WORK_STATE_QUEUED) |
                                LCOMPAT_WORK_STATE_RUNNING;

            if (!(state & LCOMPAT_WORK_STATE_QUEUED))
                break;
            if (__atomic_compare_exchange_n(&work->state, &state, next, false,
                                            __ATOMIC_ACQ_REL,
                                            __ATOMIC_ACQUIRE)) {
                should_run = true;
                break;
            }
        }

        if (should_run) {
            if (dwork)
                __atomic_store_n(&dwork->running, true, __ATOMIC_RELEASE);
            if (work->func)
                work->func(work);
            if (dwork)
                __atomic_store_n(&dwork->running, false, __ATOMIC_RELEASE);

            __atomic_fetch_and(&work->state, ~LCOMPAT_WORK_STATE_RUNNING,
                               __ATOMIC_ACQ_REL);
        }

        spin_lock(&wq->lock);
        if (!(__atomic_load_n(&work->state, __ATOMIC_ACQUIRE) &
              LCOMPAT_WORK_STATE_QUEUED))
            work->wq = NULL;
        if (wq->active)
            wq->active--;
        spin_unlock(&wq->lock);
    }
}

static bool lcompat_workqueue_ensure_worker(struct workqueue_struct *wq) {
    bool expected = false;
    task_t *task;

    if (!wq)
        return false;

    if (__atomic_load_n(&wq->worker_task, __ATOMIC_ACQUIRE))
        return true;

    if (!__atomic_compare_exchange_n(&wq->worker_starting, &expected, true,
                                     false, __ATOMIC_ACQ_REL,
                                     __ATOMIC_ACQUIRE)) {
        while (__atomic_load_n(&wq->worker_starting, __ATOMIC_ACQUIRE) &&
               !__atomic_load_n(&wq->worker_task, __ATOMIC_ACQUIRE))
            schedule(SCHED_FLAG_YIELD);
        return __atomic_load_n(&wq->worker_task, __ATOMIC_ACQUIRE) != NULL;
    }

    task = task_create(wq->name ? wq->name : "lcompat_wq",
                       lcompat_workqueue_worker, (uint64_t)wq,
                       (wq->flags & (WQ_HIGHPRI | WQ_BH)) ? KTHREAD_PRIORITY
                                                          : NORMAL_PRIORITY);
    if (!task) {
        __atomic_store_n(&wq->worker_starting, false, __ATOMIC_RELEASE);
        return false;
    }

    __atomic_store_n(&wq->worker_task, task, __ATOMIC_RELEASE);
    __atomic_store_n(&wq->worker_starting, false, __ATOMIC_RELEASE);
    return true;
}

static void lcompat_workqueue_wake(struct workqueue_struct *wq) {
    task_t *task;

    if (!wq)
        return;

    task = (task_t *)__atomic_load_n(&wq->worker_task, __ATOMIC_ACQUIRE);
    if (task && __atomic_load_n(&wq->worker_waiting, __ATOMIC_ACQUIRE))
        task_unblock(task, EOK);
}

static void lcompat_delayed_work_worker(uint64_t arg) {
    struct delayed_work *dwork = (struct delayed_work *)arg;
    unsigned int generation;
    unsigned long expires;
    struct workqueue_struct *wq;

    if (!dwork || !dwork->work.func)
        return;

    generation = __atomic_load_n(&dwork->generation, __ATOMIC_ACQUIRE);
    expires = __atomic_load_n(&dwork->expires, __ATOMIC_ACQUIRE);
    wq = __atomic_load_n(&dwork->work.wq, __ATOMIC_ACQUIRE);

    while (__atomic_load_n(&dwork->active, __ATOMIC_ACQUIRE) &&
           __atomic_load_n(&dwork->generation, __ATOMIC_ACQUIRE) ==
               generation &&
           time_before(lcompat_jiffies_refresh(), expires)) {
        schedule(SCHED_FLAG_YIELD);
    }

    if (!__atomic_load_n(&dwork->active, __ATOMIC_ACQUIRE) ||
        __atomic_load_n(&dwork->generation, __ATOMIC_ACQUIRE) != generation) {
        __atomic_store_n(&dwork->work.state, 0, __ATOMIC_RELEASE);
        return;
    }

    __atomic_store_n(&dwork->active, false, __ATOMIC_RELEASE);
    if (!__atomic_compare_exchange_n(
            &dwork->work.state, &(unsigned int){LCOMPAT_WORK_STATE_QUEUED}, 0,
            false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return;

    lcompat_queue_work(wq ? wq : system_wq, &dwork->work);
}

void lcompat_init_work(struct work_struct *work, work_func_t func) {
    if (work) {
        work->func = func;
        work->state = 0;
        work->next = NULL;
        work->wq = NULL;
        work->delayed_owner = NULL;
    }
}

void lcompat_init_delayed_work(struct delayed_work *work, work_func_t func) {
    if (work) {
        work->work.func = func;
        work->work.state = 0;
        work->work.next = NULL;
        work->work.wq = NULL;
        work->work.delayed_owner = work;
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
    wq->lock = SPIN_INIT;
    return wq;
}

void lcompat_destroy_workqueue(struct workqueue_struct *wq) { kfree(wq); }

bool lcompat_queue_work(struct workqueue_struct *wq, struct work_struct *work) {
    if (!wq)
        wq = system_wq;
    if (!work || !work->func)
        return false;
    if (!lcompat_workqueue_ensure_worker(wq))
        return false;

    for (;;) {
        unsigned int state = __atomic_load_n(&work->state, __ATOMIC_ACQUIRE);
        unsigned int next = state | LCOMPAT_WORK_STATE_QUEUED;

        if (state & LCOMPAT_WORK_STATE_QUEUED)
            return false;
        if (__atomic_compare_exchange_n(&work->state, &state, next, false,
                                        __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
            break;
    }

    spin_lock(&wq->lock);
    work->next = NULL;
    work->wq = wq;
    if (wq->tail)
        wq->tail->next = work;
    else
        wq->head = work;
    wq->tail = work;
    spin_unlock(&wq->lock);

    lcompat_workqueue_wake(wq);
    return true;
}

bool lcompat_queue_delayed_work(struct workqueue_struct *wq,
                                struct delayed_work *work,
                                unsigned long delay) {
    unsigned int expected = 0;

    if (!wq)
        wq = system_wq;
    if (!work || !work->work.func)
        return false;
    if (delay == 0)
        return lcompat_queue_work(wq, &work->work);
    if (!__atomic_compare_exchange_n(&work->work.state, &expected,
                                     LCOMPAT_WORK_STATE_QUEUED, false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return false;
    work->work.wq = wq;
    __atomic_store_n(&work->expires, lcompat_jiffies_refresh() + delay,
                     __ATOMIC_RELEASE);
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
    if (!wq)
        wq = system_wq;

    while (wq) {
        bool idle;

        spin_lock(&wq->lock);
        idle = !wq->head && wq->active == 0;
        spin_unlock(&wq->lock);

        if (idle)
            break;

        schedule(SCHED_FLAG_YIELD);
    }
}

bool lcompat_cancel_work_sync(struct work_struct *work) {
    struct workqueue_struct *wq;
    bool canceled = false;

    if (!work)
        return false;

    wq = __atomic_load_n(&work->wq, __ATOMIC_ACQUIRE);
    if (wq) {
        spin_lock(&wq->lock);
        canceled = lcompat_workqueue_remove_locked(wq, work);
        spin_unlock(&wq->lock);
        if (canceled)
            __atomic_fetch_and(&work->state, ~LCOMPAT_WORK_STATE_QUEUED,
                               __ATOMIC_ACQ_REL);
    }

    while (__atomic_load_n(&work->state, __ATOMIC_ACQUIRE) ==
           LCOMPAT_WORK_STATE_RUNNING)
        schedule(SCHED_FLAG_YIELD);

    return canceled;
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
    lcompat_cancel_work_sync(&work->work);
    while (__atomic_load_n(&work->running, __ATOMIC_ACQUIRE))
        schedule(SCHED_FLAG_YIELD);
    return pending;
}
