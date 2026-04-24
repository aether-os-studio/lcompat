#pragma once

#include <arch/arch.h>
#include <linux/hrtimer.h>

static inline u64 local_clock(void) { return (u64)nano_time(); }

static inline u64 sched_clock(void) { return local_clock(); }

static inline u64 ktime_get_raw_ns(void) { return (u64)ktime_get(); }

static inline u64 ktime_get_raw_fast_ns(void) { return ktime_get_raw_ns(); }

static inline unsigned int raw_smp_processor_id(void) {
    return (unsigned int)current_cpu_id;
}

static inline unsigned int smp_processor_id(void) {
    return raw_smp_processor_id();
}

static inline unsigned int get_cpu(void) { return smp_processor_id(); }

static inline void put_cpu(void) {}
