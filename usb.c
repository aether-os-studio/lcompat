#include "lcompat_native.h"
#undef USB_DEVICE
#undef USB_INTERFACE_INFO
#include <linux/slab.h>
#include <linux/usb.h>

#define LCOMPAT_MAX_USB_DRIVERS 256
#define LCOMPAT_MAX_USB_ENDPOINTS 16

typedef struct lcompat_usb_urb_state {
    struct urb *urb;
    usb_pipe_t *native_pipe;
    bool owns_pipe;
    int completed_length;
    bool zlp_pending;
    void *queue_owner;
    int queue_ep_index;
    bool queued;
    struct lcompat_usb_urb_state *pending_next;
} lcompat_usb_urb_state_t;

typedef struct {
    bool used;
    struct usb_driver *linux_driver;
    usb_driver_t native_driver;
    usb_device_id_t *native_ids;
} lcompat_usb_binding_t;

typedef struct {
    bool used;
    usb_device_t *native_dev;
    usb_device_interface_t *native_intf;
    struct usb_bus bus;
    struct usb_device dev;
    struct usb_interface intf;
    struct usb_host_interface altsetting[1];
    struct usb_host_endpoint endpoints[LCOMPAT_MAX_USB_ENDPOINTS];
    usb_pipe_t *native_pipes[LCOMPAT_MAX_USB_ENDPOINTS];
    lcompat_usb_urb_state_t *pending_head[LCOMPAT_MAX_USB_ENDPOINTS];
    lcompat_usb_urb_state_t *pending_tail[LCOMPAT_MAX_USB_ENDPOINTS];
    bool pipe_busy[LCOMPAT_MAX_USB_ENDPOINTS];
    spinlock_t pending_lock;
    int endpoint_count;
    char name[32];
} lcompat_usb_wrapper_t;

static lcompat_usb_binding_t lcompat_usb_bindings[LCOMPAT_MAX_USB_DRIVERS];
static lcompat_usb_wrapper_t lcompat_usb_wrappers[MAX_USBDEV_NUM];

static void lcompat_usb_urb_complete(int status, int actual_length,
                                     void *user_data);
static usb_pipe_t *lcompat_usb_get_native_pipe(struct usb_device *dev,
                                               unsigned int pipe,
                                               lcompat_usb_urb_state_t *state,
                                               bool *owns_pipe_out);
usb_endpoint_descriptor_t *
usb_find_endpoint_by_address(usb_device_t *usbdev, uint8_t epaddr,
                             usb_device_interface_t *prefer_iface,
                             usb_super_speed_endpoint_descriptor_t **ss_out);

static enum usb_device_speed lcompat_usb_linux_speed(u8 native_speed) {
    switch (native_speed) {
    case USB_LOWSPEED:
        return USB_SPEED_LOW;
    case USB_FULLSPEED:
        return USB_SPEED_FULL;
    case USB_HIGHSPEED:
        return USB_SPEED_HIGH;
    case USB_SUPERSPEED:
        return USB_SPEED_SUPER;
    default:
        return USB_SPEED_UNKNOWN;
    }
}

static usb_device_t *lcompat_usb_native_dev(const struct usb_device *dev) {
    return dev ? (usb_device_t *)dev->native : NULL;
}

static usb_device_interface_t *
lcompat_usb_native_intf(const struct usb_interface *intf) {
    return intf ? (usb_device_interface_t *)intf->native : NULL;
}

static unsigned int lcompat_usb_pipe_ep(unsigned int pipe) {
    return pipe & 0xffU;
}

static unsigned int lcompat_usb_pipe_type(unsigned int pipe) {
    return (pipe >> 16) & 0xffU;
}

static bool lcompat_usb_pipe_dir_in(unsigned int pipe) {
    return !!(pipe & USB_DIR_IN);
}

static inline void lcompat_usb_sync_out_buffer(const void *data, int size) {
    if (data && size > 0)
        dma_sync_cpu_to_device((void *)data, (size_t)size);
}

