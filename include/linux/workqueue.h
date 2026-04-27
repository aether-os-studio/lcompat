#pragma once

#include <linux/kernel.h>
#include <linux/types.h>

struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);

struct work_struct {
    work_func_t func;
    unsigned int state;
};

struct delayed_work {
    struct work_struct work;
    unsigned int generation;
    bool active;
    bool running;
    unsigned long expires;
};

struct workqueue_struct {
    const char *name;
    unsigned int flags;
    int max_active;
};

#define WQ_UNBOUND BIT(0)
#define WQ_HIGHPRI BIT(1)
#define WQ_BH BIT(2)
#define WQ_PERCPU BIT(3)

void lcompat_init_work(struct work_struct *work, work_func_t func);
void lcompat_init_delayed_work(struct delayed_work *work, work_func_t func);
struct workqueue_struct *
lcompat_alloc_workqueue(const char *name, unsigned int flags, int max_active);
void lcompat_destroy_workqueue(struct workqueue_struct *wq);
bool lcompat_queue_work(struct workqueue_struct *wq, struct work_struct *work);
bool lcompat_queue_delayed_work(struct workqueue_struct *wq,
                                struct delayed_work *work, unsigned long delay);
void lcompat_flush_workqueue(struct workqueue_struct *wq);
bool lcompat_cancel_delayed_work_sync(struct delayed_work *work);

static struct workqueue_struct lcompat_system_wq = {
    .name = "system_wq",
    .flags = 0,
    .max_active = 1,
};

static struct workqueue_struct lcompat_system_highpri_wq = {
    .name = "system_highpri_wq",
    .flags = WQ_HIGHPRI,
    .max_active = 1,
};

#define system_wq (&lcompat_system_wq)
#define system_highpri_wq (&lcompat_system_highpri_wq)

#define INIT_WORK(work, func) lcompat_init_work((work), (func))
#define INIT_DELAYED_WORK(work, func) lcompat_init_delayed_work((work), (func))

#define alloc_workqueue(name, flags, max_active)                               \
    lcompat_alloc_workqueue((name), (flags), (max_active))
#define alloc_ordered_workqueue(name, flags)                                   \
    lcompat_alloc_workqueue((name), (flags), 1)
#define destroy_workqueue(wq) lcompat_destroy_workqueue((wq))
#define queue_work(wq, work) lcompat_queue_work((wq), (work))
#define queue_delayed_work(wq, work, delay)                                    \
    lcompat_queue_delayed_work((wq), (work), (delay))
#define mod_delayed_work(wq, work, delay)                                      \
    lcompat_queue_delayed_work((wq), (work), (delay))
#define flush_workqueue(wq) lcompat_flush_workqueue((wq))
#define flush_delayed_work(work) ((void)(work))
#define cancel_delayed_work_sync(work) lcompat_cancel_delayed_work_sync((work))
#define cancel_delayed_work(work) lcompat_cancel_delayed_work_sync((work))
#define cancel_work_sync(work) ((void)(work), true)
#define cancel_work(work) ((void)(work), true)
#define schedule_work(work) lcompat_queue_work(system_wq, (work))
#define create_singlethread_workqueue(name)                                    \
    lcompat_alloc_workqueue((name), 0, 1)
