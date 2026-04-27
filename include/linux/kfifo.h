#pragma once

#include <linux/types.h>

#define DECLARE_KFIFO_PTR(name, type)                                          \
    struct {                                                                   \
        type *buffer;                                                          \
        unsigned int size;                                                     \
        unsigned int in;                                                       \
        unsigned int out;                                                      \
    } name

#define kfifo_init(fifo, buf, bytes)                                           \
    ({                                                                         \
        (fifo)->buffer = (void *)(buf);                                        \
        (fifo)->size = (unsigned int)((bytes) / sizeof(*(fifo)->buffer));      \
        (fifo)->in = 0;                                                        \
        (fifo)->out = 0;                                                       \
        0;                                                                     \
    })

#define kfifo_len(fifo) ((unsigned int)((fifo)->in - (fifo)->out))
#define kfifo_is_empty(fifo) (kfifo_len(fifo) == 0)
#define kfifo_is_full(fifo)                                                    \
    ((fifo)->size == 0 || kfifo_len(fifo) >= (fifo)->size)

#define kfifo_put(fifo, val)                                                   \
    ({                                                                         \
        bool __ok = !kfifo_is_full(fifo);                                      \
        if (__ok)                                                              \
            (fifo)->buffer[(fifo)->in++ & ((fifo)->size - 1)] = (val);         \
        __ok;                                                                  \
    })

#define kfifo_get(fifo, valp)                                                  \
    ({                                                                         \
        bool __ok = !kfifo_is_empty(fifo);                                     \
        if (__ok)                                                              \
            *(valp) = (fifo)->buffer[(fifo)->out++ & ((fifo)->size - 1)];      \
        __ok;                                                                  \
    })