static void lcompat_usb_queue_append(lcompat_usb_wrapper_t *wrapper,
                                     int ep_index,
                                     lcompat_usb_urb_state_t *state) {
    if (!wrapper || ep_index < 0 || ep_index >= LCOMPAT_MAX_USB_ENDPOINTS ||
        !state)
        return;

    state->pending_next = NULL;
    state->queued = true;
    if (wrapper->pending_tail[ep_index])
        wrapper->pending_tail[ep_index]->pending_next = state;
    else
        wrapper->pending_head[ep_index] = state;
    wrapper->pending_tail[ep_index] = state;
}

static lcompat_usb_urb_state_t *
lcompat_usb_queue_pop(lcompat_usb_wrapper_t *wrapper, int ep_index) {
    lcompat_usb_urb_state_t *state;

    if (!wrapper || ep_index < 0 || ep_index >= LCOMPAT_MAX_USB_ENDPOINTS)
        return NULL;

    state = wrapper->pending_head[ep_index];
    if (!state)
        return NULL;

    wrapper->pending_head[ep_index] = state->pending_next;
    if (!wrapper->pending_head[ep_index])
        wrapper->pending_tail[ep_index] = NULL;

    state->pending_next = NULL;
    state->queued = false;
    return state;
}

static int lcompat_usb_sync_retlen(int ret, int expected_len) {
    if (ret == EVENT_BABBLE)
        return -EOVERFLOW;
    if (ret == EVENT_TIMEOUT)
        return -ETIMEDOUT;
    if (ret == EVENT_ERROR)
        return -EPROTO;

    if (ret < 0)
        return ret <= -5 ? ret : -EIO;

    /*
     * Native USB core historically returned 0 for success, while Linux-style
     * callers expect the completed transfer length. Accept both conventions.
     */
    if (ret == 0)
        return expected_len;

    return ret;
}

static int lcompat_usb_sync_xfer(struct usb_device *dev, unsigned int pipe,
                                 int dir, const void *cmd, void *data, int size,
                                 int timeout_ms) {
    usb_device_t *native_dev = lcompat_usb_native_dev(dev);
    usb_pipe_t *native_pipe = NULL;
    usb_xfer_t xfer;
    int actual = 0;
    int ret;
    bool owns_pipe = false;

    if (!native_dev)
        return -ENODEV;

    if (lcompat_usb_pipe_type(pipe) == LCOMPAT_USB_PIPE_TYPE_CTRL) {
        native_pipe = native_dev->defpipe;
    } else {
        native_pipe = lcompat_usb_get_native_pipe(dev, pipe, NULL, &owns_pipe);
        if (!native_pipe)
            return -ENOENT;
    }

    memset(&xfer, 0, sizeof(xfer));
    xfer.pipe = native_pipe;
    xfer.dir = dir;
    xfer.cmd = cmd;
    xfer.data = data;
    xfer.datasize = size;
    xfer.timeout_ns =
        timeout_ms >= 0 ? (uint64_t)timeout_ms * 1000000ULL : (uint64_t)-1;
    xfer.actual_length_out = &actual;

    if (!dir)
        lcompat_usb_sync_out_buffer(data, size);

    ret = usb_submit_xfer(&xfer);
    if (owns_pipe)
        usb_free_pipe(native_dev, native_pipe);
    if (ret < 0)
        return lcompat_usb_sync_retlen(ret, size);

    return actual;
}

static int lcompat_usb_urb_status(int status) {
    switch (status) {
    case EVENT_SUCCESS:
    case EVENT_SHORT_PACKET:
        return 0;
    case EVENT_STALL:
        return -EPIPE;
    case EVENT_BABBLE:
        return -EOVERFLOW;
    case EVENT_TIMEOUT:
        return -ETIMEDOUT;
    case EVENT_ERROR:
        return -EPROTO;
    default:
        return status < 0 ? status : -EIO;
    }
}

