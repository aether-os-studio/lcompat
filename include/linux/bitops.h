#pragma once

#include <linux/kernel.h>

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BITS_TO_LONGS(nr) DIV_ROUND_UP((nr), BITS_PER_LONG)
#define DECLARE_BITMAP(name, bits) unsigned long name[BITS_TO_LONGS(bits)]

static inline void set_bit(unsigned int nr, volatile unsigned long *addr) {
    unsigned long mask = BIT(nr);

    __atomic_fetch_or((unsigned long *)addr, mask, __ATOMIC_ACQ_REL);
}

static inline void __set_bit(unsigned int nr, volatile unsigned long *addr) {
    set_bit(nr, addr);
}

static inline void clear_bit(unsigned int nr, volatile unsigned long *addr) {
    unsigned long mask = BIT(nr);

    __atomic_fetch_and((unsigned long *)addr, ~mask, __ATOMIC_ACQ_REL);
}

#define __clear_bit(nr, addr) ((addr) &= ~((typeof(addr))BIT(nr)))

static inline void clear_bit_unlock(unsigned int nr,
                                    volatile unsigned long *addr) {
    clear_bit(nr, addr);
}

static inline bool test_bit(unsigned int nr,
                            const volatile unsigned long *addr) {
    unsigned long mask = BIT(nr);
    unsigned long val =
        __atomic_load_n((const unsigned long *)addr, __ATOMIC_ACQUIRE);

    return !!(val & mask);
}

static inline bool test_and_set_bit(unsigned int nr,
                                    volatile unsigned long *addr) {
    unsigned long mask = BIT(nr);
    unsigned long old =
        __atomic_fetch_or((unsigned long *)addr, mask, __ATOMIC_ACQ_REL);

    return !!(old & mask);
}

static inline bool test_and_clear_bit(unsigned int nr,
                                      volatile unsigned long *addr) {
    unsigned long mask = BIT(nr);
    unsigned long old =
        __atomic_fetch_and((unsigned long *)addr, ~mask, __ATOMIC_ACQ_REL);

    return !!(old & mask);
}

static inline unsigned long find_first_bit(const unsigned long *addr,
                                           unsigned long size) {
    for (unsigned long i = 0; i < size; i++) {
        if (test_bit(i, addr))
            return i;
    }
    return size;
}

static inline unsigned long find_next_bit(const unsigned long *addr,
                                          unsigned long size,
                                          unsigned long offset) {
    for (unsigned long i = offset; i < size; i++) {
        if (test_bit(i, addr))
            return i;
    }
    return size;
}

static inline unsigned long find_first_zero_bit(const unsigned long *addr,
                                                unsigned long size) {
    for (unsigned long i = 0; i < size; i++) {
        if (!test_bit(i, addr))
            return i;
    }
    return size;
}

static inline unsigned long find_next_zero_bit(const unsigned long *addr,
                                               unsigned long size,
                                               unsigned long offset) {
    for (unsigned long i = offset; i < size; i++) {
        if (!test_bit(i, addr))
            return i;
    }
    return size;
}

static inline unsigned long __ffs(unsigned long word) {
    for (unsigned long i = 0; i < BITS_PER_LONG; i++) {
        if (word & BIT(i))
            return i;
    }
    return 0;
}

static inline unsigned long __ffs64(u64 word) {
    return word ? (unsigned long)__builtin_ctzll(word) : 0;
}

static inline unsigned long __fls(unsigned long word) {
    for (long i = (long)BITS_PER_LONG - 1; i >= 0; i--) {
        if (word & BIT(i))
            return (unsigned long)i;
    }
    return 0;
}

static inline int fls(unsigned int word) {
    return word ? (int)__fls(word) + 1 : 0;
}

static inline unsigned int hweight8(u8 w) {
    return __builtin_popcount((unsigned int)w);
}
static inline unsigned int hweight16(u16 w) {
    return __builtin_popcount((unsigned int)w);
}
static inline unsigned int hweight32(u32 w) { return __builtin_popcount(w); }
static inline unsigned int hweight64(u64 w) { return __builtin_popcountll(w); }

static inline void bitmap_zero(unsigned long *dst, unsigned long nbits) {
    if (!dst)
        return;
    memset(dst, 0, BITS_TO_LONGS(nbits) * sizeof(unsigned long));
}

#define for_each_set_bit(bit, addr, size)                                      \
    for ((bit) = find_first_bit((addr), (size)); (bit) < (size);               \
         (bit) = find_next_bit((addr), (size), (bit) + 1))
