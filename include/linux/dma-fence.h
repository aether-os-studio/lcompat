#pragma once

#include <linux/bitops.h>
#include <linux/hrtimer.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/spinlock.h>

struct dma_fence;
struct dma_fence_cb;

typedef void (*dma_fence_func_t)(struct dma_fence *fence,
                                 struct dma_fence_cb *cb);

struct dma_fence_cb {
    struct list_head node;
    dma_fence_func_t func;
};

struct dma_fence_ops {
    const char *(*get_driver_name)(struct dma_fence *fence);
    const char *(*get_timeline_name)(struct dma_fence *fence);
    bool (*enable_signaling)(struct dma_fence *fence);
    bool (*signaled)(struct dma_fence *fence);
    long (*wait)(struct dma_fence *fence, bool intr, long timeout);
    void (*release)(struct dma_fence *fence);
};

struct dma_fence {
    const struct dma_fence_ops *ops;
    spinlock_t *lock;
    struct kref refcount;
    struct list_head cb_list;
    u64 context;
    u64 seqno;
    unsigned long flags;
    int error;
    ktime_t timestamp;
};

#define DMA_FENCE_FLAG_SIGNALED_BIT 0
#define DMA_FENCE_FLAG_ENABLE_SIGNAL_BIT 1
#define DMA_FENCE_FLAG_TIMESTAMP_BIT 2
#define DMA_FENCE_FLAG_USER_BITS 16

static inline void dma_fence_init(struct dma_fence *fence,
                                  const struct dma_fence_ops *ops,
                                  spinlock_t *lock, u64 context, u64 seqno) {
    if (!fence)
        return;
    fence->ops = ops;
    fence->lock = lock;
    kref_init(&fence->refcount);
    INIT_LIST_HEAD(&fence->cb_list);
    fence->context = context;
    fence->seqno = seqno;
    fence->flags = 0;
    fence->error = 0;
    fence->timestamp = 0;
}

static inline struct dma_fence *dma_fence_get(struct dma_fence *fence) {
    if (fence)
        kref_get(&fence->refcount);
    return fence;
}

static inline struct dma_fence *dma_fence_get_rcu(struct dma_fence *fence) {
    return dma_fence_get(fence);
}

static inline struct dma_fence *
dma_fence_get_rcu_safe(struct dma_fence **fencep) {
    return fencep ? dma_fence_get(*fencep) : NULL;
}

static inline void dma_fence_put(struct dma_fence *fence) {
    if (!fence)
        return;
    kref_put(&fence->refcount, (void (*)(struct kref *))fence->ops->release);
}

static inline bool dma_fence_is_signaled(struct dma_fence *fence) {
    if (!fence)
        return true;
    if (test_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags))
        return true;
    return fence->ops && fence->ops->signaled ? fence->ops->signaled(fence)
                                              : false;
}

static inline long dma_fence_wait(struct dma_fence *fence, bool intr) {
    (void)intr;
    return dma_fence_is_signaled(fence) ? 0 : -1;
}

static inline int dma_fence_add_callback(struct dma_fence *fence,
                                         struct dma_fence_cb *cb,
                                         dma_fence_func_t func) {
    if (!fence || !cb)
        return -1;
    cb->func = func;
    INIT_LIST_HEAD(&cb->node);
    list_add_tail(&cb->node, &fence->cb_list);
    if (dma_fence_is_signaled(fence) && func)
        func(fence, cb);
    return 0;
}

static inline bool dma_fence_remove_callback(struct dma_fence *fence,
                                             struct dma_fence_cb *cb) {
    (void)fence;
    if (!cb)
        return false;
    list_del_init(&cb->node);
    return true;
}

static inline void dma_fence_signal(struct dma_fence *fence) {
    if (!fence)
        return;
    set_bit(DMA_FENCE_FLAG_SIGNALED_BIT, &fence->flags);
    set_bit(DMA_FENCE_FLAG_TIMESTAMP_BIT, &fence->flags);
    fence->timestamp = ktime_get();
}

static inline bool dma_fence_is_later(struct dma_fence *fence,
                                      struct dma_fence *other) {
    return fence && other && fence->seqno > other->seqno;
}