static int lcompat_usb_submit_native_urb(lcompat_usb_urb_state_t *state,
                                         void *data, int len) {
    struct urb *urb;
    usb_xfer_t xfer;

    if (!state || !state->urb || !state->native_pipe)
        return -EINVAL;

    urb = state->urb;

    memset(&xfer, 0, sizeof(xfer));
    xfer.pipe = state->native_pipe;
    xfer.dir = lcompat_usb_pipe_dir_in(urb->pipe) ? USB_DIR_IN : USB_DIR_OUT;
    xfer.cmd = NULL;
    xfer.data = data;
    xfer.datasize = len;
    xfer.timeout_ns = 0;
    xfer.cb = lcompat_usb_urb_complete;
    xfer.user_data = state;
    xfer.flags = USB_XFER_ASYNC;

    if (!lcompat_usb_pipe_dir_in(urb->pipe))
        lcompat_usb_sync_out_buffer(data, len);

    return usb_submit_xfer(&xfer);
}

static void lcompat_usb_urb_complete(int status, int actual_length,
                                     void *user_data) {
    lcompat_usb_urb_state_t *state = (lcompat_usb_urb_state_t *)user_data;
    lcompat_usb_wrapper_t *wrapper;
    struct urb *urb;
    usb_complete_t complete;
    void *context;
    int mapped_status;

    if (!state || !state->urb)
        return;

    wrapper = (lcompat_usb_wrapper_t *)state->queue_owner;
    urb = state->urb;
    if (!urb->submitted)
        return;

    mapped_status = lcompat_usb_urb_status(status);

    if (state->zlp_pending) {
        state->zlp_pending = false;
        actual_length = state->completed_length;
    } else if (mapped_status == 0 && !lcompat_usb_pipe_dir_in(urb->pipe) &&
               (urb->transfer_flags & URB_ZERO_PACKET) &&
               urb->transfer_buffer_length > 0 && state->native_pipe &&
               state->native_pipe->maxpacket > 0 &&
               actual_length == urb->transfer_buffer_length &&
               (actual_length % state->native_pipe->maxpacket) == 0) {
        int ret;

        state->completed_length = actual_length;
        state->zlp_pending = true;
        ret = lcompat_usb_submit_native_urb(state, NULL, 0);
        if (ret == 0)
            return;

        state->zlp_pending = false;
        mapped_status = lcompat_usb_urb_status(ret);
    }

    if (mapped_status == 0 && lcompat_usb_pipe_dir_in(urb->pipe) &&
        urb->transfer_buffer && actual_length > 0) {
        dma_sync_device_to_cpu(urb->transfer_buffer, actual_length);
    }

    if (!__sync_bool_compare_and_swap(&urb->submitted, 1, 0))
        return;

    urb->status = mapped_status;
    urb->actual_length = actual_length;
    complete = urb->complete;
    context = urb->context;

    if (!urb->killed && complete) {
        urb->context = context;
        complete(urb);
    }

    if (wrapper && state->queue_ep_index >= 0 &&
        state->queue_ep_index < LCOMPAT_MAX_USB_ENDPOINTS) {
        lcompat_usb_urb_state_t *next = NULL;
        int ep_index = state->queue_ep_index;

        spin_lock(&wrapper->pending_lock);
        next = lcompat_usb_queue_pop(wrapper, ep_index);
        if (!next)
            wrapper->pipe_busy[ep_index] = false;
        spin_unlock(&wrapper->pending_lock);

        if (next) {
            int ret = lcompat_usb_submit_native_urb(
                next, next->urb->transfer_buffer,
                next->urb->transfer_buffer_length);

            if (ret < 0) {
                usb_complete_t next_complete = next->urb->complete;
                void *next_context = next->urb->context;

                next->urb->status = ret;
                next->urb->actual_length = 0;
                next->urb->submitted = 0;

                spin_lock(&wrapper->pending_lock);
                wrapper->pipe_busy[ep_index] = false;
                spin_unlock(&wrapper->pending_lock);

                if (!next->urb->killed && next_complete) {
                    next->urb->context = next_context;
                    next_complete(next->urb);
                }

                usb_free_urb(next->urb);
            }
        }
    }

    usb_free_urb(urb);
}

