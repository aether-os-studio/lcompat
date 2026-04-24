#pragma once

#include <linux/slab.h>
#include <task/task.h>

struct task_struct {
    int dummy;
};

static inline void __lcompat_schedule0(void) { schedule(SCHED_FLAG_YIELD); }
static inline void __lcompat_schedule1(uint64_t flags) { schedule(flags); }

#define __lcompat_schedule_choose(_0, _1, NAME, ...) NAME
#define schedule(...)                                                          \
    __lcompat_schedule_choose(_, ##__VA_ARGS__, __lcompat_schedule1,           \
                              __lcompat_schedule0)(__VA_ARGS__)

static inline struct task_struct *current(void) { return NULL; }

static inline struct task_struct *
kthread_run(int (*threadfn)(void *data), void *data, const char *namefmt, ...) {
    (void)threadfn;
    (void)data;
    (void)namefmt;
    return (struct task_struct *)kzalloc(sizeof(struct task_struct),
                                         GFP_KERNEL);
}

static inline int wake_up_process(struct task_struct *task) {
    (void)task;
    return 0;
}

static inline bool kthread_should_stop(void) { return false; }
static inline bool kthread_should_park(void) { return false; }
static inline void kthread_parkme(void) {}
static inline void set_current_state(int state) { (void)state; }

static inline int kthread_park(struct task_struct *task) {
    (void)task;
    return 0;
}

static inline void kthread_unpark(struct task_struct *task) { (void)task; }

static inline int kthread_stop(struct task_struct *task) {
    kfree(task);
    return 0;
}

static inline void cond_resched(void) { schedule(); }
static inline void sched_set_fifo_low(struct task_struct *task) { (void)task; }

#define TASK_INTERRUPTIBLE 0

#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif
