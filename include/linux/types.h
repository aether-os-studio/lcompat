#pragma once

#include <linux/compiler.h>
#include <libs/klibc.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uintptr_t dma_addr_t;
typedef uintptr_t phys_addr_t;
typedef uintptr_t resource_size_t;
typedef unsigned long gfp_t;
typedef unsigned long kernel_ulong_t;
typedef uint16_t umode_t;

typedef u8 __u8;
typedef u16 __u16;
typedef u32 __u32;
typedef u64 __u64;
typedef s8 __s8;
typedef s16 __s16;
typedef s32 __s32;
typedef s64 __s64;
typedef size_t __kernel_size_t;
typedef s16 __bitwise __le16;
typedef s32 __bitwise __le32;
typedef s64 __bitwise __le64;
typedef u16 __bitwise __be16;
typedef u32 __bitwise __be32;
typedef u64 __bitwise __be64;

typedef u64 sector_t;