static int lcompat_usb_collect_endpoints(lcompat_usb_wrapper_t *wrapper,
                                         usb_device_interface_t *intf) {
    u8 *ptr;
    u8 *end;

    if (!wrapper || !intf || !intf->iface)
        return 0;

    wrapper->endpoint_count = 0;
    ptr = (u8 *)intf->iface + intf->iface->bLength;
    end = (u8 *)intf->end;

    while (ptr && end && ptr + 2 <= end &&
           wrapper->endpoint_count < LCOMPAT_MAX_USB_ENDPOINTS) {
        u8 len = ptr[0];
        u8 type = ptr[1];

        if (len < 2)
            break;
        if (type == USB_DT_INTERFACE)
            break;
        if (type == USB_DT_ENDPOINT &&
            ptr + sizeof(struct usb_endpoint_descriptor) <= end) {
            memcpy(&wrapper->endpoints[wrapper->endpoint_count].desc, ptr,
                   sizeof(struct usb_endpoint_descriptor));
            wrapper->endpoint_count++;
        }

        ptr += len;
    }

    return wrapper->endpoint_count;
}

static usb_endpoint_descriptor_t *
lcompat_usb_find_ep_in_iface(usb_device_interface_t *iface, unsigned int pipe,
                             usb_super_speed_endpoint_descriptor_t **ss_out) {
    u8 epaddr = (u8)lcompat_usb_pipe_ep(pipe);
    int dir = lcompat_usb_pipe_dir_in(pipe) ? USB_DIR_IN : USB_DIR_OUT;
    int type = lcompat_usb_pipe_type(pipe) == LCOMPAT_USB_PIPE_TYPE_INT
                   ? USB_ENDPOINT_XFER_INT
                   : USB_ENDPOINT_XFER_BULK;
    usb_endpoint_descriptor_t *matched = NULL;
    u8 *ptr;
    u8 *end;

    epaddr = (epaddr & USB_ENDPOINT_NUMBER_MASK) | (dir ? USB_DIR_IN : 0);

    if (!iface)
        return NULL;
    if (ss_out)
        *ss_out = NULL;

    ptr = (u8 *)iface->iface + iface->iface->bLength;
    end = (u8 *)iface->end;

    while (ptr && end && ptr + 2 <= end) {
        u8 len = ptr[0];
        u8 desc_type = ptr[1];

        if (len < 2)
            break;
        if (desc_type == USB_DT_INTERFACE)
            break;

        if (matched) {
            if (desc_type == USB_DT_ENDPOINT_COMPANION && ss_out &&
                ptr + sizeof(usb_super_speed_endpoint_descriptor_t) <= end)
                *ss_out = (usb_super_speed_endpoint_descriptor_t *)ptr;
            break;
        }

        if (desc_type == USB_DT_ENDPOINT &&
            ptr + sizeof(usb_endpoint_descriptor_t) <= end) {
            usb_endpoint_descriptor_t *epdesc = (void *)ptr;

            if (epdesc->bEndpointAddress == epaddr &&
                (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
                matched = epdesc;
        }

        ptr += len;
    }

    return matched;
}

static int
lcompat_usb_find_wrapper_ep_index(const lcompat_usb_wrapper_t *wrapper,
                                  unsigned int pipe) {
    u8 epaddr = (u8)lcompat_usb_pipe_ep(pipe);
    u8 type = lcompat_usb_pipe_type(pipe) == LCOMPAT_USB_PIPE_TYPE_INT
                  ? USB_ENDPOINT_XFER_INT
                  : USB_ENDPOINT_XFER_BULK;

    epaddr = (epaddr & USB_ENDPOINT_NUMBER_MASK) |
             (lcompat_usb_pipe_dir_in(pipe) ? USB_DIR_IN : 0);

    if (!wrapper)
        return -1;

    for (int i = 0; i < wrapper->endpoint_count; i++) {
        const struct usb_endpoint_descriptor *epd = &wrapper->endpoints[i].desc;

        if (epd->bEndpointAddress != epaddr)
            continue;
        if ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != type)
            continue;
        return i;
    }

    return -1;
}

static lcompat_usb_wrapper_t *
lcompat_usb_find_wrapper_for_pipe(struct usb_device *dev, unsigned int pipe,
                                  int *ep_index_out) {
    usb_device_t *native_dev = lcompat_usb_native_dev(dev);

    if (ep_index_out)
        *ep_index_out = -1;
    if (!native_dev)
        return NULL;

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_usb_wrappers); i++) {
        lcompat_usb_wrapper_t *wrapper = &lcompat_usb_wrappers[i];
        int ep_index;

        if (!wrapper->used || wrapper->native_dev != native_dev)
            continue;

        ep_index = lcompat_usb_find_wrapper_ep_index(wrapper, pipe);
        if (ep_index < 0)
            continue;

        if (ep_index_out)
            *ep_index_out = ep_index;
        return wrapper;
    }

    return NULL;
}

