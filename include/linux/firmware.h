#pragma once

#include <fs/vfs/fcntl.h>
#include <fs/vfs/vfs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>

struct firmware {
    size_t size;
    const u8 *data;
};

static inline int request_firmware(const struct firmware **fw, const char *name,
                                   struct device *device) {
    struct vfs_file *file = NULL;
    struct firmware *image = NULL;
    struct vfs_open_how how = {.flags = O_RDONLY};
    char path[256];
    u8 *data = NULL;
    loff_t pos = 0;
    ssize_t nread;
    size_t size;
    int ret;

    (void)device;
    if (!fw || !name)
        return -EINVAL;

    *fw = NULL;

    if (name[0] == '/')
        ret = snprintf(path, sizeof(path), "%s", name);
    else
        ret = snprintf(path, sizeof(path), "/lib/firmware/%s", name);
    if (ret < 0)
        return ret;
    if ((size_t)ret >= sizeof(path))
        return -ENAMETOOLONG;

    ret = vfs_openat(AT_FDCWD, path, &how, &file);
    if (ret < 0)
        return ret;
    if (!file || !file->f_inode) {
        ret = -ENOENT;
        goto out_close;
    }

    size = (size_t)file->f_inode->i_size;
    image = kzalloc(sizeof(*image), GFP_KERNEL);
    if (!image) {
        ret = -ENOMEM;
        goto out_close;
    }

    data = kmalloc(size ? size : 1, GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        goto out_free_image;
    }

    nread = vfs_read_file(file, data, size, &pos);
    if (nread < 0) {
        ret = (int)nread;
        goto out_free_data;
    }
    if ((size_t)nread != size) {
        ret = -EIO;
        goto out_free_data;
    }

    image->size = size;
    image->data = data;
    *fw = image;
    ret = 0;
    goto out_close;

out_free_data:
    kfree(data);
out_free_image:
    kfree(image);
out_close:
    if (file)
        vfs_close_file(file);
    return ret;
}

static inline int firmware_request_nowarn(const struct firmware **fw,
                                          const char *name,
                                          struct device *device) {
    return request_firmware(fw, name, device);
}

static inline void release_firmware(const struct firmware *fw) {
    if (!fw)
        return;
    kfree((void *)fw->data);
    kfree(fw);
}

typedef void (*firmware_cb_t)(const struct firmware *fw, void *context);

static inline int request_firmware_nowait(struct module *module, bool uevent,
                                          const char *name,
                                          struct device *device, gfp_t gfp,
                                          void *context, firmware_cb_t cont) {
    const struct firmware *fw = NULL;
    int ret;

    (void)module;
    (void)uevent;
    (void)gfp;

    ret = request_firmware(&fw, name, device);

    if (!cont)
        return ret;

    cont(ret ? NULL : fw, context);
    return 0;
}
