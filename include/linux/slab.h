#pragma once

#include <linux/types.h>
#include <mm/mm.h>

#ifndef GFP_KERNEL
#define GFP_KERNEL 0x00000001UL
#endif
#ifndef GFP_ATOMIC
#define GFP_ATOMIC 0x00000002UL
#endif
#ifndef GFP_NOWAIT
#define GFP_NOWAIT 0x00000004UL
#endif
#ifndef __GFP_NOWARN
#define __GFP_NOWARN 0x00000008UL
#endif
#ifndef __GFP_NORETRY
#define __GFP_NORETRY 0x00000010UL
#endif
#ifndef __GFP_RETRY_MAYFAIL
#define __GFP_RETRY_MAYFAIL 0x00000020UL
#endif
#ifndef __GFP_DMA32
#define __GFP_DMA32 0x00000040UL
#endif
#ifndef __GFP_RECLAIM
#define __GFP_RECLAIM 0x00000080UL
#endif
#ifndef __GFP_RECLAIMABLE
#define __GFP_RECLAIMABLE 0x00000100UL
#endif
#ifndef GFP_HIGHUSER
#define GFP_HIGHUSER 0x00000200UL
#endif

void *lcompat_kmalloc(size_t size, gfp_t flags);
void *lcompat_kzalloc(size_t size, gfp_t flags);
void *lcompat_kcalloc(size_t n, size_t size, gfp_t flags);
void *lcompat_krealloc(const void *ptr, size_t size, gfp_t flags);
void lcompat_kfree(const void *ptr);

#define kmalloc(size, flags) lcompat_kmalloc((size), (flags))
#define kzalloc(size, flags) lcompat_kzalloc((size), (flags))
#define kcalloc(n, size, flags) lcompat_kcalloc((n), (size), (flags))
#define krealloc(ptr, size, flags) lcompat_krealloc((ptr), (size), (flags))
#define kfree(ptr) lcompat_kfree((ptr))

#define kmalloc_array(n, size, flags) kcalloc((n), (size), (flags))
#define kcalloc_node(n, size, flags, node) kcalloc((n), (size), (flags))

static inline void *kmemdup(const void *src, size_t len, gfp_t flags) {
    void *dst;

    if (!src)
        return NULL;
    dst = kmalloc(len, flags);
    if (!dst)
        return NULL;
    memcpy(dst, src, len);
    return dst;
}

#define kmalloc_obj(obj, ...) kmalloc(sizeof(obj), ##__VA_ARGS__, GFP_KERNEL)
#define kzalloc_obj(obj, ...) kzalloc(sizeof(obj), ##__VA_ARGS__, GFP_KERNEL)
#define kmalloc_objs(obj, n, ...)                                              \
    kmalloc(sizeof(obj) * (n), ##__VA_ARGS__, GFP_KERNEL)
#define kzalloc_objs(obj, n, ...)                                              \
    kzalloc(sizeof(obj) * (n), ##__VA_ARGS__, GFP_KERNEL)
#define kvmalloc_objs(obj, n, ...)                                             \
    kmalloc(sizeof(obj) * (n), ##__VA_ARGS__, GFP_KERNEL)