static usb_endpoint_descriptor_t *
lcompat_usb_find_native_ep(usb_device_t *native_dev, unsigned int pipe,
                           usb_device_interface_t **iface_out,
                           usb_super_speed_endpoint_descriptor_t **ss_out) {
    if (!native_dev)
        return NULL;

    for (int i = 0; i < native_dev->ifaces_num; i++) {
        usb_device_interface_t *iface = &native_dev->ifaces[i];
        usb_endpoint_descriptor_t *epdesc =
            lcompat_usb_find_ep_in_iface(iface, pipe, ss_out);

        if (!epdesc)
            continue;
        if (iface_out)
            *iface_out = iface;
        return epdesc;
    }

    return NULL;
}

static usb_pipe_t *lcompat_usb_get_native_pipe(struct usb_device *dev,
                                               unsigned int pipe,
                                               lcompat_usb_urb_state_t *state,
                                               bool *owns_pipe_out) {
    usb_device_t *native_dev = lcompat_usb_native_dev(dev);
    lcompat_usb_wrapper_t *wrapper;
    usb_device_interface_t *iface = NULL;
    usb_super_speed_endpoint_descriptor_t *ss_desc = NULL;
    usb_endpoint_descriptor_t *epdesc;
    int ep_index = -1;

    if (!native_dev)
        return NULL;
    if (owns_pipe_out)
        *owns_pipe_out = false;

    if (lcompat_usb_pipe_type(pipe) == LCOMPAT_USB_PIPE_TYPE_CTRL)
        return native_dev->defpipe;

    if (state && state->native_pipe)
        return state->native_pipe;

    wrapper = lcompat_usb_find_wrapper_for_pipe(dev, pipe, &ep_index);
    if (wrapper && ep_index >= 0 && wrapper->native_pipes[ep_index]) {
        if (state) {
            state->native_pipe = wrapper->native_pipes[ep_index];
            state->owns_pipe = false;
            state->queue_owner = wrapper;
            state->queue_ep_index = ep_index;
        }
        return wrapper->native_pipes[ep_index];
    }

    if (wrapper && ep_index >= 0) {
        iface = wrapper->native_intf;
        epdesc = lcompat_usb_find_ep_in_iface(iface, pipe, &ss_desc);
    } else {
        epdesc = lcompat_usb_find_native_ep(native_dev, pipe, &iface, &ss_desc);
    }
    if (!epdesc)
        return NULL;

    usb_pipe_t *native_pipe = usb_alloc_pipe(native_dev, epdesc, ss_desc);
    if (!native_pipe)
        return NULL;

    if (wrapper && ep_index >= 0) {
        wrapper->native_pipes[ep_index] = native_pipe;
        if (state) {
            state->native_pipe = native_pipe;
            state->owns_pipe = false;
            state->queue_owner = wrapper;
            state->queue_ep_index = ep_index;
        }
        return native_pipe;
    }

    if (state) {
        state->native_pipe = native_pipe;
        state->owns_pipe = true;
        state->queue_owner = NULL;
        state->queue_ep_index = -1;
    }
    if (owns_pipe_out)
        *owns_pipe_out = true;

    return native_pipe;
}

static const struct usb_device_id *
lcompat_usb_match_one(const struct usb_device_id *ids,
                      usb_device_interface_t *iface) {
    if (!ids || !iface || !iface->iface)
        return NULL;

    for (const struct usb_device_id *id = ids; id->match_flags; id++) {
        if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
            id->idVendor != iface->usbdev->vendorid)
            continue;
        if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
            id->idProduct != iface->usbdev->productid)
            continue;
        if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
            id->bInterfaceClass != iface->iface->bInterfaceClass)
            continue;
        if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
            id->bInterfaceSubClass != iface->iface->bInterfaceSubClass)
            continue;
        if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
            id->bInterfaceProtocol != iface->iface->bInterfaceProtocol)
            continue;
        return id;
    }

    return NULL;
}

