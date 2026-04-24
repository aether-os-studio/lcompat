#pragma once

struct irq_work;
typedef void (*irq_work_func_t)(struct irq_work *work);

struct irq_work {
    irq_work_func_t func;
};

static inline void init_irq_work(struct irq_work *work, irq_work_func_t func) {
    if (work)
        work->func = func;
}

static inline bool irq_work_queue(struct irq_work *work) {
    if (!work || !work->func)
        return false;
    work->func(work);
    return true;
}
