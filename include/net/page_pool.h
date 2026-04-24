#pragma once

#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/slab.h>

struct napi_struct;

struct page_pool_params {
    struct device *dev;
    unsigned int flags;
    unsigned int order;
    unsigned int pool_size;
    unsigned int nid;
    unsigned int dma_dir;
    unsigned int offset;
    unsigned int max_len;
    struct napi_struct *napi;
};

#define PP_FLAG_DMA_MAP BIT(0)
#define PP_FLAG_DMA_SYNC_DEV BIT(1)

struct page_pool {
    struct page_pool_params params;
};

struct page_pool_stats {
    u64 alloc_fast;
    u64 alloc_slow;
    u64 alloc_slow_high_order;
    u64 alloc_empty;
    u64 alloc_refill;
    u64 release_fast;
    u64 release_slow;
    u64 recycle_cached;
    u64 recycle_cache_full;
    u64 recycle_ring;
    u64 recycle_ring_full;
    u64 recycle_released_ref;
};

static inline struct page_pool *
page_pool_create(const struct page_pool_params *params) {
    struct page_pool *pool;

    pool = kzalloc(sizeof(*pool), GFP_KERNEL);
    if (!pool)
        return NULL;
    if (params)
        pool->params = *params;
    return pool;
}

static inline void page_pool_destroy(struct page_pool *pool) { kfree(pool); }

static inline struct page *
page_pool_alloc_frag(struct page_pool *pool, u32 *offset, u32 size, gfp_t gfp) {
    struct page *page;
    size_t alloc_size = sizeof(*page) + (size ? size : PAGE_SIZE);

    (void)gfp;
    page = kzalloc(alloc_size, GFP_KERNEL);
    if (!page)
        return NULL;

    page->pp = pool;
    page->addr = (char *)page + sizeof(*page);
    page->dma_addr = (dma_addr_t)(uintptr_t)page->addr;
    if (offset)
        *offset = 0;

    return page;
}

static inline void page_pool_put_full_page(struct page_pool *pool,
                                           struct page *page,
                                           bool allow_direct) {
    (void)pool;
    (void)allow_direct;
    kfree(page);
}

static inline dma_addr_t page_pool_get_dma_addr(struct page *page) {
    return page ? page->dma_addr : 0;
}

static inline int page_pool_get_dma_dir(struct page_pool *pool) {
    return pool ? (int)pool->params.dma_dir : DMA_BIDIRECTIONAL;
}

static inline void page_pool_get_stats(struct page_pool *pool,
                                       struct page_pool_stats *stats) {
    (void)pool;
    if (stats)
        memset(stats, 0, sizeof(*stats));
}

static inline int page_pool_ethtool_stats_get_count(void) { return 0; }

static inline void
page_pool_ethtool_stats_get(u64 *data, const struct page_pool_stats *stats) {
    (void)data;
    (void)stats;
}

static inline void page_pool_ethtool_stats_get_strings(u8 *data) { (void)data; }
