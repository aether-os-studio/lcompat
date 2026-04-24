#pragma once

struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next;
    struct hlist_node **pprev;
};

#define DECLARE_HASHTABLE(name, bits) struct hlist_head name[1 << (bits)]
#define DEFINE_HASHTABLE(name, bits) struct hlist_head name[1 << (bits)] = {0}
