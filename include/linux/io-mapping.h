#pragma once

#include <linux/types.h>

struct io_mapping {
    void *base;
    unsigned long size;
};

static inline struct io_mapping *io_mapping_create_wc(resource_size_t base,
                                                      unsigned long size) {
    (void)base;
    (void)size;
    return NULL;
}

static inline void io_mapping_free(struct io_mapping *mapping) {
    (void)mapping;
}
