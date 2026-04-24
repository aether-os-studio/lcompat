#pragma once

#include <drivers/logger.h>
#include <libs/klibc.h>
#include <linux/compiler.h>
#include <linux/debugfs.h>
#include <linux/idr.h>
#include <linux/ktime.h>
#include <linux/types.h>
#include <linux/version.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define EXPORT_SYMBOL(sym)
#define EXPORT_SYMBOL_GPL(sym)
#define __nonstring
#define lockdep_assert_held(lock) ((void)(lock))

#define BIT(nr) (1UL << (nr))
#define BIT_ULL(nr) (1ULL << (nr))

#ifndef IS_ENABLED
#define IS_ENABLED(option) 0
#endif

#define GENMASK(h, l)                                                          \
    (((~0UL) - (1UL << (l)) + 1) &                                             \
     (~0UL >> (sizeof(unsigned long) * 8 - 1 - (h))))
#define GENMASK_ULL(h, l)                                                      \
    (((~0ULL) - (1ULL << (l)) + 1) &                                           \
     (~0ULL >> (sizeof(unsigned long long) * 8 - 1 - (h))))

#define ALIGN(x, a) PADDING_UP((x), (a))
#define ALIGN_DOWN(x, a) PADDING_DOWN((x), (a))
#define round_up(x, a) ALIGN((x), (a))
#define round_down(x, a) ALIGN_DOWN((x), (a))
#define L1_CACHE_ALIGN(x) ALIGN((x), 64)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_CLOSEST(x, divisor)                                          \
    ({                                                                         \
        typeof(x) __x = (x);                                                   \
        typeof(divisor) __d = (divisor);                                       \
        (((__x) + ((__d) / 2)) / (__d));                                       \
    })

#define clamp(val, lo, hi) MIN(MAX((val), (lo)), (hi))
#define clamp_t(type, val, lo, hi)                                             \
    ((type)clamp((type)(val), (type)(lo), (type)(hi)))
#define max(x, y) MAX((x), (y))
#define min(x, y) MIN((x), (y))
#define max_t(type, x, y) ((type)MAX((type)(x), (type)(y)))
#define min_t(type, x, y) ((type)MIN((type)(x), (type)(y)))
#define min3(x, y, z) MIN(MIN((x), (y)), (z))
#define max3(x, y, z) MAX(MAX((x), (y)), (z))
#define ilog2(x) ((x) ? (31 - __builtin_clz((unsigned int)(x))) : 0)
#define ffs(x) __builtin_ffs((int)(x))
#define div_u64(x, y) ((u64)(x) / (u64)(y))

#define READ_ONCE(x) (*(volatile typeof(x) *)&(x))
#define WRITE_ONCE(x, val) (*(volatile typeof(x) *)&(x) = (val))

#define cmpxchg(ptr, oldv, newv)                                               \
    __sync_val_compare_and_swap((ptr), (oldv), (newv))
#define try_cmpxchg(ptr, oldp, newv)                                           \
    ({                                                                         \
        typeof(*(oldp)) __old = *(oldp);                                       \
        typeof(__old) __ret =                                                  \
            __sync_val_compare_and_swap((ptr), __old, (newv));                 \
        bool __ok = (__ret == __old);                                          \
        if (!__ok)                                                             \
            *(oldp) = __ret;                                                   \
        __ok;                                                                  \
    })

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define upper_32_bits(n) ((u32)(((u64)(n) >> 32) & 0xffffffffU))
#define lower_32_bits(n) ((u32)((u64)(n) & 0xffffffffU))

#define __stringify_1(x) #x
#define __stringify(x) __stringify_1(x)

#define BITS_PER_TYPE(type) (sizeof(type) * 8)
#define typeof_member(type, member) typeof(((type *)0)->member)
#define BUILD_BUG_ON_INVALID(e) ((void)sizeof((long)(e)))
#define BUILD_BUG_ON(e) ((void)sizeof(char[1 - 2 * !!(e)]))
#define offsetofend(type, member)                                              \
    (offsetof(type, member) + sizeof(((type *)0)->member))
#define struct_size(ptr, member, count)                                        \
    (sizeof(*(ptr)) + sizeof((ptr)->member[0]) * (count))

#define pr_emerg(fmt, ...) printk("emerg: " fmt, ##__VA_ARGS__)
#define pr_alert(fmt, ...) printk("alert: " fmt, ##__VA_ARGS__)
#define pr_crit(fmt, ...) printk("crit: " fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...) printk("err: " fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...) printk("warn: " fmt, ##__VA_ARGS__)
#define pr_notice(fmt, ...) printk("notice: " fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...) printk("info: " fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...) printk("debug: " fmt, ##__VA_ARGS__)

