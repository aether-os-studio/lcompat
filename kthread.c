#include "lcompat_native.h"
#include <linux/sched.h>

static spinlock_t lcompat_kthread_lock = SPIN_INIT;
static struct task_struct *lcompat_kthreads;

static void lcompat_kthread_add(struct task_struct *thread) {
    spin_lock(&lcompat_kthread_lock);
    thread->next = lcompat_kthreads;
    lcompat_kthreads = thread;
    spin_unlock(&lcompat_kthread_lock);
}

static void lcompat_kthread_remove(struct task_struct *thread) {
    struct task_struct **pos;

    spin_lock(&lcompat_kthread_lock);
    for (pos = &lcompat_kthreads; *pos; pos = &(*pos)->next) {
        if (*pos != thread)
            continue;
        *pos = thread->next;
        thread->next = NULL;
        break;
    }
    spin_unlock(&lcompat_kthread_lock);
}

struct task_struct *current(void) {
    task_t *task = current_task;
    struct task_struct *pos;

    spin_lock(&lcompat_kthread_lock);
    for (pos = lcompat_kthreads; pos; pos = pos->next) {
        if (pos->task == task)
            break;
    }
    spin_unlock(&lcompat_kthread_lock);

    return pos;
}

static void lcompat_kthread_entry(uint64_t arg) {
    struct task_struct *thread = (struct task_struct *)arg;
    int ret = 0;

    if (!thread || !thread->threadfn)
        goto out;

    thread->task = current_task;
    thread->state = TASK_RUNNING;
    ret = thread->threadfn(thread->data);

out:
    if (thread) {
        thread->result = ret;
        lcompat_kthread_remove(thread);
        __atomic_store_n(&thread->exited, true, __ATOMIC_RELEASE);
    }
}

void lcompat_schedule(uint64_t flags) {
    struct task_struct *thread = current();

    if (thread &&
        __atomic_load_n(&thread->state, __ATOMIC_ACQUIRE) != TASK_RUNNING &&
        !__atomic_load_n(&thread->should_stop, __ATOMIC_ACQUIRE)) {
        task_block(current_task, TASK_BLOCKING, -1, "lcompat_kthread");
        __atomic_store_n(&thread->state, TASK_RUNNING, __ATOMIC_RELEASE);
        return;
    }

#undef schedule
    schedule(flags);
#define schedule(...)                                                          \
    __lcompat_schedule_choose(_, ##__VA_ARGS__, __lcompat_schedule1,           \
                              __lcompat_schedule0)(__VA_ARGS__)
}

struct task_struct *kthread_run(int (*threadfn)(void *data), void *data,
                                const char *namefmt, ...) {
    struct task_struct *thread;
    char name[TASK_COMM_LEN];
    va_list args;
    task_t *task;

    if (!threadfn)
        return ERR_PTR(-EINVAL);

    thread = kzalloc(sizeof(*thread), GFP_KERNEL);
    if (!thread)
        return ERR_PTR(-ENOMEM);

    thread->threadfn = threadfn;
    thread->data = data;
    thread->state = TASK_RUNNING;
    lcompat_kthread_add(thread);

    if (namefmt) {
        va_start(args, namefmt);
        vsnprintf(name, sizeof(name), namefmt, args);
        va_end(args);
    } else {
        snprintf(name, sizeof(name), "lcompat-kthread");
    }

    task = task_create(name, lcompat_kthread_entry, (uint64_t)thread,
                       KTHREAD_PRIORITY);
    if (!task) {
        lcompat_kthread_remove(thread);
        kfree(thread);
        return ERR_PTR(-ENOMEM);
    }

    thread->task = task;
    return thread;
}

int wake_up_process(struct task_struct *thread) {
    if (!thread || !thread->task)
        return 0;

    __atomic_store_n(&thread->state, TASK_RUNNING, __ATOMIC_RELEASE);
    task_unblock(thread->task, EOK);
    return 1;
}

bool kthread_should_stop(void) {
    struct task_struct *thread = current();

    return thread && __atomic_load_n(&thread->should_stop, __ATOMIC_ACQUIRE);
}

bool kthread_should_park(void) {
    struct task_struct *thread = current();

    return thread && __atomic_load_n(&thread->should_park, __ATOMIC_ACQUIRE);
}

void kthread_parkme(void) {
    struct task_struct *thread = current();

    if (!thread)
        return;

    __atomic_store_n(&thread->parked, true, __ATOMIC_RELEASE);
    while (__atomic_load_n(&thread->should_park, __ATOMIC_ACQUIRE) &&
           !__atomic_load_n(&thread->should_stop, __ATOMIC_ACQUIRE)) {
        __atomic_store_n(&thread->state, TASK_INTERRUPTIBLE, __ATOMIC_RELEASE);
        task_block(current_task, TASK_BLOCKING, -1, "lcompat_kthread_park");
    }
    __atomic_store_n(&thread->parked, false, __ATOMIC_RELEASE);
    __atomic_store_n(&thread->state, TASK_RUNNING, __ATOMIC_RELEASE);
}

void set_current_state(int state) {
    struct task_struct *thread = current();

    if (thread)
        __atomic_store_n(&thread->state, state, __ATOMIC_RELEASE);
}

int kthread_park(struct task_struct *thread) {
    if (!thread)
        return -EINVAL;

    __atomic_store_n(&thread->should_park, true, __ATOMIC_RELEASE);
    wake_up_process(thread);
    while (!__atomic_load_n(&thread->parked, __ATOMIC_ACQUIRE) &&
           !__atomic_load_n(&thread->exited, __ATOMIC_ACQUIRE))
        schedule(SCHED_FLAG_YIELD);
    return 0;
}

void kthread_unpark(struct task_struct *thread) {
    if (!thread)
        return;

    __atomic_store_n(&thread->should_park, false, __ATOMIC_RELEASE);
    wake_up_process(thread);
}

int kthread_stop(struct task_struct *thread) {
    int ret;

    if (!thread)
        return -EINVAL;

    __atomic_store_n(&thread->should_stop, true, __ATOMIC_RELEASE);
    __atomic_store_n(&thread->should_park, false, __ATOMIC_RELEASE);
    wake_up_process(thread);

    while (!__atomic_load_n(&thread->exited, __ATOMIC_ACQUIRE))
        schedule(SCHED_FLAG_YIELD);

    ret = thread->result;
    kfree(thread);
    return ret;
}

void sched_set_fifo_low(struct task_struct *thread) {
    if (thread && thread->task)
        thread->task->priority = NORMAL_PRIORITY;
}
