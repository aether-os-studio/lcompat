#pragma once

#include <linux/types.h>

struct dentry {
    const char *name;
    struct dentry *parent;
};

struct debugfs_blob_wrapper {
    void *data;
    unsigned long size;
};

struct file_operations;

struct dentry *debugfs_create_dir(const char *name, struct dentry *parent);
struct dentry *debugfs_create_file(const char *name, umode_t mode,
                                   struct dentry *parent, void *data,
                                   const struct file_operations *fops);
void debugfs_remove(struct dentry *dentry);
void debugfs_remove_recursive(struct dentry *dentry);
