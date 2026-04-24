#pragma once

#include <linux/types.h>

#define ETH_ALEN 6
#define ETH_P_PAE 0x888e

static inline bool is_valid_ether_addr(const u8 *addr) {
    return addr && !(addr[0] & 0x01);
}

static inline bool is_multicast_ether_addr(const u8 *addr) {
    return addr && !!(addr[0] & 0x01);
}

static inline bool is_unicast_ether_addr(const u8 *addr) {
    if (!addr || !is_valid_ether_addr(addr))
        return false;
    for (int i = 0; i < ETH_ALEN; i++) {
        if (addr[i] != 0xff)
            return true;
    }
    return false;
}

static inline bool is_broadcast_ether_addr(const u8 *addr) {
    if (!addr)
        return false;
    for (int i = 0; i < ETH_ALEN; i++) {
        if (addr[i] != 0xff)
            return false;
    }
    return true;
}

static inline bool is_zero_ether_addr(const u8 *addr) {
    if (!addr)
        return true;
    for (int i = 0; i < ETH_ALEN; i++) {
        if (addr[i] != 0)
            return false;
    }
    return true;
}

static inline void ether_addr_copy(u8 *dst, const u8 *src) {
    memcpy(dst, src, ETH_ALEN);
}

static inline bool ether_addr_equal(const u8 *addr1, const u8 *addr2) {
    if (!addr1 || !addr2)
        return false;
    return memcmp(addr1, addr2, ETH_ALEN) == 0;
}

static inline void eth_zero_addr(u8 *addr) {
    if (addr)
        memset(addr, 0, ETH_ALEN);
}

static inline void eth_broadcast_addr(u8 *addr) {
    if (addr)
        memset(addr, 0xff, ETH_ALEN);
}

static inline void eth_random_addr(u8 *addr) {
    if (!addr)
        return;
    for (int i = 0; i < ETH_ALEN; i++)
        addr[i] = (u8)(0x10 + i * 37);
    addr[0] &= 0xfe;
    addr[0] |= 0x02;
}

static inline void get_random_mask_addr(u8 *dst, const u8 *addr,
                                        const u8 *mask) {
    if (!dst || !addr || !mask)
        return;
    for (int i = 0; i < ETH_ALEN; i++)
        dst[i] = (addr[i] & mask[i]) | ((u8)(0x5a + i) & ~mask[i]);
}
