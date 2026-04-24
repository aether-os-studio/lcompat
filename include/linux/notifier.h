#pragma once

struct notifier_block {
    int (*notifier_call)(struct notifier_block *nb, unsigned long action,
                         void *data);
};

struct atomic_notifier_head {
    struct notifier_block *head;
};

#define NOTIFY_DONE 0x0000
#define NOTIFY_OK 0x0001
