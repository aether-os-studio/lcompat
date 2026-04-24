#pragma once

#include <linux/interrupt.h>
#include <linux/jiffies.h>

static inline void ndelay(unsigned long nsecs) {
    uint64_t deadline = nano_time() + nsecs;
    while (nano_time() < deadline)
        cpu_relax();
}

static inline void udelay(unsigned long usecs) { ndelay(usecs * 1000ULL); }

static inline void mdelay(unsigned long msecs) { ndelay(msecs * 1000000ULL); }

static inline void msleep(unsigned int msecs) {
    (void)schedule_timeout((long)msecs_to_jiffies(msecs));
}

static inline void usleep_range(unsigned long min, unsigned long max) {
    (void)max;
    udelay(min);
}

static inline void fsleep(unsigned long usecs) { usleep_range(usecs, usecs); }
