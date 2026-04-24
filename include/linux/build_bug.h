#pragma once

#define BUILD_BUG_ON(cond) _Static_assert(!(cond), "BUILD_BUG_ON(" #cond ")")
#define BUILD_BUG_ON_INVALID(e) ((void)sizeof((long)(e)))
#define static_assert _Static_assert
