#pragma once

#include <linux/mutex.h>
#include <linux/types.h>

typedef struct seqcount {
    unsigned seq;
} seqcount_t;

typedef struct seqcount_mutex {
    seqcount_t seqcount;
    struct mutex *lock;
} seqcount_mutex_t;

static inline void seqcount_init(seqcount_t *s) {
    if (s)
        s->seq = 0;
}

static inline void seqcount_mutex_init(seqcount_mutex_t *s,
                                       struct mutex *lock) {
    if (!s)
        return;
    seqcount_init(&s->seqcount);
    s->lock = lock;
}

static inline void write_seqcount_begin(seqcount_t *s) {
    if (s)
        s->seq++;
}

static inline void write_seqcount_end(seqcount_t *s) {
    if (s)
        s->seq++;
}

static inline void write_seqcount_invalidate(seqcount_t *s) {
    if (s)
        s->seq++;
}

static inline unsigned read_seqcount_begin(const seqcount_t *s) {
    return s ? s->seq : 0;
}

static inline bool read_seqcount_retry(const seqcount_t *s, unsigned start) {
    return s ? s->seq != start : false;
}
