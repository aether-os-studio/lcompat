#pragma once

#include <linux/sysfs.h>
#include <linux/types.h>

struct perf_event;
struct pmu;

typedef struct {
    long long counter;
} local64_t;

static inline long long local64_read(const local64_t *v) {
    return v ? __atomic_load_n(&v->counter, __ATOMIC_ACQUIRE) : 0;
}

static inline void local64_set(local64_t *v, long long i) {
    if (v)
        __atomic_store_n(&v->counter, i, __ATOMIC_RELEASE);
}

static inline bool local64_try_cmpxchg(local64_t *v, long long *old,
                                       long long new_value) {
    return v && old &&
           __atomic_compare_exchange_n(&v->counter, old, new_value, false,
                                       __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
}

static inline void local64_add(long long i, local64_t *v) {
    if (v)
        __atomic_add_fetch(&v->counter, i, __ATOMIC_ACQ_REL);
}

struct hw_perf_event {
    local64_t prev_count;
    int state;
};

struct perf_event_attr {
    u32 type;
    u64 config;
    u64 sample_period;
};

struct perf_event {
    struct perf_event_attr attr;
    struct pmu *pmu;
    struct perf_event *parent;
    int cpu;
    struct hw_perf_event hw;
    local64_t count;
    void (*destroy)(struct perf_event *event);
};

struct pmu {
    void *module;
    int type;
    int task_ctx_nr;
    int scope;
    const struct attribute_group **attr_groups;
    int (*event_init)(struct perf_event *event);
    int (*add)(struct perf_event *event, int flags);
    void (*del)(struct perf_event *event, int flags);
    void (*start)(struct perf_event *event, int flags);
    void (*stop)(struct perf_event *event, int flags);
    void (*read)(struct perf_event *event);
};

struct perf_pmu_events_attr {
    struct device_attribute attr;
    const char *event_str;
};

#define PERF_HES_STOPPED BIT(0)

#define PERF_EF_START BIT(0)
#define PERF_EF_UPDATE BIT(1)

#define PERF_PMU_SCOPE_SYS_WIDE 0

enum { perf_invalid_context = -1 };

static inline bool has_branch_stack(struct perf_event *event) {
    (void)event;
    return false;
}

static inline ssize_t perf_event_sysfs_show(struct device *dev,
                                            struct device_attribute *attr,
                                            char *buf) {
    struct perf_pmu_events_attr *pmu_attr =
        container_of(attr, struct perf_pmu_events_attr, attr);

    return snprintf(buf, PAGE_SIZE, "%s\n",
                    pmu_attr->event_str ? pmu_attr->event_str : "");
}

static inline int perf_pmu_register(struct pmu *pmu, const char *name,
                                    int type) {
    if (!pmu)
        return -EINVAL;
    pmu->type = type;
    (void)name;
    return 0;
}

static inline void perf_pmu_unregister(struct pmu *pmu) { (void)pmu; }
