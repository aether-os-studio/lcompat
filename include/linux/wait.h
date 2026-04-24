#pragma once

#include <linux/jiffies.h>
#include <task/task.h>

typedef struct wait_queue_head {
    int dummy;
} wait_queue_head_t;

typedef struct wait_queue_entry {
    int dummy;
} wait_queue_entry_t;

static inline void init_waitqueue_head(wait_queue_head_t *wq) {
    if (wq)
        wq->dummy = 0;
}

#define wait_event(wq, condition)                                              \
    do {                                                                       \
        (void)(wq);                                                            \
        while (!(condition))                                                   \
            schedule(SCHED_FLAG_YIELD);                                        \
    } while (0)

#define wait_event_timeout(wq, condition, timeout)                             \
    ({                                                                         \
        wait_queue_head_t *__wq = &(wq);                                       \
        long __timeout = (timeout);                                            \
        unsigned long __deadline;                                              \
        long __ret = 0;                                                        \
        (void)__wq;                                                            \
        if (condition) {                                                       \
            __ret = __timeout > 0 ? __timeout : 1;                             \
        } else if (__timeout > 0) {                                            \
            __deadline = jiffies + (unsigned long)__timeout;                   \
            for (;;) {                                                         \
                if (condition) {                                               \
                    __ret = 1;                                                 \
                    break;                                                     \
                }                                                              \
                if (time_after_eq(jiffies, __deadline))                        \
                    break;                                                     \
                schedule(SCHED_FLAG_YIELD);                                    \
            }                                                                  \
        }                                                                      \
        __ret;                                                                 \
    })

static inline void wake_up(wait_queue_head_t *wq) { (void)wq; }
static inline void wake_up_all(wait_queue_head_t *wq) { (void)wq; }
