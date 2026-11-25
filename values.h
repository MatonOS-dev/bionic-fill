/* SPDX-License-Identifier: MIT
 * Legacy glibc <values.h> subset needed by Xwayland's futex dependency. */
#ifndef BIONIC_FILL_VALUES_H
#define BIONIC_FILL_VALUES_H

#if defined(__has_include_next)
# if __has_include_next(<values.h>)
#  include_next <values.h>
# endif
#endif
#include <limits.h>
#ifndef MAXINT
#define MAXINT INT_MAX
#endif

#endif /* BIONIC_FILL_VALUES_H */
