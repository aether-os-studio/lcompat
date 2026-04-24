#pragma once

#include <linux/types.h>

typedef s64 ktime_t;

static inline ktime_t ktime_get_boottime(void) { return (ktime_t)nano_time(); }

static inline u64 ktime_get_boottime_ns(void) { return (u64)nano_time(); }

static inline ktime_t ktime_sub(ktime_t lhs, ktime_t rhs) { return lhs - rhs; }

static inline s64 ktime_to_us(ktime_t kt) { return kt / 1000; }
