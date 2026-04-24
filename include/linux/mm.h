#pragma once

#include <mm/mm.h>
#include <linux/types.h>

struct page_pool;

struct page {
    void *addr;
    struct page_pool *pp;
    dma_addr_t dma_addr;
};

#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096UL
#endif
#ifndef PAGE_MASK
#define PAGE_MASK (~(PAGE_SIZE - 1))
#endif

static inline struct page *virt_to_page(void *addr) {
    if (!addr)
        return NULL;
    return (struct page *)((char *)addr - sizeof(struct page));
}

static inline void *page_address(struct page *page) {
    return page ? page->addr : NULL;
}

static inline struct page *virt_to_head_page(void *addr) {
    return virt_to_page(addr);
}
