#pragma once

#include <linux/types.h>

struct net_device;
struct mtk_wed_bm_desc {
    dma_addr_t buf0;
    u32 token;
};

struct mtk_wed_ring {
    struct mtk_wed_bm_desc *desc;
};

struct mtk_wed_device {
    struct {
        bool hif2;
        u16 token_start;
        u16 nbuf;
    } wlan;
    struct mtk_wed_ring rx_buf_ring;
    bool active;
};

enum {
    WED_WO_STA_REC = 0,
};

static inline bool mtk_wed_device_active(struct mtk_wed_device *wed) {
    return wed ? wed->active : false;
}

static inline u32 mtk_wed_device_reg_read(struct mtk_wed_device *wed, u32 reg) {
    (void)wed;
    (void)reg;
    return 0;
}

static inline void mtk_wed_device_reg_write(struct mtk_wed_device *wed, u32 reg,
                                            u32 val) {
    (void)wed;
    (void)reg;
    (void)val;
}

static inline int mtk_wed_device_tx_ring_setup(struct mtk_wed_device *wed,
                                               void *ring, void *regs,
                                               bool reset) {
    (void)wed;
    (void)ring;
    (void)regs;
    (void)reset;
    return 0;
}

static inline int mtk_wed_device_txfree_ring_setup(struct mtk_wed_device *wed,
                                                   void *regs) {
    (void)wed;
    (void)regs;
    return 0;
}

static inline int mtk_wed_device_rx_ring_setup(struct mtk_wed_device *wed,
                                               void *ring, void *regs,
                                               bool reset) {
    (void)wed;
    (void)ring;
    (void)regs;
    (void)reset;
    return 0;
}

static inline int mtk_wed_device_rro_rx_ring_setup(struct mtk_wed_device *wed,
                                                   void *ring, void *regs) {
    (void)wed;
    (void)ring;
    (void)regs;
    return 0;
}

static inline int
mtk_wed_device_msdu_pg_rx_ring_setup(struct mtk_wed_device *wed, void *ring,
                                     void *regs) {
    (void)wed;
    (void)ring;
    (void)regs;
    return 0;
}

static inline int mtk_wed_device_ind_rx_ring_setup(struct mtk_wed_device *wed,
                                                   void *regs) {
    (void)wed;
    (void)regs;
    return 0;
}

static inline int mtk_wed_device_update_msg(struct mtk_wed_device *wed, int msg,
                                            void *data, int len) {
    (void)wed;
    (void)msg;
    (void)data;
    (void)len;
    return 0;
}

static inline int mtk_wed_get_rx_capa(struct mtk_wed_device *wed) {
    (void)wed;
    return 0;
}

static inline int mtk_wed_device_setup_tc(struct mtk_wed_device *wed,
                                          struct net_device *netdev, int type,
                                          void *type_data) {
    (void)wed;
    (void)netdev;
    (void)type;
    (void)type_data;
    return 0;
}

static inline void mtk_wed_device_detach(struct mtk_wed_device *wed) {
    if (wed)
        wed->active = false;
}

static inline int mtk_wed_device_attach(struct mtk_wed_device *wed) {
    (void)wed;
    return 0;
}

static inline void mtk_wed_device_irq_set_mask(struct mtk_wed_device *wed,
                                               u32 mask) {
    (void)wed;
    (void)mask;
}

static inline u32 mtk_wed_device_irq_get(struct mtk_wed_device *wed, u32 mask) {
    (void)wed;
    (void)mask;
    return 0;
}

static inline void mtk_wed_device_ppe_check(struct mtk_wed_device *wed,
                                            void *skb, int reason,
                                            bool *check) {
    (void)wed;
    (void)skb;
    (void)reason;
    if (check)
        *check = false;
}

static inline void mtk_wed_device_start(struct mtk_wed_device *wed,
                                        u32 irq_mask) {
    (void)wed;
    (void)irq_mask;
}

static inline void mtk_wed_device_stop(struct mtk_wed_device *wed) {
    (void)wed;
}

static inline void mtk_wed_device_start_hw_rro(struct mtk_wed_device *wed,
                                               u32 irq_mask, bool reset) {
    (void)wed;
    (void)irq_mask;
    (void)reset;
}
