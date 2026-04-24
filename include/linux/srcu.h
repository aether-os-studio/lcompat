#pragma once

struct srcu_struct {
    int dummy;
};

static inline int init_srcu_struct(struct srcu_struct *s) {
    if (s)
        s->dummy = 0;
    return 0;
}

static inline void cleanup_srcu_struct(struct srcu_struct *s) { (void)s; }
static inline int srcu_read_lock(struct srcu_struct *s) {
    (void)s;
    return 0;
}
static inline void srcu_read_unlock(struct srcu_struct *s, int idx) {
    (void)s;
    (void)idx;
}
