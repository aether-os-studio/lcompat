#pragma once

#include <linux/wait.h>
#include <linux/jiffies.h>

struct completion {
    unsigned int done;
    wait_queue_head_t wait;
};

static inline void init_completion(struct completion *x) {
    if (!x)
        return;
    x->done = 0;
    init_waitqueue_head(&x->wait);
}

static inline void reinit_completion(struct completion *x) {
    init_completion(x);
}

static inline void complete(struct completion *x) {
    if (!x)
        return;
    x->done = 1;
    wake_up_all(&x->wait);
}

static inline void complete_all(struct completion *x) { complete(x); }

static inline void wait_for_completion(struct completion *x) {
    if (!x)
        return;
    wait_event(&x->wait, x->done);
}

static inline unsigned long wait_for_completion_timeout(struct completion *x,
                                                        unsigned long timeout) {
    unsigned long deadline;

    if (!x)
        return 0;
    if (x->done)
        return timeout ? timeout : 1;
    if (timeout == 0)
        return 0;

    deadline = lcompat_jiffies_refresh() + timeout;
    while (!x->done) {
        if (time_after_eq(lcompat_jiffies_refresh(), deadline))
            return 0;
        schedule(SCHED_FLAG_YIELD);
    }

    return time_after_eq(lcompat_jiffies_refresh(), deadline)
               ? 1
               : (deadline - lcompat_jiffies_refresh());
}
