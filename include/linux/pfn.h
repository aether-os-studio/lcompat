#pragma once

#include <linux/types.h>

#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif

#define PFN_UP(x) (((x) + ((1UL << PAGE_SHIFT) - 1)) >> PAGE_SHIFT)
#define PFN_DOWN(x) ((x) >> PAGE_SHIFT)
#define PFN_PHYS(x) ((phys_addr_t)(x) << PAGE_SHIFT)
#define PHYS_PFN(x) ((unsigned long)((phys_addr_t)(x) >> PAGE_SHIFT))
