#pragma once

#include <linux/compiler.h>
#include <linux/types.h>
#include <mm/hhdm.h>

static inline void __iomem *ioremap(phys_addr_t phys_addr, size_t size) {
    (void)size;
    return phys_to_virt(phys_addr);
}

static inline void __iomem *ioremap_wc(phys_addr_t phys_addr, size_t size) {
    return ioremap(phys_addr, size);
}

static inline void iounmap(void __iomem *addr) { (void)addr; }

static inline u8 readb(const volatile void __iomem *addr) {
    return *(const volatile u8 *)addr;
}

static inline u16 readw(const volatile void __iomem *addr) {
    return *(const volatile u16 *)addr;
}

static inline u32 readl(const volatile void __iomem *addr) {
    return *(const volatile u32 *)addr;
}

static inline u64 readq(const volatile void __iomem *addr) {
    return *(const volatile u64 *)addr;
}

static inline void writeb(u8 value, volatile void __iomem *addr) {
    *(volatile u8 *)addr = value;
}

static inline void writew(u16 value, volatile void __iomem *addr) {
    *(volatile u16 *)addr = value;
}

static inline void writel(u32 value, volatile void __iomem *addr) {
    *(volatile u32 *)addr = value;
}

static inline void writeq(u64 value, volatile void __iomem *addr) {
    *(volatile u64 *)addr = value;
}

static inline u8 ioread8(const volatile void __iomem *addr) {
    return readb(addr);
}
static inline u16 ioread16(const volatile void __iomem *addr) {
    return readw(addr);
}
static inline u32 ioread32(const volatile void __iomem *addr) {
    return readl(addr);
}
static inline void iowrite8(u8 value, volatile void __iomem *addr) {
    writeb(value, addr);
}
static inline void iowrite16(u16 value, volatile void __iomem *addr) {
    writew(value, addr);
}
static inline void iowrite32(u32 value, volatile void __iomem *addr) {
    writel(value, addr);
}

static inline void memcpy_toio(void __iomem *dst, const void *src,
                               size_t count) {
    memcpy((void *)dst, src, count);
}

static inline void memcpy_fromio(void *dst, const void __iomem *src,
                                 size_t count) {
    memcpy(dst, (const void *)src, count);
}
