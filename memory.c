#include <mm/mm.h>
#include <linux/slab.h>

void *lcompat_kmalloc(size_t size, gfp_t flags) {
    (void)flags;
    return malloc(size);
}

void *lcompat_kzalloc(size_t size, gfp_t flags) {
    (void)flags;
    void *ptr = malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

void *lcompat_kcalloc(size_t n, size_t size, gfp_t flags) {
    (void)flags;
    if (n && size > SIZE_MAX / n)
        return NULL;
    return calloc(n, size);
}

void *lcompat_krealloc(const void *ptr, size_t size, gfp_t flags) {
    (void)flags;
    return realloc((void *)ptr, size);
}

void lcompat_kfree(const void *ptr) { free((void *)ptr); }
