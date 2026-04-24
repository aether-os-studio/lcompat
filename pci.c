#include <linux/pci.h>
#include "lcompat_native.h"

#define LCOMPAT_MAX_PCI_DRIVERS 64
#define LCOMPAT_MAX_PCI_WRAPPERS PCI_DEVICE_MAX

typedef struct {
    bool used;
    struct pci_driver *linux_driver;
    pci_driver_t native_driver;
} lcompat_pci_binding_t;

typedef struct {
    bool used;
    pci_device_t *native;
    struct pci_dev dev;
    struct pci_bus bus;
    char name[32];
} lcompat_pci_wrapper_t;

static lcompat_pci_binding_t lcompat_pci_bindings[LCOMPAT_MAX_PCI_DRIVERS];
static lcompat_pci_wrapper_t lcompat_pci_wrappers[LCOMPAT_MAX_PCI_WRAPPERS];

static const struct pci_device_id *
lcompat_pci_match_one(const struct pci_device_id *ids, pci_device_t *native) {
    if (!ids || !native)
        return NULL;

    for (const struct pci_device_id *id = ids;
         id->vendor || id->device || id->subvendor || id->subdevice ||
         id->class_mask;
         id++) {
        if (id->vendor != PCI_ANY_ID && id->vendor != native->vendor_id)
            continue;
        if (id->device != PCI_ANY_ID && id->device != native->device_id)
            continue;
        if (id->subvendor != PCI_ANY_ID &&
            id->subvendor != native->subsystem_vendor_id)
            continue;
        if (id->subdevice != PCI_ANY_ID &&
            id->subdevice != native->subsystem_device_id)
            continue;
        if (id->class_mask && (native->class_code & id->class_mask) !=
                                  (id->class & id->class_mask))
            continue;
        return id;
    }

    return NULL;
}

static lcompat_pci_wrapper_t *lcompat_pci_wrap(pci_device_t *native) {
    if (!native)
        return NULL;

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_pci_wrappers); i++) {
        if (lcompat_pci_wrappers[i].used &&
            lcompat_pci_wrappers[i].native == native) {
            return &lcompat_pci_wrappers[i];
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_pci_wrappers); i++) {
        lcompat_pci_wrapper_t *wrapper = &lcompat_pci_wrappers[i];
        if (wrapper->used)
            continue;

        memset(wrapper, 0, sizeof(*wrapper));
        wrapper->used = true;
        wrapper->native = native;
        wrapper->bus.domain_nr = native->segment;
        wrapper->bus.number = native->bus;

        snprintf(wrapper->name, sizeof(wrapper->name), "%04x:%02x:%02x.%u",
                 native->segment, native->bus, native->slot, native->func);

        wrapper->dev.native = native;
        wrapper->dev.bus = &wrapper->bus;
        wrapper->dev.vendor = native->vendor_id;
        wrapper->dev.device = native->device_id;
        wrapper->dev.subsystem_vendor = native->subsystem_vendor_id;
        wrapper->dev.subsystem_device = native->subsystem_device_id;
        wrapper->dev.revision = native->revision_id;
        wrapper->dev.class = native->class_code;
        wrapper->dev.devfn = PCI_DEVFN(native->slot, native->func);
        wrapper->dev.irq = native->irq_line;
        wrapper->dev.dev.kobj_name = wrapper->name;
        wrapper->dev.dev.native = native;
        wrapper->dev.dev.busdev = native->device;

        for (int bar = 0; bar < 6; bar++) {
            wrapper->dev.resource[bar].start = native->bars[bar].address;
            wrapper->dev.resource[bar].end =
                native->bars[bar].size
                    ? native->bars[bar].address + native->bars[bar].size - 1
                    : native->bars[bar].address;
            wrapper->dev.resource[bar].flags =
                native->bars[bar].mmio ? IORESOURCE_MEM : IORESOURCE_IO;
            if (native->bars[bar].prefetchable)
                wrapper->dev.resource[bar].flags |= IORESOURCE_PREFETCH;
            if (!native->bars[bar].size)
                wrapper->dev.resource[bar].flags |= IORESOURCE_UNSET;
        }

        return wrapper;
    }

    return NULL;
}

static bool lcompat_pci_native_match(pci_device_t *dev,
                                     const pci_driver_t *native_driver) {
    lcompat_pci_binding_t *binding =
        native_driver ? native_driver->private_data : NULL;
    return binding && binding->linux_driver &&
           lcompat_pci_match_one(binding->linux_driver->id_table, dev);
}

