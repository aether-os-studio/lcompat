#pragma once

#include <linux/dma-fence.h>

enum dma_resv_usage {
    DMA_RESV_USAGE_KERNEL = 0,
    DMA_RESV_USAGE_WRITE = 1,
    DMA_RESV_USAGE_READ = 2,
    DMA_RESV_USAGE_BOOKKEEP = 3,
};

struct dma_resv {
    struct dma_fence *fence;
    void *locking_ctx;
};

struct dma_resv_iter {
    struct dma_resv *resv;
    struct dma_fence *fence;
    enum dma_resv_usage usage;
    bool done;
};

static inline enum dma_resv_usage dma_resv_usage_rw(bool write) {
    return write ? DMA_RESV_USAGE_WRITE : DMA_RESV_USAGE_READ;
}

static inline void dma_resv_init(struct dma_resv *resv) {
    if (!resv)
        return;
    resv->fence = NULL;
    resv->locking_ctx = NULL;
}

static inline void dma_resv_fini(struct dma_resv *resv) {
    if (!resv)
        return;
    dma_fence_put(resv->fence);
    resv->fence = NULL;
}

static inline void dma_resv_assert_held(struct dma_resv *resv) { (void)resv; }
static inline int dma_resv_lock(struct dma_resv *resv, void *ctx) {
    if (resv)
        resv->locking_ctx = ctx;
    return 0;
}
static inline int dma_resv_lock_interruptible(struct dma_resv *resv,
                                              void *ctx) {
    return dma_resv_lock(resv, ctx);
}
static inline int dma_resv_lock_slow(struct dma_resv *resv, void *ctx) {
    return dma_resv_lock(resv, ctx);
}
static inline int dma_resv_lock_slow_interruptible(struct dma_resv *resv,
                                                   void *ctx) {
    return dma_resv_lock(resv, ctx);
}
static inline void dma_resv_unlock(struct dma_resv *resv) {
    if (resv)
        resv->locking_ctx = NULL;
}
static inline bool dma_resv_trylock(struct dma_resv *resv) {
    (void)resv;
    return true;
}
static inline void *dma_resv_locking_ctx(struct dma_resv *resv) {
    return resv ? resv->locking_ctx : NULL;
}
static inline int dma_resv_reserve_fences(struct dma_resv *resv,
                                          unsigned int num) {
    (void)resv;
    (void)num;
    return 0;
}
static inline void dma_resv_add_fence(struct dma_resv *resv,
                                      struct dma_fence *fence,
                                      enum dma_resv_usage usage) {
    (void)usage;
    if (!resv)
        return;
    dma_fence_put(resv->fence);
    resv->fence = dma_fence_get(fence);
}
static inline bool dma_resv_test_signaled(struct dma_resv *resv,
                                          enum dma_resv_usage usage) {
    (void)usage;
    return !resv || !resv->fence || dma_fence_is_signaled(resv->fence);
}
static inline int dma_resv_get_singleton(struct dma_resv *resv,
                                         enum dma_resv_usage usage,
                                         struct dma_fence **fence) {
    (void)usage;
    if (!fence)
        return -1;
    *fence = resv ? dma_fence_get(resv->fence) : NULL;
    return 0;
}
static inline long dma_resv_wait_timeout(struct dma_resv *resv,
                                         enum dma_resv_usage usage, bool intr,
                                         unsigned long timeout) {
    (void)usage;
    (void)intr;
    return (!resv || !resv->fence || dma_fence_is_signaled(resv->fence))
               ? timeout
               : 0;
}
static inline void dma_resv_iter_begin(struct dma_resv_iter *iter,
                                       struct dma_resv *resv,
                                       enum dma_resv_usage usage) {
    if (!iter)
        return;
    iter->resv = resv;
    iter->fence = resv ? resv->fence : NULL;
    iter->usage = usage;
    iter->done = false;
}
static inline void dma_resv_iter_end(struct dma_resv_iter *iter) { (void)iter; }
static inline bool dma_resv_iter_is_restarted(struct dma_resv_iter *iter) {
    (void)iter;
    return false;
}
static inline enum dma_resv_usage
dma_resv_iter_usage(struct dma_resv_iter *iter) {
    return iter ? iter->usage : DMA_RESV_USAGE_KERNEL;
}

#define dma_resv_for_each_fence(iter, resv, usage, fence)                      \
    for (dma_resv_iter_begin((iter), (resv), (usage)),                         \
         (fence) = (iter)->fence;                                              \
         !(iter)->done && (fence) != NULL;                                     \
         (iter)->done = true, (fence) = NULL)

#define dma_resv_for_each_fence_unlocked(iter, fence)                          \
    for ((fence) = (iter)->fence; !(iter)->done && (fence) != NULL;            \
         (iter)->done = true, (fence) = NULL)
