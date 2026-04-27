#pragma once

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <net/page_pool.h>

struct sk_buff;

struct skb_shared_info {
    int nr_frags;
    struct sk_buff *frag_list;
    struct {
        struct page *page;
        unsigned int page_offset;
        unsigned int size;
    } frags[8];
};

struct sk_buff {
    struct sk_buff *next;
    struct sk_buff *prev;
    struct list_head list;
    u8 *head;
    u8 *data;
    struct page *head_page;
    unsigned int len;
    unsigned int tail;
    unsigned int end;
    unsigned int priority;
    unsigned int data_len;
    unsigned short queue_mapping;
    int ip_summed;
    __be16 protocol;
    unsigned char cb[128];
    struct skb_shared_info shinfo;
};

#define CHECKSUM_NONE 0
#define CHECKSUM_UNNECESSARY 1

struct sk_buff_head {
    struct sk_buff *head;
    struct sk_buff *tail;
    unsigned int qlen;
    spinlock_t lock;
};

#include <linux/scatterlist.h>

static inline bool lcompat_skb_valid_bounds(const struct sk_buff *skb) {
    unsigned int data_off;
    unsigned int linear_len;

    if (!skb || !skb->head || skb->data < skb->head)
        return false;

    if (skb->data_len > skb->len)
        return false;
    linear_len = skb->len - skb->data_len;

    data_off = (unsigned int)(skb->data - skb->head);
    if (data_off > skb->end || skb->tail > skb->end)
        return false;
    if (linear_len > skb->end - data_off)
        return false;
    if (skb->tail < data_off || linear_len > skb->tail - data_off)
        return false;

    return true;
}

static inline struct sk_buff *alloc_skb(size_t size, gfp_t flags) {
    struct sk_buff *skb = kzalloc(sizeof(*skb), flags);
    struct page *page;
    if (!skb)
        return NULL;
    if (size > UINT_MAX) {
        kfree(skb);
        return NULL;
    }

    page = __dev_alloc_pages(flags, get_order(size ? size : 1));
    if (!page) {
        kfree(skb);
        return NULL;
    }

    skb->head_page = page;
    skb->head = page_address(page);
    skb->data = skb->head;
    skb->end = (unsigned int)size;
    INIT_LIST_HEAD(&skb->list);
    return skb;
}

static inline struct sk_buff *dev_alloc_skb(size_t size) {
    return alloc_skb(size, GFP_KERNEL);
}

static inline void lcompat_skb_release_frags(struct sk_buff *skb) {
    int i;

    if (!skb)
        return;

    for (i = 0; i < skb->shinfo.nr_frags; i++) {
        if (skb->shinfo.frags[i].page)
            put_page(skb->shinfo.frags[i].page);
        skb->shinfo.frags[i].page = NULL;
        skb->shinfo.frags[i].page_offset = 0;
        skb->shinfo.frags[i].size = 0;
    }
    skb->shinfo.nr_frags = 0;
    skb->data_len = 0;
}

static inline void kfree_skb(struct sk_buff *skb) {
    if (!skb)
        return;
    lcompat_skb_release_frags(skb);
    if (skb->head_page)
        put_page(skb->head_page);
    else
        kfree(skb->head);
    kfree(skb);
}

static inline void dev_kfree_skb(struct sk_buff *skb) { kfree_skb(skb); }
static inline void dev_kfree_skb_any(struct sk_buff *skb) { kfree_skb(skb); }
static inline void dev_kfree_skb_irq(struct sk_buff *skb) { kfree_skb(skb); }
static inline void consume_skb(struct sk_buff *skb) { kfree_skb(skb); }
static inline struct sk_buff *skb_get(struct sk_buff *skb) { return skb; }
static inline unsigned char *skb_mac_header(const struct sk_buff *skb) {
    return skb ? skb->data : NULL;
}

