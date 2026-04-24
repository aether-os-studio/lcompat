#pragma once

#include <linux/slab.h>

static inline void *vmalloc(size_t size) { return kzalloc(size, GFP_KERNEL); }
static inline void *vzalloc(size_t size) { return kzalloc(size, GFP_KERNEL); }
static inline void vfree(const void *ptr) { kfree(ptr); }