static int lcompat_pci_native_probe(pci_device_t *dev) {
    pci_driver_t *native_driver = pci_get_current_probe_driver();
    lcompat_pci_binding_t *binding =
        native_driver ? native_driver->private_data : NULL;
    lcompat_pci_wrapper_t *wrapper;
    const struct pci_device_id *id;

    if (!binding || !binding->linux_driver || !binding->linux_driver->probe)
        return -EINVAL;

    wrapper = lcompat_pci_wrap(dev);
    if (!wrapper)
        return -ENOMEM;

    id = lcompat_pci_match_one(binding->linux_driver->id_table, dev);
    if (!id)
        return -ENODEV;

    return binding->linux_driver->probe(&wrapper->dev, id);
}

static void lcompat_pci_native_remove(pci_device_t *dev) {
    lcompat_pci_wrapper_t *wrapper = lcompat_pci_wrap(dev);
    if (!wrapper)
        return;

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_pci_bindings); i++) {
        lcompat_pci_binding_t *binding = &lcompat_pci_bindings[i];
        if (!binding->used || !binding->linux_driver ||
            !binding->linux_driver->remove)
            continue;

        if (!lcompat_pci_match_one(binding->linux_driver->id_table, dev))
            continue;

        binding->linux_driver->remove(&wrapper->dev);
        return;
    }
}

static void lcompat_pci_native_shutdown(pci_device_t *dev) {
    lcompat_pci_wrapper_t *wrapper = lcompat_pci_wrap(dev);
    if (!wrapper)
        return;

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_pci_bindings); i++) {
        lcompat_pci_binding_t *binding = &lcompat_pci_bindings[i];
        if (!binding->used || !binding->linux_driver ||
            !binding->linux_driver->shutdown)
            continue;

        if (!lcompat_pci_match_one(binding->linux_driver->id_table, dev))
            continue;

        binding->linux_driver->shutdown(&wrapper->dev);
        return;
    }
}

int pci_register_driver(struct pci_driver *driver) {
    if (!driver || !driver->probe)
        return -EINVAL;

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_pci_bindings); i++) {
        lcompat_pci_binding_t *binding = &lcompat_pci_bindings[i];
        if (binding->used)
            continue;

        memset(binding, 0, sizeof(*binding));
        binding->used = true;
        binding->linux_driver = driver;
        binding->native_driver.name = driver->name;
        binding->native_driver.class_id = 0;
        binding->native_driver.match = lcompat_pci_native_match;
        binding->native_driver.probe = lcompat_pci_native_probe;
        binding->native_driver.remove = lcompat_pci_native_remove;
        binding->native_driver.shutdown = lcompat_pci_native_shutdown;
        binding->native_driver.private_data = binding;

        int ret = regist_pci_driver(&binding->native_driver);
        if (ret)
            return ret;

        for (size_t dev_idx = 0; dev_idx < pci_device_number; dev_idx++) {
            const struct pci_device_id *id =
                lcompat_pci_match_one(driver->id_table, pci_devices[dev_idx]);
            if (!id)
                continue;
            ret =
                driver->probe(&lcompat_pci_wrap(pci_devices[dev_idx])->dev, id);
            if (ret == 0)
                break;
        }

        return 0;
    }

    return -ENOSPC;
}

void pci_unregister_driver(struct pci_driver *driver) {
    for (size_t i = 0; i < ARRAY_SIZE(lcompat_pci_bindings); i++) {
        if (lcompat_pci_bindings[i].used &&
            lcompat_pci_bindings[i].linux_driver == driver) {
            lcompat_pci_bindings[i].used = false;
            return;
        }
    }
}

const struct pci_device_id *pci_match_id(const struct pci_device_id *ids,
                                         struct pci_dev *pdev) {
    return pdev ? lcompat_pci_match_one(ids, (pci_device_t *)pdev->native)
                : NULL;
}

struct pci_dev *pci_get_class(unsigned int class_code, struct pci_dev *from) {
    size_t start = 0;

    if (from && from->native) {
        for (size_t i = 0; i < pci_device_number; i++) {
            if (pci_devices[i] == (pci_device_t *)from->native) {
                start = i + 1;
                break;
            }
        }
    }

    for (size_t i = start; i < pci_device_number; i++) {
        if ((pci_devices[i]->class_code & 0xffffff00U) ==
            (class_code & 0xffffff00U))
            return &lcompat_pci_wrap(pci_devices[i])->dev;
    }

    return NULL;
}

