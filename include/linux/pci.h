#pragma once

#include <linux/compiler.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/types.h>

#define IORESOURCE_IO 0x00000100UL
#define IORESOURCE_MEM 0x00000200UL
#define IORESOURCE_PREFETCH 0x00002000UL
#define IORESOURCE_UNSET 0x20000000UL

#define PCI_ANY_ID (~0U)

#define PCI_SLOT(devfn) (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn) ((devfn) & 0x07)
#define PCI_DEVFN(slot, func) ((((slot) & 0x1f) << 3) | ((func) & 0x07))

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06
#define PCI_REVISION_ID 0x08

typedef int pci_power_t;

struct resource {
    resource_size_t start;
    resource_size_t end;
    unsigned long flags;
    const char *name;
};

struct pci_bus {
    u16 domain_nr;
    u8 number;
};

struct pci_dev {
    struct device dev;
    void *native;
    struct pci_bus *bus;
    u16 vendor;
    u16 device;
    u16 subsystem_vendor;
    u16 subsystem_device;
    u8 revision;
    u32 class;
    u8 devfn;
    u8 irq;
    u8 msi_cap;
    bool msi_enabled;
    pci_power_t current_state;
    struct resource resource[6];
    void *driver_data;
};

struct pci_device_id {
    u32 vendor;
    u32 device;
    u32 subvendor;
    u32 subdevice;
    u32 class;
    u32 class_mask;
    kernel_ulong_t driver_data;
};

struct pci_driver {
    const char *name;
    const struct pci_device_id *id_table;
    int (*probe)(struct pci_dev *pdev, const struct pci_device_id *id);
    void (*remove)(struct pci_dev *pdev);
    void (*shutdown)(struct pci_dev *pdev);
    struct device_driver driver;
};

#define PCI_DEVICE(vend, dev)                                                  \
    {.vendor = (vend),                                                         \
     .device = (dev),                                                          \
     .subvendor = PCI_ANY_ID,                                                  \
     .subdevice = PCI_ANY_ID}

#define PCI_DEVICE_SUB(vend, dev, subvend, subdev)                             \
    {.vendor = (vend),                                                         \
     .device = (dev),                                                          \
     .subvendor = (subvend),                                                   \
     .subdevice = (subdev)}

#define PCI_VDEVICE(vendor, dev) PCI_DEVICE(PCI_VENDOR_ID_##vendor, (dev))

int pci_register_driver(struct pci_driver *driver);
void pci_unregister_driver(struct pci_driver *driver);
const struct pci_device_id *pci_match_id(const struct pci_device_id *ids,
                                         struct pci_dev *pdev);

struct pci_dev *pci_get_class(unsigned int class_code, struct pci_dev *from);
struct pci_dev *pci_get_domain_bus_and_slot(int domain, unsigned int bus,
                                            unsigned int devfn);
bool pci_dev_present(const struct pci_device_id *ids);
void pci_dev_put(struct pci_dev *pdev);

static inline void *pci_get_drvdata(struct pci_dev *pdev) {
    return pdev ? pdev->driver_data : NULL;
}

static inline void pci_set_drvdata(struct pci_dev *pdev, void *data) {
    if (!pdev)
        return;
    pdev->driver_data = data;
    dev_set_drvdata(&pdev->dev, data);
}

static inline unsigned int pci_domain_nr(struct pci_bus *bus) {
    return bus ? bus->domain_nr : 0;
}

static inline resource_size_t pci_resource_start(struct pci_dev *pdev,
                                                 int bar) {
    return pdev ? pdev->resource[bar].start : 0;
}

static inline resource_size_t pci_resource_end(struct pci_dev *pdev, int bar) {
    return pdev ? pdev->resource[bar].end : 0;
}

static inline resource_size_t pci_resource_len(struct pci_dev *pdev, int bar) {
    if (!pdev || pdev->resource[bar].end < pdev->resource[bar].start)
        return 0;
    return pdev->resource[bar].end - pdev->resource[bar].start + 1;
}

static inline unsigned long pci_resource_flags(struct pci_dev *pdev, int bar) {
    return pdev ? pdev->resource[bar].flags : 0;
}

int pci_enable_device(struct pci_dev *pdev);
void pci_disable_device(struct pci_dev *pdev);
void pci_set_master(struct pci_dev *pdev);
int pci_enable_msi(struct pci_dev *pdev);
void pci_disable_msi(struct pci_dev *pdev);
int pci_save_state(struct pci_dev *pdev);
void pci_restore_state(struct pci_dev *pdev);
int pci_set_power_state(struct pci_dev *pdev, pci_power_t state);
void pci_d3cold_disable(struct pci_dev *pdev);
void pci_d3cold_enable(struct pci_dev *pdev);
void __iomem *pci_iomap_range(struct pci_dev *pdev, int bar,
                              unsigned long offset, unsigned long maxlen);
void pci_iounmap(struct pci_dev *pdev, void __iomem *addr);

int pci_read_config_byte(struct pci_dev *pdev, int where, u8 *val);
int pci_read_config_word(struct pci_dev *pdev, int where, u16 *val);
int pci_read_config_dword(struct pci_dev *pdev, int where, u32 *val);
int pci_write_config_byte(struct pci_dev *pdev, int where, u8 val);
int pci_write_config_word(struct pci_dev *pdev, int where, u16 val);
int pci_write_config_dword(struct pci_dev *pdev, int where, u32 val);