#define no_printk(fmt, ...) ((void)sizeof(fmt))

#define might_sleep() ((void)0)
#define might_sleep_if(cond) ((void)(cond))
#define might_lock(lock) ((void)(lock))
#define data_race(expr) (expr)

#define LOCKDEP_STILL_OK 0

#define TAINT_WARN 9
#define TAINT_USER 10
#define TAINT_MACHINE_CHECK 11
#define KERN_DEBUG ""
#define KERN_INFO ""
#define DUMP_PREFIX_OFFSET 0
#define S8_MIN ((s8) - 128)
#define S8_MAX ((s8)127)
#define U32_MAX ((u32)0xffffffffU)
#define U8_MAX ((u8)0xff)
#define NUMA_NO_NODE (-1)
#define EPROBE_DEFER 517

#ifndef ENOTSUPP
#define ENOTSUPP ENOTSUP
#endif

#ifndef abs
#define abs(x)                                                                 \
    ({                                                                         \
        typeof(x) __x = (x);                                                   \
        __x < 0 ? -__x : __x;                                                  \
    })
#endif

static inline void add_taint(unsigned flag, int lockdep_ok) {
    (void)flag;
    (void)lockdep_ok;
}

static inline s32 sign_extend32(u32 value, int index) {
    u32 shift = 31U - (u32)index;
    return (s32)(value << shift) >> shift;
}

static inline u32 get_unaligned_le32(const void *p) {
    const u8 *b = (const u8 *)p;
    return (u32)b[0] | ((u32)b[1] << 8) | ((u32)b[2] << 16) | ((u32)b[3] << 24);
}

static inline u16 get_unaligned_le16(const void *p) {
    const u8 *b = (const u8 *)p;
    return (u16)b[0] | ((u16)b[1] << 8);
}

static inline void put_unaligned_le32(u32 val, void *p) {
    u8 *b = (u8 *)p;
    b[0] = (u8)(val & 0xff);
    b[1] = (u8)((val >> 8) & 0xff);
    b[2] = (u8)((val >> 16) & 0xff);
    b[3] = (u8)((val >> 24) & 0xff);
}

static inline u16 get_unaligned_be16(const void *p) {
    const u8 *b = (const u8 *)p;
    return ((u16)b[0] << 8) | (u16)b[1];
}

static inline void put_unaligned_le16(u16 val, void *p) {
    u8 *b = (u8 *)p;
    b[0] = (u8)(val & 0xff);
    b[1] = (u8)((val >> 8) & 0xff);
}

static inline int strcasecmp(const char *s1, const char *s2) {
    unsigned char c1, c2;

    do {
        c1 = (unsigned char)tolower(*s1++);
        c2 = (unsigned char)tolower(*s2++);
        if (c1 != c2)
            return (int)c1 - (int)c2;
    } while (c1);

    return 0;
}

#define DUMP_ALL 0
static inline void ftrace_dump(int reason) { (void)reason; }
static inline void print_hex_dump(const char *level, const char *prefix_str,
                                  int prefix_type, int rowsize, int groupsize,
                                  const void *buf, size_t len, bool ascii) {
    (void)level;
    (void)prefix_str;
    (void)prefix_type;
    (void)rowsize;
    (void)groupsize;
    (void)buf;
    (void)len;
    (void)ascii;
}
static inline void pr_err_once(const char *fmt, ...) { (void)fmt; }
static inline unsigned long round_jiffies_relative(unsigned long j) {
    return j;
}
static inline void wmb(void) { barrier(); }

#define swap(a, b)                                                             \
    do {                                                                       \
        typeof(a) __tmp = (a);                                                 \
        (a) = (b);                                                             \
        (b) = __tmp;                                                           \
    } while (0)

#define from_timer(var, callback_timer, timer_field)                           \
    container_of(callback_timer, typeof(*(var)), timer_field)

#define WARN_ON(cond)                                                          \
    ({                                                                         \
        bool __cond = !!(cond);                                                \
        if (__cond)                                                            \
            pr_warn("WARN_ON: %s\n", __stringify(cond));                       \
        __cond;                                                                \
    })

#define WARN(cond, fmt, ...)                                                   \
    ({                                                                         \
        bool __cond = !!(cond);                                                \
        if (__cond)                                                            \
            pr_warn(fmt, ##__VA_ARGS__);                                       \
        __cond;                                                                \
    })

#define WARN_ON_ONCE(cond) WARN_ON(cond)
#define WARN_ONCE(cond, fmt, ...) WARN((cond), fmt, ##__VA_ARGS__)

static inline int lockdep_is_held(const void *lock) {
    (void)lock;
    return 1;
}
