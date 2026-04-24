#pragma once

#include <linux/kernel.h>
#include <task/task.h>

#ifndef HZ
#define HZ SCHED_HZ
#endif

#define MAX_JIFFY_OFFSET ((LONG_MAX >> 1) - 1)
#define MAX_SCHEDULE_TIMEOUT LONG_MAX

static inline unsigned long lcompat_jiffies_now(void) {
    return (unsigned long)(nano_time() / (1000000000ULL / HZ));
}

#define jiffies lcompat_jiffies_now()

static inline u64 get_jiffies_64(void) { return (u64)lcompat_jiffies_now(); }

static inline unsigned long msecs_to_jiffies(const unsigned int m) {
    if (m == 0)
        return 0;
    return DIV_ROUND_UP((u64)m * HZ, 1000);
}

static inline unsigned long usecs_to_jiffies(const unsigned int u) {
    if (u == 0)
        return 0;
    return DIV_ROUND_UP((u64)u * HZ, 1000000);
}

static inline unsigned long nsecs_to_jiffies(u64 n) {
    if (n == 0)
        return 0;
    return DIV_ROUND_UP(n * HZ, 1000000000ULL);
}

static inline unsigned int jiffies_to_msecs(const unsigned long j) {
    return (unsigned int)(((u64)j * 1000) / HZ);
}

static inline unsigned int jiffies_to_usecs(const unsigned long j) {
    return (unsigned int)(((u64)j * 1000000) / HZ);
}

static inline unsigned int jiffies_delta_to_msecs(const unsigned long delta) {
    return jiffies_to_msecs(delta);
}

#define time_after(a, b) ((long)((b) - (a)) < 0)
#define time_before(a, b) time_after((b), (a))
#define time_after_eq(a, b) ((long)((a) - (b)) >= 0)
#define time_before_eq(a, b) time_after_eq((b), (a))

#define time_after32(a, b) ((s32)((b) - (a)) < 0)
#define time_before32(a, b) time_after32((b), (a))

static inline unsigned long round_jiffies_up(unsigned long j) { return j; }
static inline unsigned long round_jiffies_up_relative(unsigned long j) {
    return j;
}

static inline long schedule_timeout(long timeout) {
    unsigned long deadline;

    if (timeout <= 0)
        return 0;
    if (timeout == MAX_SCHEDULE_TIMEOUT) {
        schedule(SCHED_FLAG_YIELD);
        return MAX_SCHEDULE_TIMEOUT;
    }

    deadline = jiffies + (unsigned long)timeout;
    while (time_before(jiffies, deadline))
        schedule(SCHED_FLAG_YIELD);

    return 0;
}

static inline long io_schedule_timeout(long timeout) {
    return schedule_timeout(timeout);
}

#define time_is_after_jiffies(a) time_before(jiffies, (a))
