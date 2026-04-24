#pragma once

#include <linux/types.h>

struct mtd_info {
    const char *name;
};

static inline struct mtd_info *get_mtd_device_nm(const char *name) {
    (void)name;
    return (struct mtd_info *)(intptr_t)-ENOENT;
}

static inline int mtd_read(struct mtd_info *mtd, loff_t from, size_t len,
                           size_t *retlen, void *buf) {
    (void)mtd;
    (void)from;
    (void)len;
    (void)buf;
    if (retlen)
        *retlen = 0;
    return -ENOENT;
}

static inline void put_mtd_device(struct mtd_info *mtd) { (void)mtd; }
static inline bool mtd_is_bitflip(int err) { return false; }
