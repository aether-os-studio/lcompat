#pragma once

#define __always_inline inline __attribute__((always_inline))
#define __packed __attribute__((packed))
#define __aligned(x) __attribute__((aligned(x)))
#define __printf(a, b) __attribute__((format(printf, a, b)))
#define __bitwise
#define __force
#define __user
#define __acquires(lock)
#define __releases(lock)
#define __malloc
#define __must_check __attribute__((warn_unused_result))
#define __iomem
#define fallthrough __attribute__((fallthrough))
#define barrier() __asm__ __volatile__("" : : : "memory")
#define DECLARE_FLEX_ARRAY(type, member) type member[]
#define struct_group_tagged(TAG, NAME, ...)                                    \
    union {                                                                    \
        struct {                                                               \
            __VA_ARGS__                                                        \
        };                                                                     \
        struct TAG {                                                           \
            __VA_ARGS__                                                        \
        } NAME;                                                                \
    }

#ifndef IS_BUILTIN
#define IS_BUILTIN(option) 0
#endif
#ifndef IS_MODULE
#define IS_MODULE(option) 0
#endif
#ifndef IS_REACHABLE
#define IS_REACHABLE(option) (IS_BUILTIN(option) || IS_MODULE(option))
#endif
