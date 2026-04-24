#pragma once

#include <linux/types.h>

static inline u8 bcd2bin(u8 val) {
    return ((val & 0xf0) >> 4) * 10 + (val & 0x0f);
}
static inline u8 bin2bcd(u8 val) { return ((val / 10) << 4) | (val % 10); }
