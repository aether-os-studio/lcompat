#pragma once

#include <drivers/bus/bus.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>

struct device_driver;
struct device_node;

struct device {
    const char *init_name;
    const char *kobj_name;
    struct device *parent;
    struct device_node *of_node;
    bus_device_t *busdev;
    void *driver_data;
    void *native;
    struct device_driver *driver;
};

struct device_driver {
    const char *name;
    void *pm;
};

static inline void *dev_get_drvdata(const struct device *dev) {
    return dev ? dev->driver_data : NULL;
}

static inline void dev_set_drvdata(struct device *dev, void *data) {
    if (dev)
        dev->driver_data = data;
}

static inline char *devm_kasprintf(struct device *dev, gfp_t gfp,
                                   const char *fmt, ...) {
    va_list args;
    char *buf;

    (void)dev;
    buf = kzalloc(128, gfp);
    if (!buf)
        return NULL;

    va_start(args, fmt);
    vsnprintf(buf, 128, fmt, args);
    va_end(args);
    return buf;
}

static inline void *devm_kzalloc(struct device *dev, size_t size, gfp_t flags) {
    (void)dev;
    return kzalloc(size, flags);
}

static inline void *devm_kmalloc(struct device *dev, size_t size, gfp_t flags) {
    (void)dev;
    return kmalloc(size, flags);
}

static inline void *devm_kcalloc(struct device *dev, size_t n, size_t size,
                                 gfp_t flags) {
    (void)dev;
    return kcalloc(n, size, flags);
}

static inline void *devm_kmemdup(struct device *dev, const void *src,
                                 size_t len, gfp_t flags) {
    void *dst;

    (void)dev;
    if (!src)
        return NULL;

    dst = kmalloc(len, flags);
    if (!dst)
        return NULL;

    memcpy(dst, src, len);
    return dst;
}

static inline void devm_kfree(struct device *dev, const void *ptr) {
    (void)dev;
    kfree(ptr);
}

static inline const char *dev_name(const struct device *dev) {
    if (!dev)
        return "(null)";
    if (dev->kobj_name)
        return dev->kobj_name;
    if (dev->init_name)
        return dev->init_name;
    return "(device)";
}

static inline int device_set_wakeup_enable(struct device *dev, bool enabled) {
    (void)dev;
    (void)enabled;
    return 0;
}

#define dev_err(dev, fmt, ...)                                                 \
    printk("%s: err: " fmt, dev_name(dev), ##__VA_ARGS__)
#define dev_warn(dev, fmt, ...)                                                \
    printk("%s: warn: " fmt, dev_name(dev), ##__VA_ARGS__)
#define dev_info(dev, fmt, ...)                                                \
    printk("%s: info: " fmt, dev_name(dev), ##__VA_ARGS__)
#define dev_err_ratelimited dev_err
#define dev_dbg(dev, fmt, ...)                                                 \
    printk("%s: debug: " fmt, dev_name(dev), ##__VA_ARGS__)
#define dev_dbg_ratelimited(dev, fmt, ...) dev_dbg(dev, fmt, ##__VA_ARGS__)
#define dev_warn_once(dev, fmt, ...) dev_warn(dev, fmt, ##__VA_ARGS__)
