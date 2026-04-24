#pragma once

#include <linux/kernel.h>

static inline int __bf_shf(unsigned long mask) {
    return mask ? __builtin_ctzl(mask) : 0;
}

#define FIELD_MAX(mask) ((mask) >> __bf_shf(mask))
#define FIELD_FIT(mask, val) (((val) & ~FIELD_MAX(mask)) == 0)
#define FIELD_PREP(mask, val) (((typeof(mask))(val) << __bf_shf(mask)) & (mask))
#define FIELD_GET(mask, reg) (((reg) & (mask)) >> __bf_shf(mask))

#define u32_get_bits(reg, mask) FIELD_GET(mask, reg)
#define le32_get_bits(reg, mask) FIELD_GET(mask, le32_to_cpu(reg))
#define le64_get_bits(reg, mask) FIELD_GET(mask, le64_to_cpu(reg))
#define u8_get_bits(reg, mask) FIELD_GET(mask, reg)
#define u16_encode_bits(val, mask) FIELD_PREP(mask, val)
#define u8_encode_bits(val, mask) FIELD_PREP(mask, val)
#define u32_encode_bits(val, mask) FIELD_PREP(mask, val)
#define u64_encode_bits(val, mask) FIELD_PREP(mask, val)
#define le32_encode_bits(val, mask) cpu_to_le32(FIELD_PREP(mask, val))

static inline void u32p_replace_bits(u32 *ptr, u32 val, u32 mask) {
    if (ptr)
        *ptr = (*ptr & ~mask) | FIELD_PREP(mask, val);
}

static inline void u8p_replace_bits(u8 *ptr, u8 val, u8 mask) {
    if (ptr)
        *ptr = (u8)((*ptr & ~mask) | FIELD_PREP(mask, val));
}

static inline void le32p_replace_bits(__le32 *ptr, u32 val, u32 mask) {
    u32 tmp;

    if (!ptr)
        return;
    tmp = le32_to_cpu(*ptr);
    tmp = (tmp & ~mask) | FIELD_PREP(mask, val);
    *ptr = cpu_to_le32(tmp);
}
