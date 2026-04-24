#pragma once

#include <drivers/logger.h>
#include <libs/klibc.h>

#define BUG() panic(__FILE__, __LINE__, __func__, "BUG()")
#define BUG_ON(cond)                                                           \
    do {                                                                       \
        if (cond)                                                              \
            BUG();                                                             \
    } while (0)

#define WARN_ON(cond)                                                          \
    ({                                                                         \
        bool __warn = !!(cond);                                                \
        if (__warn)                                                            \
            printk("warn: %s:%d: %s\n", __FILE__, __LINE__, #cond);            \
        __warn;                                                                \
    })

#define WARN_ON_ONCE(cond) WARN_ON(cond)
