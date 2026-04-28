#include "lcompat_native.h"
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <net/page_pool.h>

static LIST_HEAD(lcompat_pages);
static spinlock_t lcompat_pages_lock = SPIN_INIT;

static struct page *lcompat_page_alloc(struct page_pool *pool, size_t size,
                                       gfp_t gfp) {
    struct page *page;
    size_t alloc_size;

    if (!size)
        size = PAGE_SIZE;
    if (size > SIZE_MAX - sizeof(*page))
        return NULL;

    page = kzalloc(sizeof(*page), gfp);
    if (!page)
        return NULL;

    page->addr = alloc_frames_bytes(size);
    page->size = size;
    page->pp = pool;
    page->dma_addr = (dma_addr_t)(uintptr_t)page->addr;
    page->lcompat_refcnt = 1;
    INIT_LIST_HEAD(&page->lcompat_node);

    spin_lock(&lcompat_pages_lock);
    list_add_tail(&page->lcompat_node, &lcompat_pages);
    spin_unlock(&lcompat_pages_lock);

    return page;
}

static void lcompat_page_free(struct page *page) {
    if (!page)
        return;

    spin_lock(&lcompat_pages_lock);
    if (page->lcompat_node.next && page->lcompat_node.prev)
        list_del_init(&page->lcompat_node);
    spin_unlock(&lcompat_pages_lock);

    free_frames_bytes(page->addr, page->size);
    kfree(page);
}

struct page *virt_to_page(const void *addr) { return virt_to_head_page(addr); }

struct page *virt_to_head_page(const void *addr) {
    struct page *page;
    struct page *found = NULL;
    uintptr_t target = (uintptr_t)addr;

    if (!addr)
        return NULL;

    spin_lock(&lcompat_pages_lock);
    list_for_each_entry(page, &lcompat_pages, lcompat_node) {
        uintptr_t start = (uintptr_t)page->addr;
        uintptr_t end = start + page->size;

        if (target >= start && target < end) {
            found = page;
            break;
        }
    }
    spin_unlock(&lcompat_pages_lock);

    return found;
}

void *page_address(struct page *page) { return page ? page->addr : NULL; }

void get_page(struct page *page) {
    if (!page)
        return;
    __atomic_add_fetch(&page->lcompat_refcnt, 1, __ATOMIC_ACQUIRE);
}

void put_page(struct page *page) {
    if (!page)
        return;
    if (__atomic_sub_fetch(&page->lcompat_refcnt, 1, __ATOMIC_RELEASE) == 0)
        lcompat_page_free(page);
}

struct page *__dev_alloc_pages(gfp_t gfp_mask, unsigned int order) {
    size_t size = PAGE_SIZE;

    if (order >= sizeof(size_t) * 8)
        return NULL;
    size <<= order;

    return lcompat_page_alloc(NULL, size, gfp_mask);
}

unsigned int get_order(unsigned long size) {
    unsigned int order = 0;
    unsigned long pages;

    if (size <= PAGE_SIZE)
        return 0;

    pages = (size - 1) >> PAGE_SHIFT;
    while (pages) {
        pages >>= 1;
        order++;
    }

    return order;
}

struct page_pool *page_pool_create(const struct page_pool_params *params) {
    struct page_pool *pool;

    pool = kzalloc(sizeof(*pool), GFP_KERNEL);
    if (!pool)
        return NULL;
    if (params)
        pool->params = *params;
    return pool;
}

void page_pool_destroy(struct page_pool *pool) {
    struct page *page;

    if (!pool)
        return;

    spin_lock(&lcompat_pages_lock);
    list_for_each_entry(page, &lcompat_pages, lcompat_node) {
        if (page->pp == pool)
            page->pp = NULL;
    }
    spin_unlock(&lcompat_pages_lock);

    kfree(pool);
}

struct page *page_pool_alloc_frag(struct page_pool *pool, u32 *offset, u32 size,
                                  gfp_t gfp) {
    struct page *page;

    page = lcompat_page_alloc(pool, size ? size : PAGE_SIZE, gfp);
    if (!page) {
        if (pool)
            pool->stats.alloc_empty++;
        return NULL;
    }

    if (offset)
        *offset = 0;
    if (pool)
        pool->stats.alloc_slow++;

    return page;
}

void page_pool_put_full_page(struct page_pool *pool, struct page *page,
                             bool allow_direct) {
    (void)allow_direct;
    if (pool)
        pool->stats.release_slow++;
    put_page(page);
}
