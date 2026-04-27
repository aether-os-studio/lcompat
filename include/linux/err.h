#pragma once

#include <linux/types.h>

#define MAX_ERRNO 4095

#ifndef ERR_PTR
static inline void *ERR_PTR(long error) { return (void *)error; }
#endif
#ifndef PTR_ERR
static inline long PTR_ERR(const void *ptr) { return (long)ptr; }
#endif
#ifndef IS_ERR
static inline bool IS_ERR(const void *ptr) {
    return (unsigned long)ptr >= (unsigned long)-MAX_ERRNO;
}
#endif
#ifndef IS_ERR_OR_NULL
static inline bool IS_ERR_OR_NULL(const void *ptr) {
    return ptr == NULL || IS_ERR(ptr);
}
#endif
#ifndef PTR_ERR_OR_ZERO
static inline int PTR_ERR_OR_ZERO(const void *ptr) {
    return IS_ERR(ptr) ? (int)PTR_ERR(ptr) : 0;
}
#endif
