#pragma once

#include <linux/device.h>
#include <linux/delay.h>
#include <linux/scatterlist.h>
#include <linux/types.h>

#define USB_DIR_OUT 0
#define USB_DIR_IN 0x80
#define USB_TYPE_VENDOR (0x02 << 5)

#define USB_ENDPOINT_NUMBER_MASK 0x0f
#define USB_ENDPOINT_DIR_MASK 0x80
#define USB_ENDPOINT_XFERTYPE_MASK 0x03
#define USB_ENDPOINT_XFER_CONTROL 0
#define USB_ENDPOINT_XFER_ISOC 1
#define USB_ENDPOINT_XFER_BULK 2
#define USB_ENDPOINT_XFER_INT 3

#define USB_DEVICE_ID_MATCH_VENDOR (1U << 0)
#define USB_DEVICE_ID_MATCH_PRODUCT (1U << 1)
#define USB_DEVICE_ID_MATCH_INT_CLASS (1U << 2)
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS (1U << 3)
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL (1U << 4)
#define USB_DEVICE_ID_MATCH_DEVICE                                             \
    (USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)
#define USB_DEVICE_ID_MATCH_INT_INFO                                           \
    (USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_INT_SUBCLASS |        \
     USB_DEVICE_ID_MATCH_INT_PROTOCOL)

enum usb_device_speed {
    USB_SPEED_UNKNOWN = 0,
    USB_SPEED_LOW = 1,
    USB_SPEED_FULL = 2,
    USB_SPEED_HIGH = 3,
    USB_SPEED_WIRELESS = 4,
    USB_SPEED_SUPER = 5,
};

enum usb_device_state {
    USB_STATE_NOTATTACHED = 0,
    USB_STATE_ATTACHED = 1,
    USB_STATE_CONFIGURED = 2,
};

struct usb_interface_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bInterfaceNumber;
    u8 bAlternateSetting;
    u8 bNumEndpoints;
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
    u8 iInterface;
} __attribute__((packed));

struct usb_endpoint_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bEndpointAddress;
    u8 bmAttributes;
    u16 wMaxPacketSize;
    u8 bInterval;
} __attribute__((packed));

struct usb_host_endpoint {
    struct usb_endpoint_descriptor desc;
};

struct usb_host_interface {
    struct usb_interface_descriptor desc;
    struct usb_host_endpoint *endpoint;
};

struct usb_bus {
    int sg_tablesize;
    bool no_sg_constraint;
};

struct usb_device {
    struct device dev;
    void *native;
    struct usb_bus *bus;
    u16 descriptor_id_vendor;
    u16 descriptor_id_product;
    enum usb_device_speed speed;
    enum usb_device_state state;
};

struct usb_interface {
    struct device dev;
    void *native;
    struct usb_device *usb_dev;
    u8 minor;
    struct usb_host_interface *altsetting;
    struct usb_host_interface *cur_altsetting;
    int num_altsetting;
};

struct usb_device_id {
    u16 match_flags;
    u16 idVendor;
    u16 idProduct;
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
    kernel_ulong_t driver_info;
};

struct usb_driver {
    const char *name;
    const struct usb_device_id *id_table;
    int (*probe)(struct usb_interface *intf, const struct usb_device_id *id);
    void (*disconnect)(struct usb_interface *intf);
    int soft_unbind;
    int disable_hub_initiated_lpm;
};

struct urb;
typedef void (*usb_complete_t)(struct urb *urb);

struct urb {
    struct usb_device *dev;
    unsigned int pipe;
    void *transfer_buffer;
    int transfer_buffer_length;
    int actual_length;
    int status;
    unsigned int transfer_flags;
    unsigned int interval;
    usb_complete_t complete;
    void *context;
    void *hcpriv;
    struct scatterlist *sg;
    int num_sgs;
    volatile int submitted;
    volatile int killed;
    unsigned int refcount;
};

#define URB_ZERO_PACKET BIT(0)

#define USB_DEVICE(vendor, product)                                            \
    .match_flags = USB_DEVICE_ID_MATCH_DEVICE, .idVendor = (vendor),           \
    .idProduct = (product)

#define USB_INTERFACE_INFO(cls, subcls, proto)                                 \
    .match_flags = USB_DEVICE_ID_MATCH_INT_INFO, .bInterfaceClass = (cls),     \
    .bInterfaceSubClass = (subcls), .bInterfaceProtocol = (proto)

#define USB_DEVICE_AND_INTERFACE_INFO(vendor, product, cls, subcls, proto)     \
    .match_flags = USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_INT_INFO,  \
    .idVendor = (vendor), .idProduct = (product), .bInterfaceClass = (cls),    \
    .bInterfaceSubClass = (subcls), .bInterfaceProtocol = (proto)

