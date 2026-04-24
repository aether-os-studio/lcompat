#pragma once

#include <linux/scatterlist.h>

enum dma_data_direction {
    DMA_BIDIRECTIONAL = 0,
    DMA_TO_DEVICE = 1,
    DMA_FROM_DEVICE = 2,
    DMA_NONE = 3,
};

#define DMA_ATTR_SKIP_CPU_SYNC BIT(0)
#define DMA_ATTR_NO_KERNEL_MAPPING BIT(1)
#define DMA_ATTR_NO_WARN BIT(2)

#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL << (n)) - 1))

struct device;
struct page;

static inline int dma_set_mask(struct device *dev, u64 mask) {
    (void)dev;
    (void)mask;
    return 0;
}

static inline int dma_set_coherent_mask(struct device *dev, u64 mask) {
    (void)dev;
    (void)mask;
    return 0;
}

static inline int dma_set_max_seg_size(struct device *dev, unsigned int size) {
    (void)dev;
    (void)size;
    return 0;
}

static inline dma_addr_t dma_map_page(struct device *dev, struct page *page,
                                      size_t offset, size_t size,
                                      enum dma_data_direction dir) {
    (void)dev;
    (void)page;
    (void)offset;
    (void)size;
    (void)dir;
    return 0;
}

static inline dma_addr_t dma_map_single(struct device *dev, void *cpu_addr,
                                        size_t size,
                                        enum dma_data_direction dir) {
    (void)dev;
    (void)size;
    (void)dir;
    return (dma_addr_t)(uintptr_t)cpu_addr;
}

static inline void dma_unmap_page(struct device *dev, dma_addr_t addr,
                                  size_t size, enum dma_data_direction dir) {
    (void)dev;
    (void)addr;
    (void)size;
    (void)dir;
}

static inline void dma_unmap_single(struct device *dev, dma_addr_t addr,
                                    size_t size, enum dma_data_direction dir) {
    (void)dev;
    (void)addr;
    (void)size;
    (void)dir;
}

static inline int dma_mapping_error(struct device *dev, dma_addr_t addr) {
    (void)dev;
    (void)addr;
    return 0;
}

static inline void dma_sync_single_for_cpu(struct device *dev, dma_addr_t addr,
                                           size_t size,
                                           enum dma_data_direction dir) {
    (void)dev;
    (void)addr;
    (void)size;
    (void)dir;
}

static inline void dma_sync_single_for_device(struct device *dev,
                                              dma_addr_t addr, size_t size,
                                              enum dma_data_direction dir) {
    (void)dev;
    (void)addr;
    (void)size;
    (void)dir;
}

static inline int dma_map_sg(struct device *dev, struct scatterlist *sgl,
                             int nents, enum dma_data_direction dir) {
    (void)dev;
    (void)sgl;
    (void)dir;
    return nents;
}

static inline int dma_map_sg_attrs(struct device *dev, struct scatterlist *sgl,
                                   int nents, enum dma_data_direction dir,
                                   unsigned long attrs) {
    (void)attrs;
    return dma_map_sg(dev, sgl, nents, dir);
}

static inline void dma_unmap_sg(struct device *dev, struct scatterlist *sgl,
                                int nents, enum dma_data_direction dir) {
    (void)dev;
    (void)sgl;
    (void)nents;
    (void)dir;
}

static inline int dma_map_sgtable(struct device *dev, struct sg_table *sgt,
                                  enum dma_data_direction dir,
                                  unsigned long attrs) {
    (void)attrs;
    if (!sgt)
        return -EINVAL;
    return dma_map_sg(dev, sgt->sgl, (int)sgt->nents, dir) > 0 ? 0 : -EINVAL;
}

static inline void dma_unmap_sgtable(struct device *dev, struct sg_table *sgt,
                                     enum dma_data_direction dir,
                                     unsigned long attrs) {
    (void)attrs;
    if (!sgt)
        return;
    dma_unmap_sg(dev, sgt->sgl, (int)sgt->nents, dir);
}

static inline void *dma_alloc_coherent(struct device *dev, size_t size,
                                       dma_addr_t *dma_handle, gfp_t gfp) {
    void *ptr;

    (void)dev;
    (void)gfp;
    ptr = kzalloc(size, GFP_KERNEL);
    if (dma_handle)
        *dma_handle = (dma_addr_t)(uintptr_t)ptr;
    return ptr;
}

static inline void dma_free_coherent(struct device *dev, size_t size,
                                     void *cpu_addr, dma_addr_t dma_handle) {
    (void)dev;
    (void)size;
    (void)dma_handle;
    kfree(cpu_addr);
}

static inline void *dmam_alloc_coherent(struct device *dev, size_t size,
                                        dma_addr_t *dma_handle, gfp_t gfp) {
    return dma_alloc_coherent(dev, size, dma_handle, gfp);
}
