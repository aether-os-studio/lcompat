#pragma once

struct lock_class_key {
    int dummy;
};

#ifndef LOCKDEP_STILL_OK
#define LOCKDEP_STILL_OK 0
#endif

#define lockdep_assert_held(lock) ((void)(lock))
