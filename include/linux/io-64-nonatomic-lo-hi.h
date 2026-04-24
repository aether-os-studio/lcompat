#pragma once

#include <linux/io.h>

static inline u64 lo_hi_readq(const volatile void __iomem *addr) {
    return readq(addr);
}

static inline void lo_hi_writeq(u64 value, volatile void __iomem *addr) {
    writeq(value, addr);
}
