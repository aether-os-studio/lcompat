#pragma once

#include <linux/init.h>
#include <lcompat/module_runtime.h>

#define __LCOMPAT_SECTION_INIT "lcompat_modinit"
#define __LCOMPAT_SECTION_EXIT "lcompat_modexit"

#define module_init(fn)                                                        \
    static lcompat_initcall_t __lcompat_modinit_##fn                           \
        __attribute__((used, section(__LCOMPAT_SECTION_INIT))) = (fn)

#define module_exit(fn)                                                        \
    static lcompat_exitcall_t __lcompat_modexit_##fn                           \
        __attribute__((used, section(__LCOMPAT_SECTION_EXIT))) = (fn)

#define MODULE_AUTHOR(...)
#define MODULE_DESCRIPTION(...)
#define MODULE_LICENSE(...)
#define MODULE_FIRMWARE(...)
#define MODULE_DEVICE_TABLE(...)
#define MODULE_IMPORT_NS(...)
#define MODULE_SOFTDEP(...)
#define MODULE_PARM_DESC(...)

#define module_param(...)
#define module_param_named(...)
#define module_param_named_unsafe(...)
#define module_param_string(...)
#define module_param_unsafe(...)

#define THIS_MODULE NULL

#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME "lcompat"
#endif

#define LCOMPAT_MODULE_ENTRY()                                                 \
    extern lcompat_initcall_t __start_lcompat_modinit[];                       \
    extern lcompat_initcall_t __stop_lcompat_modinit[];                        \
    extern lcompat_exitcall_t __start_lcompat_modexit[];                       \
    extern lcompat_exitcall_t __stop_lcompat_modexit[];                        \
    int dlmain(void) {                                                         \
        return lcompat_module_bootstrap(                                       \
            __start_lcompat_modinit, __stop_lcompat_modinit,                   \
            __start_lcompat_modexit, __stop_lcompat_modexit);                  \
    }
