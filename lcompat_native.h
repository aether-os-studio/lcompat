#pragma once

#include <drivers/logger.h>
#include <libs/klibc.h>
#include <mm/mm.h>
#include <task/task.h>

#define pci_driver naos_native_pci_driver
#include <drivers/bus/pci.h>
#undef pci_driver

#undef USB_DEVICE
#undef USB_INTERFACE_INFO
#define usb_driver naos_native_usb_driver
#define usb_device naos_native_usb_device
#define usb_device_id naos_native_usb_device_id
#include <drivers/bus/usb.h>
#undef usb_driver
#undef usb_device
#undef usb_device_id
