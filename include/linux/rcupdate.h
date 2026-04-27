#pragma once

#include <linux/kernel.h>
#include <linux/slab.h>

#define __rcu

struct rcu_head {
    void (*func)(struct rcu_head *head);
};

#define rcu_assign_pointer(p, v)                                               \
    do {                                                                       \
        WRITE_ONCE((p), (v));                                                  \
    } while (0)

#define rcu_replace_pointer(p, v, c)                                           \
    ({                                                                         \
        typeof(p) __old = READ_ONCE(p);                                        \
        (void)(c);                                                             \
        WRITE_ONCE((p), (v));                                                  \
        __old;                                                                 \
    })

#define rcu_dereference(p) READ_ONCE(p)
#define rcu_dereference_protected(p, c) (p)
#define rcu_access_pointer(p) READ_ONCE(p)
#define rcu_read_lock() ((void)0)
#define rcu_read_unlock() ((void)0)

static inline void call_rcu(struct rcu_head *head,
                            void (*func)(struct rcu_head *)) {
    if (!head || !func)
        return;
    func(head);
}

#define kfree_rcu(ptr, member) kfree(ptr)
