#pragma once

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/list.h>

struct page;

struct sk_buff {
    struct sk_buff *next;
    struct sk_buff *prev;
    struct list_head list;
    u8 *head;
    u8 *data;
    unsigned int len;
    unsigned int tail;
    unsigned int end;
    unsigned int priority;
    unsigned int data_len;
    unsigned short queue_mapping;
    __be16 protocol;
    unsigned char cb[128];
    int nr_frags;
};

struct sk_buff_head {
    struct sk_buff *head;
    struct sk_buff *tail;
    unsigned int qlen;
    spinlock_t lock;
};

static inline struct sk_buff *alloc_skb(unsigned int size, gfp_t flags) {
    struct sk_buff *skb = kzalloc(sizeof(*skb), flags);
    if (!skb)
        return NULL;

    skb->head = alloc_frames_bytes(size ? size : 1);
    if (!skb->head) {
        kfree(skb);
        return NULL;
    }

    skb->data = skb->head;
    skb->end = size;
    INIT_LIST_HEAD(&skb->list);
    return skb;
}

static inline struct sk_buff *dev_alloc_skb(unsigned int size) {
    return alloc_skb(size, GFP_KERNEL);
}

static inline void kfree_skb(struct sk_buff *skb) {
    if (!skb)
        return;
    free_frames_bytes(skb->head, skb->end);
    kfree(skb);
}

static inline void dev_kfree_skb(struct sk_buff *skb) { kfree_skb(skb); }
static inline void dev_kfree_skb_any(struct sk_buff *skb) { kfree_skb(skb); }
static inline void dev_kfree_skb_irq(struct sk_buff *skb) { kfree_skb(skb); }
static inline void consume_skb(struct sk_buff *skb) { kfree_skb(skb); }
static inline struct sk_buff *skb_get(struct sk_buff *skb) { return skb; }

static inline void skb_queue_head_init(struct sk_buff_head *list) {
    if (!list)
        return;
    memset(list, 0, sizeof(*list));
    list->lock = SPIN_INIT;
}

static inline void __skb_queue_head_init(struct sk_buff_head *list) {
    skb_queue_head_init(list);
}

static inline struct sk_buff *skb_peek(struct sk_buff_head *list) {
    return list ? list->head : NULL;
}

static inline bool skb_queue_empty(const struct sk_buff_head *list) {
    return !list || list->qlen == 0;
}

static inline unsigned int skb_queue_len(const struct sk_buff_head *list) {
    return list ? list->qlen : 0;
}

static inline void __skb_queue_tail(struct sk_buff_head *list,
                                    struct sk_buff *skb) {
    skb->next = NULL;
    if (list->tail)
        list->tail->next = skb;
    else
        list->head = skb;
    list->tail = skb;
    list->qlen++;
}

static inline void skb_queue_tail(struct sk_buff_head *list,
                                  struct sk_buff *skb) {
    if (!list || !skb)
        return;
    spin_lock(&list->lock);
    __skb_queue_tail(list, skb);
    spin_unlock(&list->lock);
}

static inline void skb_queue_head(struct sk_buff_head *list,
                                  struct sk_buff *skb) {
    if (!list || !skb)
        return;
    spin_lock(&list->lock);
    skb->next = list->head;
    list->head = skb;
    if (!list->tail)
        list->tail = skb;
    list->qlen++;
    spin_unlock(&list->lock);
}

static inline struct sk_buff *skb_dequeue(struct sk_buff_head *list) {
    struct sk_buff *skb;
    if (!list)
        return NULL;
    spin_lock(&list->lock);
    skb = list->head;
    if (skb) {
        list->head = skb->next;
        if (!list->head)
            list->tail = NULL;
        list->qlen--;
        skb->next = NULL;
    }
    spin_unlock(&list->lock);
    return skb;
}

static inline struct sk_buff *__skb_dequeue(struct sk_buff_head *list) {
    struct sk_buff *skb;

    if (!list)
        return NULL;
    skb = list->head;
    if (skb) {
        list->head = skb->next;
        if (!list->head)
            list->tail = NULL;
        list->qlen--;
        skb->next = NULL;
    }
    return skb;
}

static inline void __skb_unlink(struct sk_buff *skb,
                                struct sk_buff_head *list) {
    struct sk_buff *prev = NULL;
    struct sk_buff *cur;

    if (!skb || !list)
        return;

    for (cur = list->head; cur; cur = cur->next) {
        if (cur != skb) {
            prev = cur;
            continue;
        }

        if (prev)
            prev->next = cur->next;
        else
            list->head = cur->next;
        if (list->tail == cur)
            list->tail = prev;
        if (list->qlen > 0)
            list->qlen--;
        cur->next = NULL;
        break;
    }
}

static inline void skb_unlink(struct sk_buff *skb, struct sk_buff_head *list) {
    if (!list)
        return;
    spin_lock(&list->lock);
    __skb_unlink(skb, list);
    spin_unlock(&list->lock);
}

static inline void skb_list_del_init(struct sk_buff *skb) {
    if (skb)
        list_del_init(&skb->list);
}

static inline struct sk_buff *skb_copy(const struct sk_buff *skb, gfp_t flags) {
    struct sk_buff *new_skb;

    if (!skb)
        return NULL;
    new_skb = alloc_skb(skb->end, flags);
    if (!new_skb)
        return NULL;
    new_skb->priority = skb->priority;
    new_skb->queue_mapping = skb->queue_mapping;
    memcpy(new_skb->cb, skb->cb, sizeof(new_skb->cb));
    memcpy(new_skb->data, skb->data, skb->len);
    new_skb->len = skb->len;
    new_skb->tail = skb->len;
    return new_skb;
}

