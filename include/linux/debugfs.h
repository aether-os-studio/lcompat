#pragma once

#include <linux/seq_file.h>
#include <linux/types.h>

struct device;

struct dentry {
    const char *name;
    struct dentry *parent;
};

struct debugfs_blob_wrapper {
    void *data;
    unsigned long size;
};

struct file_operations;
struct file {
    void *private_data;
};

struct inode {
    void *i_private;
};

struct file_operations {
    int (*open)(struct inode *inode, struct file *file);
    ssize_t (*read)(struct file *file, char __user *buf, size_t size,
                    loff_t *ppos);
    ssize_t (*write)(struct file *file, const char __user *buf, size_t size,
                     loff_t *ppos);
    loff_t (*llseek)(struct file *file, loff_t offset, int whence);
    int (*release)(struct inode *inode, struct file *file);
};

#define DEFINE_DEBUGFS_ATTRIBUTE(__fops, __get, __set, __fmt)                  \
    static const struct file_operations __fops = {}

struct dentry *debugfs_create_dir(const char *name, struct dentry *parent);
struct dentry *debugfs_create_file(const char *name, umode_t mode,
                                   struct dentry *parent, void *data,
                                   const struct file_operations *fops);
struct dentry *debugfs_create_file_unsafe(const char *name, umode_t mode,
                                          struct dentry *parent, void *data,
                                          const struct file_operations *fops);
struct dentry *debugfs_create_u8(const char *name, umode_t mode,
                                 struct dentry *parent, u8 *value);
struct dentry *debugfs_create_u16(const char *name, umode_t mode,
                                  struct dentry *parent, u16 *value);
struct dentry *debugfs_create_u32(const char *name, umode_t mode,
                                  struct dentry *parent, u32 *value);
struct dentry *debugfs_create_bool(const char *name, umode_t mode,
                                   struct dentry *parent, bool *value);
struct dentry *debugfs_create_blob(const char *name, umode_t mode,
                                   struct dentry *parent,
                                   struct debugfs_blob_wrapper *blob);
struct dentry *debugfs_create_devm_seqfile(struct device *dev, const char *name,
                                           struct dentry *parent,
                                           int (*read_fn)(struct seq_file *s,
                                                          void *data));
void debugfs_remove(struct dentry *dentry);
void debugfs_remove_recursive(struct dentry *dentry);
