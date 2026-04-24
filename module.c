#include <lcompat/module_runtime.h>

int lcompat_module_bootstrap(const lcompat_initcall_t *init_begin,
                             const lcompat_initcall_t *init_end,
                             const lcompat_exitcall_t *exit_begin,
                             const lcompat_exitcall_t *exit_end) {
    (void)exit_begin;
    (void)exit_end;

    for (const lcompat_initcall_t *it = init_begin; it < init_end; it++) {
        if (!*it)
            continue;
        int ret = (*it)();
        if (ret)
            return ret;
    }

    return 0;
}

void lcompat_module_shutdown(const lcompat_exitcall_t *exit_begin,
                             const lcompat_exitcall_t *exit_end) {
    for (const lcompat_exitcall_t *it = exit_end; it > exit_begin;) {
        it--;
        if (*it)
            (*it)();
    }
}