static inline struct usb_device *
interface_to_usbdev(struct usb_interface *intf) {
    return intf ? intf->usb_dev : NULL;
}

static inline void usb_set_intfdata(struct usb_interface *intf, void *data) {
    if (intf)
        dev_set_drvdata(&intf->dev, data);
}

static inline void *usb_get_intfdata(struct usb_interface *intf) {
    return intf ? dev_get_drvdata(&intf->dev) : NULL;
}

static inline struct usb_interface *to_usb_interface(struct device *dev) {
    return dev ? (struct usb_interface *)dev->native : NULL;
}

static inline u8 usb_endpoint_num(const struct usb_endpoint_descriptor *epd) {
    return epd ? (epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK) : 0;
}

static inline bool
usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd) {
    return epd && !!(epd->bEndpointAddress & USB_DIR_IN);
}

static inline bool
usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd) {
    return epd && !(epd->bEndpointAddress & USB_DIR_IN);
}

static inline bool
usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd) {
    return epd && ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
                   USB_ENDPOINT_XFER_BULK);
}

static inline bool
usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd) {
    return usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_in(epd);
}

static inline bool
usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd) {
    return usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_out(epd);
}

static inline bool
usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd) {
    return epd && ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
                   USB_ENDPOINT_XFER_INT);
}

#define LCOMPAT_USB_PIPE_TYPE_CTRL 0U
#define LCOMPAT_USB_PIPE_TYPE_BULK 1U
#define LCOMPAT_USB_PIPE_TYPE_INT 2U
#define __LCOMPAT_USB_PIPE(ep, dir, type)                                      \
    ((((type) & 0xffU) << 16) | ((ep) & 0xffU) | ((dir) ? USB_DIR_IN : 0))

#define usb_sndctrlpipe(dev, ep)                                               \
    __LCOMPAT_USB_PIPE((ep), 0, LCOMPAT_USB_PIPE_TYPE_CTRL)
#define usb_rcvctrlpipe(dev, ep)                                               \
    __LCOMPAT_USB_PIPE((ep), 1, LCOMPAT_USB_PIPE_TYPE_CTRL)
#define usb_sndbulkpipe(dev, ep)                                               \
    __LCOMPAT_USB_PIPE((ep), 0, LCOMPAT_USB_PIPE_TYPE_BULK)
#define usb_rcvbulkpipe(dev, ep)                                               \
    __LCOMPAT_USB_PIPE((ep), 1, LCOMPAT_USB_PIPE_TYPE_BULK)
#define usb_rcvintpipe(dev, ep)                                                \
    __LCOMPAT_USB_PIPE((ep), 1, LCOMPAT_USB_PIPE_TYPE_INT)

int usb_register_driver(struct usb_driver *driver);
void usb_deregister(struct usb_driver *driver);
int usb_control_msg(struct usb_device *dev, unsigned int pipe, u8 request,
                    u8 requesttype, u16 value, u16 index, void *data, u16 size,
                    int timeout_ms);
struct urb *usb_alloc_urb(int iso_packets, gfp_t mem_flags);
void usb_free_urb(struct urb *urb);
void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev,
                       unsigned int pipe, void *transfer_buffer,
                       int buffer_length, usb_complete_t complete_fn,
                       void *context);
int usb_submit_urb(struct urb *urb, gfp_t mem_flags);
void usb_kill_urb(struct urb *urb);
int usb_reset_device(struct usb_device *dev);
int usb_bulk_msg(struct usb_device *usb_dev, unsigned int pipe, void *data,
                 int len, int *actual_length, int timeout);

static inline void usb_init_urb(struct urb *urb) {
    if (urb)
        memset(urb, 0, sizeof(*urb));
}

static inline void usb_poison_urb(struct urb *urb) { usb_kill_urb(urb); }
static inline void usb_unpoison_urb(struct urb *urb) {
    if (urb)
        urb->killed = 0;
}

static inline int usb_maxpacket(struct usb_device *dev, unsigned int pipe) {
    (void)dev;
    (void)pipe;
    return 512;
}

static inline struct usb_device *usb_get_dev(struct usb_device *dev) {
    return dev;
}

static inline void usb_put_dev(struct usb_device *dev) { (void)dev; }

#define module_usb_driver(__usb_driver)                                        \
    static int __lcompat_usb_init_##__usb_driver(void) {                       \
        return usb_register_driver(&(__usb_driver));                           \
    }                                                                          \
    static void __lcompat_usb_exit_##__usb_driver(void) {                      \
        usb_deregister(&(__usb_driver));                                       \
    }                                                                          \
    module_init(__lcompat_usb_init_##__usb_driver);                            \
    module_exit(__lcompat_usb_exit_##__usb_driver)
