#pragma once

#include <linux/types.h>

struct netlink_ext_ack {
    const char *msg;
};

struct netlink_callback {
    int dummy;
};

struct nlattr {
    u16 nla_len;
    u16 nla_type;
};

struct nla_policy {
    u8 type;
};
