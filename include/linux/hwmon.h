#pragma once

#include <linux/device.h>
#include <linux/err.h>
#include <linux/sysfs.h>

static inline struct device *
devm_hwmon_device_register_with_groups(struct device *dev, const char *name,
                                       void *drvdata,
                                       const struct attribute_group **groups) {
    (void)name;
    (void)groups;
    dev_set_drvdata(dev, drvdata);
    return dev;
}