static inline int skb_cow_head(struct sk_buff *skb, unsigned int headroom) {
    unsigned int data_off;
    unsigned int tailroom;
    unsigned int linear_len;
    unsigned int new_end;
    struct page *new_page;
    u8 *new_head;

    if (!lcompat_skb_valid_bounds(skb))
        return -EINVAL;

    data_off = (unsigned int)(skb->data - skb->head);
    if (data_off >= headroom)
        return 0;

    linear_len = skb->len - skb->data_len;
    tailroom = skb->end - skb->tail;
    if (headroom > UINT_MAX - linear_len ||
        headroom + linear_len > UINT_MAX - tailroom)
        return -ENOMEM;

    new_end = headroom + linear_len + tailroom;
    new_page = __dev_alloc_pages(GFP_ATOMIC, get_order(new_end ? new_end : 1));
    if (!new_page)
        return -ENOMEM;
    new_head = page_address(new_page);

    memcpy(new_head + headroom, skb->data, linear_len);
    if (skb->head_page)
        put_page(skb->head_page);
    else
        kfree(skb->head);
    skb->head = new_head;
    skb->head_page = new_page;
    skb->data = new_head + headroom;
    skb->tail = headroom + linear_len;
    skb->end = new_end;
    return 0;
}

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
    unsigned int data_off;
    unsigned int linear_len;

    if (!lcompat_skb_valid_bounds(skb))
        return NULL;
    new_skb = alloc_skb(skb->end, flags);
    if (!new_skb)
        return NULL;
    data_off = (unsigned int)(skb->data - skb->head);
    linear_len = skb->len - skb->data_len;
    new_skb->data = new_skb->head + data_off;
    new_skb->tail = data_off + linear_len;
    new_skb->len = linear_len;
    new_skb->priority = skb->priority;
    new_skb->queue_mapping = skb->queue_mapping;
    memcpy(new_skb->cb, skb->cb, sizeof(new_skb->cb));
    memcpy(new_skb->data, skb->data, linear_len);
    return new_skb;
}

static inline void skb_queue_purge(struct sk_buff_head *list) {
    struct sk_buff *skb;
    while ((skb = skb_dequeue(list)) != NULL)
        kfree_skb(skb);
}

static inline void *skb_put(struct sk_buff *skb, unsigned int len) {
    void *pos;

    if (!lcompat_skb_valid_bounds(skb) || len > skb->end - skb->tail ||
        len > UINT_MAX - skb->len)
        return NULL;
    pos = skb->head + skb->tail;
    skb->tail += len;
    skb->len += len;
    return pos;
}

static inline void skb_reserve(struct sk_buff *skb, unsigned int len) {
    if (!lcompat_skb_valid_bounds(skb) || len > skb->end - skb->tail)
        return;
    skb->data += len;
    skb->tail += len;
}

static inline void *skb_push(struct sk_buff *skb, unsigned int len) {
    if (!lcompat_skb_valid_bounds(skb) ||
        (unsigned int)(skb->data - skb->head) < len ||
        len > UINT_MAX - skb->len)
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
    unsigned int data_off;
    unsigned int linear_len;
    unsigned int copied;
    unsigned int new_size;
    struct page *new_page;
    u8 *new_head;

    if (!lcompat_skb_valid_bounds(skb))
        return -EINVAL;
    if (!skb->data_len)
        return 0;

    data_off = (unsigned int)(skb->data - skb->head);
    linear_len = skb->len - skb->data_len;
    if (skb->len > UINT_MAX - data_off)
        return -ENOMEM;
    new_size = data_off + skb->len;
    new_page =
        __dev_alloc_pages(GFP_ATOMIC, get_order(new_size ? new_size : 1));
    if (!new_page)
        return -ENOMEM;

    new_head = page_address(new_page);
    memcpy(new_head + data_off, skb->data, linear_len);
    copied = linear_len;

    for (int i = 0; i < skb->shinfo.nr_frags && copied < skb->len; i++) {
        struct page *page = skb->shinfo.frags[i].page;
        unsigned int frag_len =
            min_t(unsigned int, skb->shinfo.frags[i].size, skb->len - copied);

        if (!page || !page_address(page)) {
            put_page(new_page);
            return -EINVAL;
        }

        memcpy(new_head + data_off + copied,
               (u8 *)page_address(page) + skb->shinfo.frags[i].page_offset,
               frag_len);
        copied += frag_len;
    }

    if (copied != skb->len) {
        put_page(new_page);
        return -EINVAL;
    }

    lcompat_skb_release_frags(skb);
    if (skb->head_page)
        put_page(skb->head_page);
    else
        kfree(skb->head);

    skb->head_page = new_page;
    skb->head = new_head;
    skb->data = new_head + data_off;
    skb->tail = data_off + skb->len;
    skb->end = data_off + skb->len;
    return 0;
}

static inline struct sk_buff *build_skb(void *data, size_t frag_size) {
    struct sk_buff *skb;

    if (!data)
        return NULL;
    if (frag_size > UINT_MAX)
        return NULL;

    skb = kzalloc(sizeof(*skb), GFP_ATOMIC);
    if (!skb)
        return NULL;

    skb->head = data;
    skb->data = data;
    skb->head_page = virt_to_head_page(data);
    skb->end = (unsigned int)frag_size;
    INIT_LIST_HEAD(&skb->list);

    return skb;
}

static inline struct skb_shared_info *skb_shinfo(struct sk_buff *skb) {
    return skb ? &skb->shinfo : NULL;
}

