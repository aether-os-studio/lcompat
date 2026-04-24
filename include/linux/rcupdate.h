#pragma once

#include <linux/kernel.h>
#include <linux/slab.h>

#define __rcu

struct rcu_head {
    void (*func)(struct rcu_head *head);
};

static inline void rcu_assign_pointer(void *p, void *v) {
    WRITE_ONCE(*(void **)p, v);
}

static inline void *rcu_replace_pointer(void *p, void *v, int c) {
    void *old;

    (void)c;
    old = READ_ONCE(*(void **)p);
    WRITE_ONCE(*(void **)p, v);
    return old;
}

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
