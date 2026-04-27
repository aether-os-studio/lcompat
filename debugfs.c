#include <mm/mm.h>
#include <linux/debugfs.h>
#include <linux/slab.h>

struct dentry *debugfs_create_dir(const char *name, struct dentry *parent) {
    struct dentry *entry = kzalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry)
        return NULL;
    entry->name = name ? strdup(name) : NULL;
    entry->parent = parent;
    return entry;
}

struct dentry *debugfs_create_file(const char *name, umode_t mode,
                                   struct dentry *parent, void *data,
                                   const struct file_operations *fops) {
    (void)mode;
    (void)data;
    (void)fops;
    return debugfs_create_dir(name, parent);
}

struct dentry *debugfs_create_file_unsafe(const char *name, umode_t mode,
                                          struct dentry *parent, void *data,
                                          const struct file_operations *fops) {
    return debugfs_create_file(name, mode, parent, data, fops);
}

struct dentry *debugfs_create_u8(const char *name, umode_t mode,
                                 struct dentry *parent, u8 *value) {
    return debugfs_create_file(name, mode, parent, value, NULL);
}

struct dentry *debugfs_create_u16(const char *name, umode_t mode,
                                  struct dentry *parent, u16 *value) {
    return debugfs_create_file(name, mode, parent, value, NULL);
}

struct dentry *debugfs_create_u32(const char *name, umode_t mode,
                                  struct dentry *parent, u32 *value) {
    return debugfs_create_file(name, mode, parent, value, NULL);
}

struct dentry *debugfs_create_bool(const char *name, umode_t mode,
                                   struct dentry *parent, bool *value) {
    return debugfs_create_file(name, mode, parent, value, NULL);
}

struct dentry *debugfs_create_blob(const char *name, umode_t mode,
                                   struct dentry *parent,
                                   struct debugfs_blob_wrapper *blob) {
    return debugfs_create_file(name, mode, parent, blob, NULL);
}

struct dentry *debugfs_create_devm_seqfile(struct device *dev, const char *name,
                                           struct dentry *parent,
                                           int (*read_fn)(struct seq_file *s,
                                                          void *data)) {
    (void)dev;
    (void)read_fn;
    return debugfs_create_file(name, 0400, parent, NULL, NULL);
}

void debugfs_remove(struct dentry *dentry) {
    if (!dentry)
        return;
    free((void *)dentry->name);
    kfree(dentry);
}

void debugfs_remove_recursive(struct dentry *dentry) { debugfs_remove(dentry); }
