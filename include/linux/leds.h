#pragma once

#include <linux/device.h>

enum led_brightness {
    LED_OFF = 0,
    LED_ON = 1,
};

struct led_classdev {
    const char *name;
    const char *default_trigger;
    enum led_brightness brightness;
    int (*brightness_set_blocking)(struct led_classdev *led,
                                   enum led_brightness brightness);
    void (*brightness_set)(struct led_classdev *led,
                           enum led_brightness brightness);
    int (*blink_set)(struct led_classdev *led, unsigned long *delay_on,
                     unsigned long *delay_off);
};

static inline int led_classdev_register(struct device *dev,
                                        struct led_classdev *led) {
    (void)dev;
    (void)led;
    return 0;
}

static inline void led_classdev_unregister(struct led_classdev *led) {
    (void)led;
}
