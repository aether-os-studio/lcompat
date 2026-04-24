#pragma once

#include <libs/klibc.h>

static inline void kref_init(struct kref *kref) {
    if (!kref)
        return;
    kref_set((kref_t *)kref, 1);
}

static inline bool kref_get_unless_zero(struct kref *kref) {
    if (!kref || kref->ref_count <= 0)
        return false;
    kref_ref((kref_t *)kref);
    return true;
}

static inline int kref_read(const struct kref *kref) {
    return kref ? kref->ref_count : 0;
}

static inline int kref_put(struct kref *kref,
                           void (*release)(struct kref *kref)) {
    if (!kref)
        return 0;
    kref_unref((kref_t *)kref);
    if (kref->ref_count == 0) {
        if (release)
            release(kref);
        return 1;
    }
    return 0;
}
