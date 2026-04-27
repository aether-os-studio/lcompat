#pragma once

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <task/task.h>

struct task_struct {
    task_t *task;
    int (*threadfn)(void *data);
    void *data;
    struct task_struct *next;
    volatile bool should_stop;
    volatile bool should_park;
    volatile bool parked;
    volatile bool exited;
    volatile int state;
    int result;
};

void lcompat_schedule(uint64_t flags);
struct task_struct *current(void);
struct task_struct *kthread_run(int (*threadfn)(void *data), void *data,
                                const char *namefmt, ...);
int wake_up_process(struct task_struct *task);
bool kthread_should_stop(void);
bool kthread_should_park(void);
void kthread_parkme(void);
void set_current_state(int state);
int kthread_park(struct task_struct *task);
void kthread_unpark(struct task_struct *task);
int kthread_stop(struct task_struct *task);
void sched_set_fifo_low(struct task_struct *task);

static inline void __lcompat_schedule0(void) {
    lcompat_schedule(SCHED_FLAG_YIELD);
}

static inline void __lcompat_schedule1(uint64_t flags) {
    lcompat_schedule(flags);
}

#define __lcompat_schedule_choose(_0, _1, NAME, ...) NAME
#define schedule(...)                                                          \
    __lcompat_schedule_choose(_, ##__VA_ARGS__, __lcompat_schedule1,           \
                              __lcompat_schedule0)(__VA_ARGS__)

static inline void cond_resched(void) { schedule(); }

#ifndef TASK_INTERRUPTIBLE
#define TASK_INTERRUPTIBLE 1
#endif

#ifndef TASK_KILLABLE
#define TASK_KILLABLE TASK_INTERRUPTIBLE
#endif

#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif
