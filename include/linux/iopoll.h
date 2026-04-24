#pragma once

#include <linux/delay.h>
#include <linux/kernel.h>

#undef read_poll_timeout
#define read_poll_timeout(op, val, cond, sleep_us, timeout_us,                 \
                          sleep_before_read, args...)                          \
    ({                                                                         \
        int __loops = 0;                                                       \
        (void)(sleep_before_read);                                             \
        for (;;) {                                                             \
            (val) = op(args);                                                  \
            if (cond)                                                          \
                break;                                                         \
            if ((timeout_us) &&                                                \
                ++__loops > ((timeout_us) / ((sleep_us) ? (sleep_us) : 1)))    \
                break;                                                         \
            if (sleep_us)                                                      \
                udelay(sleep_us);                                              \
        }                                                                      \
        (cond) ? 0 : -ETIMEDOUT;                                               \
    })

#define read_poll_timeout_atomic(op, val, cond, delay_us, timeout_us,          \
                                 sleep_before_read, args...)                   \
    read_poll_timeout(op, val, cond, delay_us, timeout_us, sleep_before_read,  \
                      ##args)
