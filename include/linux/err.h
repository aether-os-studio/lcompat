#pragma once

#include <linux/types.h>

#define MAX_ERRNO 4095

static inline void *ERR_PTR(long error) { return (void *)error; }
static inline long PTR_ERR(const void *ptr) { return (long)ptr; }
static inline bool IS_ERR(const void *ptr) {
    return (unsigned long)ptr >= (unsigned long)-MAX_ERRNO;
}
static inline bool IS_ERR_OR_NULL(const void *ptr) {
    return ptr == NULL || IS_ERR(ptr);
}
static inline int PTR_ERR_OR_ZERO(const void *ptr) {
    return IS_ERR(ptr) ? (int)PTR_ERR(ptr) : 0;
}
