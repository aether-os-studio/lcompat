#pragma once

#include <linux/mm.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/types.h>

struct page;

struct scatterlist {
    struct scatterlist *next;
    struct page *page;
    unsigned int offset;
    unsigned int length;
    dma_addr_t dma_address;
    unsigned int dma_length;
};

struct sg_table {
    struct scatterlist *sgl;
    unsigned int nents;
    unsigned int orig_nents;
};

static inline struct scatterlist *sg_next(struct scatterlist *sg) {
    return sg ? sg->next : NULL;
}

static inline void sg_mark_end(struct scatterlist *sg) {
    if (sg)
        sg->next = NULL;
}

static inline bool sg_is_last(struct scatterlist *sg) {
    return !sg || sg->next == NULL;
}

static inline bool sg_is_chain(struct scatterlist *sg) {
    (void)sg;
    return false;
}

static inline struct scatterlist *sg_chain_ptr(struct scatterlist *sg) {
    return sg ? sg->next : NULL;
}

static inline struct page *sg_page(struct scatterlist *sg) {
    return sg ? sg->page : NULL;
}

static inline void sg_assign_page(struct scatterlist *sg, struct page *page) {
    if (sg)
        sg->page = page;
}

static inline void sg_set_page(struct scatterlist *sg, struct page *page,
                               unsigned int len, unsigned int offset) {
    if (!sg)
        return;
    sg->page = page;
    sg->length = len;
    sg->offset = offset;
}

static inline void *sg_virt(struct scatterlist *sg) {
    return sg && sg->page ? (u8 *)page_address(sg->page) + sg->offset : NULL;
}

#define sg_dma_address(sg) ((sg)->dma_address)
#define sg_dma_len(sg) ((sg)->dma_length)

static inline int sg_alloc_table(struct sg_table *table, unsigned int nents,
                                 gfp_t gfp_mask) {
    (void)gfp_mask;
    if (!table)
        return -1;
    table->sgl = kcalloc(nents, sizeof(*table->sgl), GFP_KERNEL);
    if (!table->sgl)
        return -1;
    table->nents = nents;
    table->orig_nents = nents;
    for (unsigned int i = 0; i + 1 < nents; i++)
        table->sgl[i].next = &table->sgl[i + 1];
    if (nents)
        table->sgl[nents - 1].next = NULL;
    return 0;
}

static inline void sg_free_table(struct sg_table *table) {
    if (!table)
        return;
    kfree(table->sgl);
    table->sgl = NULL;
    table->nents = 0;
    table->orig_nents = 0;
}

static inline void sg_init_table(struct scatterlist *sgl, unsigned int nents) {
    if (!sgl)
        return;
    memset(sgl, 0, nents * sizeof(*sgl));
    for (unsigned int i = 0; i + 1 < nents; i++)
        sgl[i].next = &sgl[i + 1];
    if (nents)
        sgl[nents - 1].next = NULL;
}

static inline void sg_init_marker(struct scatterlist *sgl, unsigned int nents) {
    (void)sgl;
    (void)nents;
}

static inline int skb_to_sgvec(struct sk_buff *skb, struct scatterlist *sg,
                               int offset, int len) {
    if (!skb || !sg || offset < 0 || len <= 0 ||
        (unsigned int)offset >= skb->len)
        return 0;

    if ((unsigned int)(offset + len) > skb->len)
        len = (int)skb->len - offset;

    sg_set_page(sg, virt_to_head_page(skb->data + offset), (unsigned int)len,
                (unsigned int)((skb->data + offset) -
                               (u8 *)page_address(
                                   virt_to_head_page(skb->data + offset))));
    return 1;
}

#define for_each_sg(sgl, sg, nr, i)                                            \
    for ((i) = 0, (sg) = (sgl); (i) < (nr) && (sg) != NULL;                    \
         (i)++, (sg) = sg_next(sg))
