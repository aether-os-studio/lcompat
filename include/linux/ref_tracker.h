#pragma once

#include <linux/slab.h>

struct seq_file;

struct ref_tracker {
    int dummy;
};

struct ref_tracker_dir {
    spinlock_t lock;
    const char *name;
    int dead_limit;
};

static inline void ref_tracker_dir_init(struct ref_tracker_dir *dir,
                                        int dead_limit, const char *name) {
    if (!dir)
        return;
    spin_lock_init(&dir->lock);
    dir->name = name;
    dir->dead_limit = dead_limit;
}

static inline void ref_tracker_dir_exit(struct ref_tracker_dir *dir) {
    (void)dir;
}

static inline int ref_tracker_alloc(struct ref_tracker_dir *dir,
                                    struct ref_tracker **tracker, gfp_t gfp) {
    (void)dir;
    (void)gfp;
    if (!tracker)
        return -EINVAL;
    *tracker = kzalloc(sizeof(**tracker), GFP_NOWAIT);
    return *tracker ? 0 : -ENOMEM;
}

static inline void ref_tracker_free(struct ref_tracker_dir *dir,
                                    struct ref_tracker **tracker) {
    (void)dir;
    if (!tracker || !*tracker)
        return;
    kfree(*tracker);
    *tracker = NULL;
}

static inline void ref_tracker_dir_print_locked(struct ref_tracker_dir *dir,
                                                int limit) {
    (void)dir;
    (void)limit;
}

static inline int ref_tracker_dir_snprint(struct ref_tracker_dir *dir,
                                          char *buf, int buf_size) {
    (void)dir;
    if (!buf || buf_size <= 0)
        return 0;
    buf[0] = '\0';
    return 0;
}
