#pragma once

#include <linux/kernel.h>

#define overflows_type(n, type) ((n) > (typeof(type))~(typeof(type))0)

#define check_mul_overflow(a, b, d) __builtin_mul_overflow((a), (b), (d))
#define check_add_overflow(a, b, d) __builtin_add_overflow((a), (b), (d))
