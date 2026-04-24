#pragma once

#include <libs/klibc.h>
#include <linux/types.h>

static inline char *kstrdup(const char *s, gfp_t flags) {
    (void)flags;
    return s ? strdup(s) : NULL;
}
