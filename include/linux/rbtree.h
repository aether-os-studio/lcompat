#pragma once

#include <libs/rbtree.h>

typedef rb_node_t rb_node;
typedef rb_root_t rb_root;

struct rb_root_cached {
    struct rb_root rb_root;
    struct rb_node *rb_leftmost;
};

#define RB_ROOT ((struct rb_root){.rb_node = NULL})
#define RB_ROOT_CACHED                                                         \
    ((struct rb_root_cached){.rb_root = RB_ROOT, .rb_leftmost = NULL})
#define RB_EMPTY_ROOT(root) ((root)->rb_node == NULL)

static inline struct rb_node *
rb_first_cached(const struct rb_root_cached *root) {
    return root ? root->rb_leftmost : NULL;
}
