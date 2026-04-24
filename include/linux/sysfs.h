#pragma once

#include <linux/device.h>

struct kobject {
    const char *name;
    struct kobject *parent;
};

struct attribute_group {
    const char *name;
    struct attribute **attrs;
    struct bin_attribute **bin_attrs;
};

struct device_attribute {
    struct attribute attr;
    ssize_t (*show)(struct device *dev, struct device_attribute *attr,
                    char *buf);
    ssize_t (*store)(struct device *dev, struct device_attribute *attr,
                     const char *buf, size_t count);
};

struct kobj_attribute {
    struct attribute attr;
    ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
                    char *buf);
    ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
                     const char *buf, size_t count);
};

#define __ATTR(_name, _mode, _show, _store)                                    \
    {                                                                          \
        .attr = {.name = __stringify(_name), .value = NULL},                   \
        .show = (_show),                                                       \
        .store = (_store),                                                     \
    }

static inline void sysfs_attr_init(struct attribute *attr) { (void)attr; }

static inline ssize_t sysfs_emit(char *buf, const char *fmt, ...) {
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vsnprintf(buf, PAGE_SIZE, fmt, args);
    va_end(args);

    return ret;
}

static inline int sysfs_create_group(struct kobject *kobj,
                                     const struct attribute_group *grp) {
    (void)kobj;
    (void)grp;
    return 0;
}

static inline void sysfs_remove_group(struct kobject *kobj,
                                      const struct attribute_group *grp) {
    (void)kobj;
    (void)grp;
}

static inline int sysfs_create_bin_file(struct kobject *kobj,
                                        const struct bin_attribute *attr) {
    (void)kobj;
    (void)attr;
    return 0;
}

static inline void sysfs_remove_bin_file(struct kobject *kobj,
                                         const struct bin_attribute *attr) {
    (void)kobj;
    (void)attr;
}

static inline struct kobject *kobject_create_and_add(const char *name,
                                                     struct kobject *parent) {
    (void)name;
    (void)parent;
    return NULL;
}

static inline void kobject_put(struct kobject *kobj) { (void)kobj; }
