#pragma once

#define DECLARE_EWMA(name, precision, weight_rcp)                              \
    struct ewma_##name {                                                       \
        unsigned long internal;                                                \
    };                                                                         \
    static inline void ewma_##name##_init(struct ewma_##name *avg) {           \
        if (avg)                                                               \
            avg->internal = 0;                                                 \
    }                                                                          \
    static inline void ewma_##name##_add(struct ewma_##name *avg,              \
                                         unsigned long val) {                  \
        if (avg)                                                               \
            avg->internal = val;                                               \
    }                                                                          \
    static inline unsigned long ewma_##name##_read(                            \
        const struct ewma_##name *avg) {                                       \
        return avg ? avg->internal : 0;                                        \
    }