struct pci_dev *pci_get_domain_bus_and_slot(int domain, unsigned int bus,
                                            unsigned int devfn) {
    pci_device_t *native =
        pci_find_bdfs(bus, PCI_SLOT(devfn), PCI_FUNC(devfn), domain);
    lcompat_pci_wrapper_t *wrapper = lcompat_pci_wrap(native);
    return wrapper ? &wrapper->dev : NULL;
}

bool pci_dev_present(const struct pci_device_id *ids) {
    for (size_t i = 0; i < pci_device_number; i++) {
        if (lcompat_pci_match_one(ids, pci_devices[i]))
            return true;
    }
    return false;
}

void pci_dev_put(struct pci_dev *pdev) { (void)pdev; }

int pci_enable_device(struct pci_dev *pdev) {
    pci_device_t *native = pdev ? (pci_device_t *)pdev->native : NULL;
    if (!native)
        return -EINVAL;

    u16 cmd = native->op->read16(native->bus, native->slot, native->func,
                                 native->segment, PCI_COMMAND);
    cmd |= BIT(0) | BIT(1) | BIT(2);
    native->op->write16(native->bus, native->slot, native->func,
                        native->segment, PCI_COMMAND, cmd);
    return 0;
}

void pci_disable_device(struct pci_dev *pdev) { (void)pdev; }

void pci_set_master(struct pci_dev *pdev) { (void)pci_enable_device(pdev); }

int pci_enable_msi(struct pci_dev *pdev) {
    if (!pdev)
        return -EINVAL;
    pdev->msi_enabled = false;
    return -EOPNOTSUPP;
}

void pci_disable_msi(struct pci_dev *pdev) {
    if (pdev)
        pdev->msi_enabled = false;
}

int pci_save_state(struct pci_dev *pdev) {
    (void)pdev;
    return 0;
}

void pci_restore_state(struct pci_dev *pdev) { (void)pdev; }

int pci_set_power_state(struct pci_dev *pdev, pci_power_t state) {
    if (!pdev)
        return -EINVAL;
    pdev->current_state = state;
    return 0;
}

void pci_d3cold_disable(struct pci_dev *pdev) { (void)pdev; }
void pci_d3cold_enable(struct pci_dev *pdev) { (void)pdev; }

void *pci_iomap_range(struct pci_dev *pdev, int bar, unsigned long offset,
                      unsigned long maxlen) {
    (void)maxlen;
    if (!pdev || bar < 0 || bar >= 6 || !pci_resource_start(pdev, bar))
        return NULL;
    return phys_to_virt(pci_resource_start(pdev, bar) + offset);
}

void pci_iounmap(struct pci_dev *pdev, void *addr) {
    (void)pdev;
    (void)addr;
}

int pci_read_config_byte(struct pci_dev *pdev, int where, u8 *val) {
    pci_device_t *native = pdev ? (pci_device_t *)pdev->native : NULL;
    if (!native || !val)
        return -EINVAL;
    *val = native->op->read8(native->bus, native->slot, native->func,
                             native->segment, where);
    return 0;
}

int pci_read_config_word(struct pci_dev *pdev, int where, u16 *val) {
    pci_device_t *native = pdev ? (pci_device_t *)pdev->native : NULL;
    if (!native || !val)
        return -EINVAL;
    *val = native->op->read16(native->bus, native->slot, native->func,
                              native->segment, where);
    return 0;
}

int pci_read_config_dword(struct pci_dev *pdev, int where, u32 *val) {
    pci_device_t *native = pdev ? (pci_device_t *)pdev->native : NULL;
    if (!native || !val)
        return -EINVAL;
    *val = native->op->read32(native->bus, native->slot, native->func,
                              native->segment, where);
    return 0;
}

int pci_write_config_byte(struct pci_dev *pdev, int where, u8 val) {
    pci_device_t *native = pdev ? (pci_device_t *)pdev->native : NULL;
    if (!native)
        return -EINVAL;
    native->op->write8(native->bus, native->slot, native->func, native->segment,
                       where, val);
    return 0;
}

int pci_write_config_word(struct pci_dev *pdev, int where, u16 val) {
    pci_device_t *native = pdev ? (pci_device_t *)pdev->native : NULL;
    if (!native)
        return -EINVAL;
    native->op->write16(native->bus, native->slot, native->func,
                        native->segment, where, val);
    return 0;
}

int pci_write_config_dword(struct pci_dev *pdev, int where, u32 val) {
    pci_device_t *native = pdev ? (pci_device_t *)pdev->native : NULL;
    if (!native)
        return -EINVAL;
    native->op->write32(native->bus, native->slot, native->func,
                        native->segment, where, val);
    return 0;
}
