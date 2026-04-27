#pragma once

#include <mm/mm.h>
#include <linux/list.h>
#include <linux/types.h>

struct page_pool;

struct page {
    void *addr;
    size_t size;
    struct page_pool *pp;
    dma_addr_t dma_addr;
    struct list_head lcompat_node;
    unsigned int lcompat_refcnt;
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

struct page *virt_to_page(const void *addr);
struct page *virt_to_head_page(const void *addr);
void *page_address(struct page *page);
void get_page(struct page *page);
void put_page(struct page *page);
struct page *__dev_alloc_pages(gfp_t gfp_mask, unsigned int order);
unsigned int get_order(unsigned long size);
