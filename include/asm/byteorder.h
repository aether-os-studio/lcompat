#pragma once

#include <linux/types.h>
#include <libs/endian.h>

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __BYTE_ORDER BYTE_ORDER

static inline __le16 cpu_to_le16(u16 v) { return (__le16)htole16(v); }
static inline __le32 cpu_to_le32(u32 v) { return (__le32)htole32(v); }
static inline __le64 cpu_to_le64(u64 v) { return (__le64)htole64(v); }
static inline u16 le16_to_cpu(__le16 v) { return (u16)le16toh((u16)v); }
static inline u32 le32_to_cpu(__le32 v) { return (u32)le32toh((u32)v); }
static inline u64 le64_to_cpu(__le64 v) { return (u64)le64toh((u64)v); }
static inline void le16_add_cpu(__le16 *p, u16 v) {
    if (p)
        *p = cpu_to_le16(le16_to_cpu(*p) + v);
}
static inline u16 __le16_to_cpu(__le16 v) { return le16_to_cpu(v); }
static inline u32 __le32_to_cpu(__le32 v) { return le32_to_cpu(v); }
static inline __be16 cpu_to_be16(u16 v) { return (__be16)htobe16(v); }
static inline __be32 cpu_to_be32(u32 v) { return (__be32)htobe32(v); }
static inline __be64 cpu_to_be64(u64 v) { return (__be64)htobe64(v); }
static inline u16 be16_to_cpu(__be16 v) { return (u16)be16toh((u16)v); }
static inline u32 be32_to_cpu(__be32 v) { return (u32)be32toh((u32)v); }
static inline u64 be64_to_cpu(__be64 v) { return (u64)be64toh((u64)v); }

#define cpu_to_le16(v) ((__le16)(u16)(v))
#define cpu_to_le32(v) ((__le32)(u32)(v))
#define cpu_to_le64(v) ((__le64)(u64)(v))
#define le16_to_cpu(v) ((u16)(v))
#define le32_to_cpu(v) ((u32)(v))
#define le64_to_cpu(v) ((u64)(v))
