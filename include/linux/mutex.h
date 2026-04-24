#pragma once

#include <libs/mutex.h>

#define __naos_mutex_init mutex_init
#define __naos_mutex_lock mutex_lock
#define __naos_mutex_trylock mutex_trylock
#define __naos_mutex_unlock mutex_unlock

struct mutex {
    mutex_t native;
};

static inline void lcompat_mutex_init(struct mutex *lock) {
    __naos_mutex_init(&lock->native);
}
static inline void lcompat_mutex_lock(struct mutex *lock) {
    __naos_mutex_lock(&lock->native);
}
static inline bool lcompat_mutex_trylock(struct mutex *lock) {
    return __naos_mutex_trylock(&lock->native);
}
static inline void lcompat_mutex_unlock(struct mutex *lock) {
    __naos_mutex_unlock(&lock->native);
}

#define mutex_init(lock) lcompat_mutex_init(lock)
#define mutex_lock(lock) lcompat_mutex_lock(lock)
#define mutex_trylock(lock) lcompat_mutex_trylock(lock)
#define mutex_unlock(lock) lcompat_mutex_unlock(lock)
#define mutex_destroy(lock) ((void)(lock))
