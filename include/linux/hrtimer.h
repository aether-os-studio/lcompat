#pragma once

#include <linux/types.h>

typedef s64 ktime_t;

enum hrtimer_restart {
    HRTIMER_NORESTART = 0,
    HRTIMER_RESTART = 1,
};

struct hrtimer;
typedef enum hrtimer_restart (*hrtimer_func_t)(struct hrtimer *timer);

#define HRTIMER_MODE_REL 0
#define HRTIMER_MODE_ABS 1

struct hrtimer {
    hrtimer_func_t function;
    ktime_t expires;
};

static inline void hrtimer_init(struct hrtimer *timer, clockid_t clock_id,
                                int mode) {
    (void)clock_id;
    (void)mode;
    if (!timer)
        return;
    timer->function = NULL;
    timer->expires = 0;
}

static inline int hrtimer_start(struct hrtimer *timer, ktime_t tim, int mode) {
    (void)mode;
    if (!timer)
        return 0;
    timer->expires = tim;
    if (timer->function)
        return timer->function(timer) == HRTIMER_RESTART;
    return 0;
}

static inline int hrtimer_cancel(struct hrtimer *timer) {
    if (!timer)
        return 0;
    timer->expires = 0;
    return 1;
}

static inline bool hrtimer_active(const struct hrtimer *timer) {
    return timer && timer->expires != 0;
}

static inline ktime_t ktime_get(void) { return (ktime_t)nano_time(); }
static inline bool ktime_before(ktime_t a, ktime_t b) { return a < b; }
static inline s64 ktime_us_delta(ktime_t later, ktime_t earlier) {
    return (later - earlier) / 1000;
}
