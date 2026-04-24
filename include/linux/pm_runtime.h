#pragma once

#include <linux/device.h>

static inline int pm_runtime_resume_and_get(struct device *dev) {
    (void)dev;
    return 0;
}

static inline void pm_runtime_put(struct device *dev) { (void)dev; }
static inline void pm_runtime_put_autosuspend(struct device *dev) { (void)dev; }
static inline void pm_runtime_enable(struct device *dev) { (void)dev; }
static inline void pm_runtime_disable(struct device *dev) { (void)dev; }
static inline void pm_runtime_set_active(struct device *dev) { (void)dev; }
static inline void pm_runtime_set_suspended(struct device *dev) { (void)dev; }