static inline void skb_queue_purge(struct sk_buff_head *list) {
    struct sk_buff *skb;
    while ((skb = skb_dequeue(list)) != NULL)
        kfree_skb(skb);
}

static inline void *skb_put(struct sk_buff *skb, unsigned int len) {
    void *pos = skb ? skb->head + skb->tail : NULL;
    if (!skb || skb->tail + len > skb->end)
        return NULL;
    skb->tail += len;
    skb->len += len;
    return pos;
}

static inline void skb_reserve(struct sk_buff *skb, unsigned int len) {
    if (!skb || len > skb->end)
        return;
    skb->data += len;
    skb->tail += len;
}

static inline void *skb_push(struct sk_buff *skb, unsigned int len) {
    if (!skb || (unsigned int)(skb->data - skb->head) < len)
        return NULL;
    skb->data -= len;
    skb->len += len;
    return skb->data;
}

static inline void *skb_put_zero(struct sk_buff *skb, unsigned int len) {
    void *pos = skb_put(skb, len);
    if (pos)
        memset(pos, 0, len);
    return pos;
}

static inline void *__skb_put(struct sk_buff *skb, unsigned int len) {
    return skb_put(skb, len);
}

static inline void *__skb_put_zero(struct sk_buff *skb, unsigned int len) {
    return skb_put_zero(skb, len);
}

static inline void *__skb_push(struct sk_buff *skb, unsigned int len) {
    return skb_push(skb, len);
}

#define SKB_WITH_OVERHEAD(x) (x)

static inline int skb_linearize(struct sk_buff *skb) {
    (void)skb;
    return 0;
}

static inline struct sk_buff *build_skb(void *data, unsigned int frag_size) {
    (void)data;
    return alloc_skb(frag_size ? frag_size : 2048, GFP_ATOMIC);
}

struct skb_shared_info {
    int nr_frags;
    struct sk_buff *frag_list;
    struct {
        struct page *page;
        unsigned int page_offset;
        unsigned int size;
    } frags[8];
};

static inline struct skb_shared_info *skb_shinfo(struct sk_buff *skb) {
    return (struct skb_shared_info *)&skb->nr_frags;
}

static inline void skb_add_rx_frag(struct sk_buff *skb, int i,
                                   struct page *page, int off, int size,
                                   int truesize) {
    struct skb_shared_info *shinfo;
    (void)truesize;
    if (skb) {
        shinfo = skb_shinfo(skb);
        if (i >= 0 && i < (int)ARRAY_SIZE(shinfo->frags)) {
            shinfo->frags[i].page = page;
            shinfo->frags[i].page_offset = (unsigned int)off;
            shinfo->frags[i].size = (unsigned int)size;
        }
        skb->nr_frags++;
    }
}

static inline void skb_mark_for_recycle(struct sk_buff *skb) { (void)skb; }

static inline unsigned int skb_headlen(const struct sk_buff *skb) {
    return skb ? skb->len : 0;
}

#define skb_walk_frags(skb, iter)                                              \
    for ((iter) = NULL; (iter) != NULL; (iter) = NULL)

static inline void skb_put_data(struct sk_buff *skb, const void *data,
                                unsigned int len) {
    void *pos = skb_put(skb, len);
    if (pos && data)
        memcpy(pos, data, len);
}

static inline u8 *skb_pull(struct sk_buff *skb, unsigned int len) {
    if (!skb || len > skb->len)
        return NULL;
    skb->data += len;
    skb->len -= len;
    return skb->data;
}

static inline void skb_trim(struct sk_buff *skb, unsigned int len) {
    if (skb && len <= skb->len) {
        skb->len = len;
        skb->tail = (unsigned int)(skb->data - skb->head) + len;
    }
}

static inline int skb_pad(struct sk_buff *skb, int pad) {
    void *pos;

    if (!skb || pad < 0)
        return -EINVAL;

    pos = skb_put(skb, (unsigned int)pad);
    if (!pos)
        return -ENOMEM;

    memset(pos, 0, (size_t)pad);
    skb_trim(skb, skb->len - (unsigned int)pad);
    return 0;
}

static inline void skb_reset_tail_pointer(struct sk_buff *skb) {
    if (!skb)
        return;
    skb->data = skb->head;
    skb->len = 0;
    skb->tail = 0;
}

static inline u16 skb_get_queue_mapping(const struct sk_buff *skb) {
    return skb ? skb->queue_mapping : 0;
}

static inline void skb_set_queue_mapping(struct sk_buff *skb,
                                         u16 queue_mapping) {
    if (skb)
        skb->queue_mapping = queue_mapping;
}

static inline void skb_queue_splice_tail_init(struct sk_buff_head *list,
                                              struct sk_buff_head *head) {
    struct sk_buff *skb;

    if (!list || !head)
        return;

    while ((skb = __skb_dequeue(list)) != NULL)
        __skb_queue_tail(head, skb);
}

#define skb_queue_walk_safe(queue, skb, tmp)                                   \
    for ((skb) = (queue)->head, (tmp) = (skb) ? (skb)->next : NULL; (skb);     \
         (skb) = (tmp), (tmp) = (skb) ? (skb)->next : NULL)