static lcompat_usb_wrapper_t *lcompat_usb_wrap(usb_device_t *dev,
                                               usb_device_interface_t *intf) {
    if (!dev || !intf)
        return NULL;

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_usb_wrappers); i++) {
        if (lcompat_usb_wrappers[i].used &&
            lcompat_usb_wrappers[i].native_dev == dev &&
            lcompat_usb_wrappers[i].native_intf == intf) {
            return &lcompat_usb_wrappers[i];
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_usb_wrappers); i++) {
        lcompat_usb_wrapper_t *wrapper = &lcompat_usb_wrappers[i];
        if (wrapper->used)
            continue;

        memset(wrapper, 0, sizeof(*wrapper));
        wrapper->used = true;
        wrapper->pending_lock = SPIN_INIT;
        wrapper->native_dev = dev;
        wrapper->native_intf = intf;
        snprintf(wrapper->name, sizeof(wrapper->name), "%s:1.%u", dev->topology,
                 intf->iface->bInterfaceNumber);

        wrapper->dev.native = dev;
        wrapper->dev.bus = &wrapper->bus;
        wrapper->dev.descriptor_id_vendor = dev->vendorid;
        wrapper->dev.descriptor_id_product = dev->productid;
        wrapper->dev.speed = lcompat_usb_linux_speed(dev->speed);
        wrapper->dev.state =
            dev->online ? USB_STATE_CONFIGURED : USB_STATE_NOTATTACHED;
        wrapper->bus.sg_tablesize = 0;
        wrapper->bus.no_sg_constraint = false;
        wrapper->dev.dev.kobj_name = dev->topology;
        wrapper->dev.dev.native = dev;
        wrapper->dev.dev.busdev = dev->bus_device;

        wrapper->intf.native = intf;
        wrapper->intf.usb_dev = &wrapper->dev;
        wrapper->intf.dev.kobj_name = wrapper->name;
        wrapper->intf.dev.native = intf;
        wrapper->intf.dev.busdev = intf->bus_device;
        wrapper->intf.altsetting = &wrapper->altsetting[0];
        wrapper->intf.cur_altsetting = &wrapper->altsetting[0];
        wrapper->intf.num_altsetting = 1;
        memcpy(&wrapper->altsetting[0].desc, intf->iface,
               sizeof(wrapper->altsetting[0].desc));
        lcompat_usb_collect_endpoints(wrapper, intf);
        wrapper->altsetting[0].endpoint = wrapper->endpoints;

        return wrapper;
    }

    return NULL;
}

static int lcompat_usb_native_probe(usb_device_t *usbdev,
                                    usb_device_interface_t *iface) {
    usb_driver_t *native_driver = usb_get_current_probe_driver();
    lcompat_usb_binding_t *binding = NULL;
    lcompat_usb_wrapper_t *wrapper;
    const struct usb_device_id *id;

    for (size_t i = 0; native_driver && i < ARRAY_SIZE(lcompat_usb_bindings);
         i++) {
        if (lcompat_usb_bindings[i].used &&
            &lcompat_usb_bindings[i].native_driver == native_driver) {
            binding = &lcompat_usb_bindings[i];
            break;
        }
    }

    if (!binding || !binding->linux_driver || !binding->linux_driver->probe)
        return -EINVAL;

    wrapper = lcompat_usb_wrap(usbdev, iface);
    if (!wrapper)
        return -ENOMEM;

    id = lcompat_usb_match_one(binding->linux_driver->id_table, iface);
    if (!id)
        return -ENODEV;

    return binding->linux_driver->probe(&wrapper->intf, id);
}