static inline void skb_add_rx_frag(struct sk_buff *skb, int i,
                                   struct page *page, int off, int size,
                                   int truesize) {
    struct skb_shared_info *shinfo;
    (void)truesize;
    if (!skb || !page || i < 0 || i >= (int)ARRAY_SIZE(skb->shinfo.frags) ||
        off < 0 || size < 0)
        return;

    shinfo = skb_shinfo(skb);
    shinfo->frags[i].page = page;
    shinfo->frags[i].page_offset = (unsigned int)off;
    shinfo->frags[i].size = (unsigned int)size;
    if (i >= shinfo->nr_frags)
        shinfo->nr_frags = i + 1;
    skb->len += (unsigned int)size;
    skb->data_len += (unsigned int)size;
}

static inline void skb_mark_for_recycle(struct sk_buff *skb) { (void)skb; }

static inline unsigned int skb_headlen(const struct sk_buff *skb) {
    return skb && skb->len >= skb->data_len ? skb->len - skb->data_len : 0;
}

static inline int skb_to_sgvec(struct sk_buff *skb, struct scatterlist *sg,
                               int offset, int len) {
    unsigned int linear_len;
    unsigned int skip;
    int nents = 0;
    int remaining = len;

    if (!skb || !sg || offset < 0 || len <= 0 ||
        (unsigned int)offset >= skb->len)
        return 0;

    if (remaining > (int)(skb->len - (unsigned int)offset))
        remaining = (int)skb->len - offset;

    linear_len = skb_headlen(skb);
    skip = (unsigned int)offset;

    if (skip < linear_len && remaining > 0) {
        struct page *page = virt_to_head_page(skb->data + skip);
        unsigned int frag_len =
            min_t(unsigned int, linear_len - skip, (unsigned int)remaining);

        if (!page)
            return 0;
        sg_set_page(
            &sg[nents++], page, frag_len,
            (unsigned int)((skb->data + skip) - (u8 *)page_address(page)));
        remaining -= (int)frag_len;
        skip = 0;
    } else {
        skip -= min_t(unsigned int, skip, linear_len);
    }

    for (int i = 0; i < skb->shinfo.nr_frags && remaining > 0; i++) {
        unsigned int frag_size = skb->shinfo.frags[i].size;
        unsigned int frag_off;
        unsigned int frag_len;
        struct page *page = skb->shinfo.frags[i].page;

        if (skip >= frag_size) {
            skip -= frag_size;
            continue;
        }

        if (!page || nents >= 8)
            break;

        frag_off = skb->shinfo.frags[i].page_offset + skip;
        frag_len =
            min_t(unsigned int, frag_size - skip, (unsigned int)remaining);
        sg_set_page(&sg[nents++], page, frag_len, frag_off);
        remaining -= (int)frag_len;
        skip = 0;
    }

    return remaining == 0 ? nents : 0;
}

#define skb_walk_frags(skb, iter)                                              \
    for ((iter) = NULL; (iter) != NULL; (iter) = NULL)

static inline void *skb_put_data(struct sk_buff *skb, const void *data,
                                 unsigned int len) {
    void *pos = skb_put(skb, len);
    if (pos && data)
        memcpy(pos, data, len);
    return pos;
}

static inline u8 *skb_pull(struct sk_buff *skb, unsigned int len) {
    if (!lcompat_skb_valid_bounds(skb) || len > skb->len)
        return NULL;
    skb->data += len;
    skb->len -= len;
    return skb->data;
}

static inline void skb_trim(struct sk_buff *skb, unsigned int len) {
    unsigned int data_off;
    unsigned int linear_len;

    if (lcompat_skb_valid_bounds(skb) && len <= skb->len) {
        linear_len = skb->len - skb->data_len;
        if (len < linear_len) {
            lcompat_skb_release_frags(skb);
            linear_len = len;
        } else if (len < skb->len) {
            skb->data_len -= skb->len - len;
        }
        data_off = (unsigned int)(skb->data - skb->head);
        if (linear_len > skb->end - data_off)
            return;
        skb->len = len;
        skb->tail = data_off + linear_len;
    }
}

static inline int pskb_trim(struct sk_buff *skb, unsigned int len) {
    if (!skb)
        return -EINVAL;
    skb_trim(skb, len);
    return 0;
}

static inline int skb_pad(struct sk_buff *skb, int pad) {
    unsigned int old_len;
    void *pos;

    if (!skb || pad < 0)
        return -EINVAL;

    old_len = skb->len;
    pos = skb_put(skb, (unsigned int)pad);
    if (!pos)
        return -ENOMEM;

    memset(pos, 0, (size_t)pad);
    skb_trim(skb, old_len);
    return 0;
}

static inline void skb_reset_tail_pointer(struct sk_buff *skb) {
    unsigned int data_off;

    if (!skb)
        return;
    if (!skb->head || skb->data < skb->head)
        return;
    data_off = (unsigned int)(skb->data - skb->head);
    if (data_off > skb->end)
        return;
    skb->len = 0;
    skb->tail = data_off;
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
