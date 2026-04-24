#pragma once

struct llist_node {
    struct llist_node *next;
};

struct llist_head {
    struct llist_node *first;
};

static inline void init_llist_head(struct llist_head *head) {
    head->first = NULL;
}

#define llist_empty(head) (!(head) || (head)->first == NULL)