static int lcompat_usb_native_remove(usb_device_t *usbdev) {
    for (size_t i = 0; i < ARRAY_SIZE(lcompat_usb_wrappers); i++) {
        lcompat_usb_wrapper_t *wrapper = &lcompat_usb_wrappers[i];
        if (!wrapper->used || wrapper->native_dev != usbdev)
            continue;

        wrapper->dev.state = USB_STATE_NOTATTACHED;

        for (size_t j = 0; j < ARRAY_SIZE(lcompat_usb_bindings); j++) {
            lcompat_usb_binding_t *binding = &lcompat_usb_bindings[j];
            if (!binding->used || !binding->linux_driver ||
                !binding->linux_driver->disconnect)
                continue;
            if (!lcompat_usb_match_one(binding->linux_driver->id_table,
                                       wrapper->native_intf))
                continue;
            binding->linux_driver->disconnect(&wrapper->intf);
            break;
        }

        for (int ep = 0; ep < wrapper->endpoint_count; ep++) {
            if (!wrapper->native_pipes[ep])
                continue;
            usb_free_pipe(wrapper->native_dev, wrapper->native_pipes[ep]);
            wrapper->native_pipes[ep] = NULL;
        }

        wrapper->used = false;
    }

    return 0;
}

int usb_register_driver(struct usb_driver *driver) {
    if (!driver || !driver->probe)
        return -EINVAL;

    for (size_t i = 0; i < ARRAY_SIZE(lcompat_usb_bindings); i++) {
        lcompat_usb_binding_t *binding = &lcompat_usb_bindings[i];
        if (binding->used)
            continue;

        memset(binding, 0, sizeof(*binding));
        binding->used = true;
        binding->linux_driver = driver;
        binding->native_driver.name = driver->name;
        binding->native_driver.priority = 0;
        binding->native_driver.probe = lcompat_usb_native_probe;
        binding->native_driver.remove = lcompat_usb_native_remove;

        size_t count = 0;
        while (driver->id_table && driver->id_table[count].match_flags)
            count++;

        binding->native_ids =
            kzalloc(sizeof(*binding->native_ids) * (count + 1), GFP_KERNEL);
        if (!binding->native_ids) {
            binding->used = false;
            return -ENOMEM;
        }

        for (size_t id_idx = 0; id_idx < count; id_idx++) {
            binding->native_ids[id_idx].match_flags =
                driver->id_table[id_idx].match_flags;
            binding->native_ids[id_idx].idVendor =
                driver->id_table[id_idx].idVendor;
            binding->native_ids[id_idx].idProduct =
                driver->id_table[id_idx].idProduct;
            binding->native_ids[id_idx].bInterfaceClass =
                driver->id_table[id_idx].bInterfaceClass;
            binding->native_ids[id_idx].bInterfaceSubClass =
                driver->id_table[id_idx].bInterfaceSubClass;
            binding->native_ids[id_idx].bInterfaceProtocol =
                driver->id_table[id_idx].bInterfaceProtocol;
        }

        binding->native_driver.id_table = binding->native_ids;
        regist_usb_driver(&binding->native_driver);
        return 0;
    }

    return -ENOSPC;
}

void usb_deregister(struct usb_driver *driver) {
    for (size_t i = 0; i < ARRAY_SIZE(lcompat_usb_bindings); i++) {
        if (lcompat_usb_bindings[i].used &&
            lcompat_usb_bindings[i].linux_driver == driver) {
            unregist_usb_driver(&lcompat_usb_bindings[i].native_driver);
            kfree(lcompat_usb_bindings[i].native_ids);
            lcompat_usb_bindings[i].native_ids = NULL;
            lcompat_usb_bindings[i].used = false;
            return;
        }
    }
}

int usb_control_msg(struct usb_device *dev, unsigned int pipe, u8 request,
                    u8 requesttype, u16 value, u16 index, void *data, u16 size,
                    int timeout_ms) {
    usb_ctrl_request_t req;
    int ret = -ENODEV;
    bool is_in = !!(requesttype & USB_DIR_IN);

    memset(&req, 0, sizeof(req));
    req.bRequestType = requesttype;
    req.bRequest = request;
    req.wValue = value;
    req.wIndex = index;
    req.wLength = size;

    ret = lcompat_usb_sync_xfer(dev, pipe, requesttype & USB_DIR_IN, &req, data,
                                size, timeout_ms);
    if (ret < 0)
        return ret;

    if (is_in && data && ret > 0)
        dma_sync_device_to_cpu(data, ret);

    return ret;
}

struct urb *usb_alloc_urb(int iso_packets, gfp_t mem_flags) {
    struct urb *urb;

