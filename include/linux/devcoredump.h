#pragma once

#include <linux/device.h>

static inline void dev_coredumpv(struct device *dev, void *data, size_t len,
                                 gfp_t gfp) {
    (void)dev;
    (void)data;
    (void)len;
    (void)gfp;
}
