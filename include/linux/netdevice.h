#pragma once

#include <linux/skbuff.h>
#include <linux/types.h>

struct ieee80211_channel;

struct net_device {
    char name[16];
    bool threaded;
};

struct napi_struct {
    struct net_device *dev;
};

enum tc_setup_type {
    TC_SETUP_QDISC_MQPRIO = 0,
};

struct survey_info {
    u64 time;
    u64 time_busy;
    u64 time_tx;
    u64 time_rx;
    u64 time_bss_rx;
    int noise;
    struct ieee80211_channel *channel;
    u64 filled;
};

#define NETIF_F_RXCSUM BIT(0)

#define SURVEY_INFO_TIME BIT(0)
#define SURVEY_INFO_TIME_BUSY BIT(1)
#define SURVEY_INFO_TIME_TX BIT(2)
#define SURVEY_INFO_TIME_RX BIT(3)
#define SURVEY_INFO_TIME_BSS_RX BIT(4)
#define SURVEY_INFO_NOISE_DBM BIT(5)
#define SURVEY_INFO_IN_USE BIT(6)

static inline void netif_napi_add(struct net_device *dev,
                                  struct napi_struct *napi,
                                  int (*poll)(struct napi_struct *, int)) {
    (void)dev;
    if (napi)
        napi->dev = dev;
    (void)poll;
}

static inline void netif_napi_add_tx(struct net_device *dev,
                                     struct napi_struct *napi,
                                     int (*poll)(struct napi_struct *, int)) {
    netif_napi_add(dev, napi, poll);
}

static inline void netif_napi_del(struct napi_struct *napi) { (void)napi; }
static inline void napi_enable(struct napi_struct *napi) { (void)napi; }
static inline void napi_disable(struct napi_struct *napi) { (void)napi; }
static inline void napi_schedule(struct napi_struct *napi) { (void)napi; }
static inline bool napi_complete(struct napi_struct *napi) {
    (void)napi;
    return true;
}
static inline bool napi_complete_done(struct napi_struct *napi, int done) {
    (void)napi;
    (void)done;
    return true;
}
static inline void napi_gro_receive(struct napi_struct *napi,
                                    struct sk_buff *skb) {
    (void)napi;
    dev_kfree_skb_any(skb);
}

static inline void netif_receive_skb_list(struct list_head *head) {
    struct sk_buff *skb, *tmp;

    if (!head)
        return;

    list_for_each_entry_safe(skb, tmp, head, list) {
        skb_list_del_init(skb);
        dev_kfree_skb_any(skb);
    }
}

static inline void napi_consume_skb(struct sk_buff *skb, int budget) {
    (void)budget;
    dev_kfree_skb_any(skb);
}
static inline struct sk_buff *napi_build_skb(void *data,
                                             unsigned int frag_size) {
    struct sk_buff *skb = alloc_skb(frag_size ? frag_size : 2048, GFP_ATOMIC);
    (void)data;
    return skb;
}

static inline void *netdev_priv(struct net_device *dev) { return dev; }

static inline struct net_device *alloc_netdev_dummy(int sizeof_priv) {
    size_t total =
        sizeof(struct net_device) + (sizeof_priv > 0 ? (size_t)sizeof_priv : 0);
    return kzalloc(total, GFP_KERNEL);
}

static inline void free_netdev(struct net_device *dev) { kfree(dev); }

static inline int dev_set_threaded(struct net_device *dev, bool threaded) {
    if (!dev)
        return -EINVAL;
    dev->threaded = threaded;
    return 0;
}
