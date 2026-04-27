#pragma once

#include <linux/sysfs.h>

struct sensor_device_attribute {
    struct device_attribute dev_attr;
    int index;
};

#define to_sensor_dev_attr(_dev_attr)                                          \
    container_of((_dev_attr), struct sensor_device_attribute, dev_attr)

#define SENSOR_DEVICE_ATTR_RO(_name, _func, _index)                            \
    struct sensor_device_attribute sensor_dev_attr_##_name = {                 \
        .dev_attr = __ATTR(_name, 0444, _func##_show, NULL),                   \
        .index = (_index),                                                     \
    }

#define ATTRIBUTE_GROUPS(_name)                                                \
    static const struct attribute_group _name##_group = {                      \
        .attrs = _name##_attrs,                                                \
    };                                                                         \
    static const struct attribute_group *_name##_groups[] = {                  \
        &_name##_group,                                                        \
        NULL,                                                                  \
    }
