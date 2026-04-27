#pragma once

#include <linux/errno.h>
#include <linux/types.h>

struct regmap;

static inline int regmap_update_bits(struct regmap *map, unsigned int reg,
                                     unsigned int mask, unsigned int val) {
    (void)map;
    (void)reg;
    (void)mask;
    (void)val;
    return -ENODEV;
}
