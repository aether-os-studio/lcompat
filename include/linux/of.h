#pragma once

#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/types.h>

struct property {
    const char *name;
    const void *value;
    int length;
};

typedef u32 phandle;

struct device_node {
    const char *name;
    struct device_node *parent;
};

struct nvmem_cell;
struct wiphy;

static inline struct device_node *of_get_child_by_name(struct device_node *np,
                                                       const char *name) {
    (void)np;
    (void)name;
    return NULL;
}

static inline bool of_device_is_available(const struct device_node *np) {
    (void)np;
    return false;
}

static inline void of_node_put(struct device_node *np) { (void)np; }

static inline bool of_property_read_bool(const struct device_node *np,
                                         const char *propname) {
    (void)np;
    (void)propname;
    return false;
}

static inline int of_property_read_u32(const struct device_node *np,
                                       const char *propname, u32 *out_value) {
    (void)np;
    (void)propname;
    if (out_value)
        *out_value = 0;
    return -ENOENT;
}

static inline const void *of_get_property(const struct device_node *np,
                                          const char *name, int *lenp) {
    (void)np;
    (void)name;
    if (lenp)
        *lenp = 0;
    return NULL;
}

static inline struct device_node *of_find_node_by_phandle(phandle handle) {
    (void)handle;
    return NULL;
}

static inline struct property *of_find_property(const struct device_node *np,
                                                const char *name, int *lenp) {
    (void)np;
    (void)name;
    if (lenp)
        *lenp = 0;
    return NULL;
}

static inline const char *of_prop_next_string(const struct property *prop,
                                              const char *cur) {
    (void)prop;
    (void)cur;
    return NULL;
}

static inline int of_get_mac_address(const struct device_node *np, u8 *addr) {
    (void)np;
    if (addr)
        eth_random_addr(addr);
    return addr ? 0 : -EINVAL;
}

static inline struct nvmem_cell *of_nvmem_cell_get(const struct device_node *np,
                                                   const char *id) {
    (void)np;
    (void)id;
    return NULL;
}

static inline void wiphy_read_of_freq_limits(struct wiphy *wiphy) {
    (void)wiphy;
}

#define for_each_child_of_node(parent, child)                                  \
    for ((child) = NULL; (child) != NULL; (child) = NULL)
