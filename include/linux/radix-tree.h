#pragma once

struct radix_tree_root {
    void *xa_head;
};

static inline void INIT_RADIX_TREE(struct radix_tree_root *root,
                                   unsigned int mask) {
    (void)mask;
    if (root)
        root->xa_head = NULL;
}
