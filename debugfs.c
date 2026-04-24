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

void debugfs_remove(struct dentry *dentry) {
    if (!dentry)
        return;
    free((void *)dentry->name);
    kfree(dentry);
}

void debugfs_remove_recursive(struct dentry *dentry) { debugfs_remove(dentry); }
