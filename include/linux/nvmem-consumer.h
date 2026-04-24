#pragma once

#include <linux/of.h>
#include <linux/types.h>

struct nvmem_cell;

static inline void *nvmem_cell_read(struct nvmem_cell *cell, size_t *len) {
    (void)cell;
    if (len)
        *len = 0;
    return (void *)(intptr_t)-ENOENT;
}

static inline void nvmem_cell_put(struct nvmem_cell *cell) { (void)cell; }
