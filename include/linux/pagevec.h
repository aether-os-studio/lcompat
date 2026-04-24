#pragma once

#include <linux/mm.h>

struct pagevec {
    unsigned int nr;
    struct page *pages[16];
};

static inline void pagevec_init(struct pagevec *pvec) {
    if (pvec)
        pvec->nr = 0;
}
