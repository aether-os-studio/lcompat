#pragma once

#include <linux/types.h>

typedef s64 loff_t;

struct file;
struct inode;

struct seq_file {
    void *private;
};

static inline int seq_printf(struct seq_file *m, const char *fmt, ...) {
    (void)m;
    (void)fmt;
    return 0;
}

static inline int seq_puts(struct seq_file *m, const char *s) {
    (void)m;
    (void)s;
    return 0;
}

static inline int single_open(struct file *file,
                              int (*show)(struct seq_file *, void *),
                              void *data) {
    (void)file;
    (void)show;
    (void)data;
    return 0;
}

static inline int single_release(struct inode *inode, struct file *file) {
    (void)inode;
    (void)file;
    return 0;
}

static inline ssize_t seq_read(struct file *file, char __user *buf, size_t size,
                               loff_t *ppos) {
    (void)file;
    (void)buf;
    (void)size;
    (void)ppos;
    return 0;
}

static inline loff_t seq_lseek(struct file *file, loff_t offset, int whence) {
    (void)file;
    (void)offset;
    (void)whence;
    return 0;
}