    (void)iso_packets;
    urb = kzalloc(sizeof(*urb), mem_flags);
    if (!urb)
        return NULL;
    urb->refcount = 1;
    return urb;
}

void usb_free_urb(struct urb *urb) {
    lcompat_usb_urb_state_t *state;
    unsigned int refs;

    if (!urb)
        return;

    refs = __sync_sub_and_fetch(&urb->refcount, 1);
    if ((int)refs > 0)
        return;
    if ((int)refs < 0)
        return;

    state = (lcompat_usb_urb_state_t *)urb->hcpriv;
    if (state) {
        if (state->owns_pipe && state->native_pipe && urb->dev)
            usb_free_pipe(lcompat_usb_native_dev(urb->dev), state->native_pipe);
        kfree(state);
    }

    kfree(urb);
}

void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev,
                       unsigned int pipe, void *transfer_buffer,
                       int buffer_length, usb_complete_t complete_fn,
                       void *context) {
    if (!urb)
        return;
    urb->dev = dev;
    urb->pipe = pipe;
    urb->transfer_buffer = transfer_buffer;
    urb->transfer_buffer_length = buffer_length;
    urb->complete = complete_fn;
    urb->context = context;
}

int usb_submit_urb(struct urb *urb, gfp_t mem_flags) {
    lcompat_usb_urb_state_t *state;
    usb_pipe_t *native_pipe;

    (void)mem_flags;
    if (!urb || !urb->dev)
        return -EINVAL;
    if (urb->submitted)
        return -EBUSY;

    state = (lcompat_usb_urb_state_t *)urb->hcpriv;
    if (!state) {
        state = kzalloc(sizeof(*state), GFP_KERNEL);
        if (!state)
            return -ENOMEM;
        state->urb = urb;
        urb->hcpriv = state;
    }

    native_pipe = lcompat_usb_get_native_pipe(urb->dev, urb->pipe, state, NULL);
    if (!native_pipe)
        return -ENOENT;

    urb->status = -EINPROGRESS;
    urb->actual_length = 0;
    urb->submitted = 1;
    urb->killed = 0;
    state->completed_length = 0;
    state->zlp_pending = false;
    state->pending_next = NULL;
    state->queued = false;
    __sync_add_and_fetch(&urb->refcount, 1);

    if (state->queue_owner && state->queue_ep_index >= 0 &&
        state->queue_ep_index < LCOMPAT_MAX_USB_ENDPOINTS) {
        lcompat_usb_wrapper_t *wrapper =
            (lcompat_usb_wrapper_t *)state->queue_owner;
        bool should_queue = false;

        spin_lock(&wrapper->pending_lock);
        if (wrapper->pipe_busy[state->queue_ep_index])
            should_queue = true;
        else
            wrapper->pipe_busy[state->queue_ep_index] = true;
        spin_unlock(&wrapper->pending_lock);

        if (should_queue) {
            spin_lock(&wrapper->pending_lock);
            lcompat_usb_queue_append(wrapper, state->queue_ep_index, state);
            spin_unlock(&wrapper->pending_lock);
            return 0;
        }
    }

    int ret = lcompat_usb_submit_native_urb(state, urb->transfer_buffer,
                                            urb->transfer_buffer_length);
    if (ret < 0) {
        if (state->queue_owner && state->queue_ep_index >= 0 &&
            state->queue_ep_index < LCOMPAT_MAX_USB_ENDPOINTS) {
            lcompat_usb_wrapper_t *wrapper =
                (lcompat_usb_wrapper_t *)state->queue_owner;
            spin_lock(&wrapper->pending_lock);
            wrapper->pipe_busy[state->queue_ep_index] = false;
            spin_unlock(&wrapper->pending_lock);
        }
        urb->submitted = 0;
        usb_free_urb(urb);
        return ret;
    }

    return 0;
}

void usb_kill_urb(struct urb *urb) {
    if (!urb)
        return;

    urb->killed = 1;
    while (urb->submitted)
        schedule(SCHED_FLAG_YIELD);
}

int usb_reset_device(struct usb_device *dev) {
    (void)dev;
    return 0;
}
