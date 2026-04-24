#pragma once

#include <libs/klibc.h>

typedef int (*lcompat_initcall_t)(void);
typedef void (*lcompat_exitcall_t)(void);

int lcompat_module_bootstrap(const lcompat_initcall_t *init_begin,
                             const lcompat_initcall_t *init_end,
                             const lcompat_exitcall_t *exit_begin,
                             const lcompat_exitcall_t *exit_end);

void lcompat_module_shutdown(const lcompat_exitcall_t *exit_begin,
                             const lcompat_exitcall_t *exit_end);
