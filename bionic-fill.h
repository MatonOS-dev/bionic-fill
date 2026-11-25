/* bionic-fill: glibc-isms missing from Android bionic. Force-include with
 * -include bionic-fill.h. Each item exists because a compiler demanded it;
 * see README.md. SPDX-License-Identifier: MIT */
#ifndef BIONIC_FILL_H
#define BIONIC_FILL_H
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <sys/stat.h>
#include <endian.h>
/* bionic exposes unshare() only when GNU extensions are requested. */
extern int unshare(int flags);
extern int setns(int fd, int nstype);
extern int syncfs(int fd);
#ifndef strdupa
#define strdupa(s) strcpy((char *)__builtin_alloca(strlen(s) + 1), (s))
#endif
#ifndef IFTODT
#define IFTODT(mode) (((mode) & S_IFMT) >> 12)
#endif
static inline int bionic_strverscmp(const char *a, const char *b) {
  while (*a && *b) {
    if (*a >= '0' && *a <= '9' && *b >= '0' && *b <= '9') {
      const char *ae = a, *be = b;
      while (*ae >= '0' && *ae <= '9') ae++;
      while (*be >= '0' && *be <= '9') be++;
      const char *as = a, *bs = b;
      while (as < ae && *as == '0') as++;
      while (bs < be && *bs == '0') bs++;
      size_t al = (size_t)(ae - as), bl = (size_t)(be - bs);
      if (al != bl) return al < bl ? -1 : 1;
      int r = strncmp(as, bs, al);
      if (r) return r;
      if ((ae - a) != (be - b)) return (ae - a) < (be - b) ? 1 : -1;
      a = ae; b = be;
    } else if (*a != *b) {
      return (unsigned char)*a < (unsigned char)*b ? -1 : 1;
    } else { a++; b++; }
  }
  return (unsigned char)*a - (unsigned char)*b;
}
#ifndef strverscmp
#define strverscmp(a, b) bionic_strverscmp((a), (b))
#endif
#ifndef le64toh
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define le64toh(x) ((uint64_t)(x))
#else
#define le64toh(x) __builtin_bswap64((uint64_t)(x))
#endif
#endif
#ifndef explicit_bzero
static inline void bionic_explicit_bzero(void *ptr, size_t len) {
  volatile unsigned char *p = (volatile unsigned char *)ptr;
  while (len--) *p++ = 0;
}
#define explicit_bzero(p, n) bionic_explicit_bzero((p), (n))
#endif
static inline char *bionic_get_current_dir_name(void) {
  size_t size = 128;
  for (;;) {
    char *buf = malloc(size);
    if (buf == NULL) return NULL;
    if (getcwd(buf, size) != NULL) return buf;
    free(buf);
    if (size > ((size_t)-1) / 2) return NULL;
    size *= 2;
  }
}
#define get_current_dir_name bionic_get_current_dir_name
/* Obsolete BSD names that bionic dropped; glibc still has them. */
#ifndef index
#define index(s, c) strchr((s), (c))
#endif
#ifndef rindex
#define rindex(s, c) strrchr((s), (c))
#endif
/* Define HAVE_* to 1 when a configure probe finds a native implementation.
 * Function declarations are not preprocessor macros: #ifndef alone cannot
 * detect future bionic symbols. */
#ifdef __BIONIC__
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
/* glibc declares these GNU extensions only under _GNU_SOURCE (and bionic's
 * cpu_set_t needs it too), so a plain configure probe must not see them. */
#ifdef _GNU_SOURCE
#if !defined(HAVE_PTHREAD_SETAFFINITY_NP) || !HAVE_PTHREAD_SETAFFINITY_NP
#ifndef pthread_setaffinity_np
static inline int bionic_pthread_setaffinity_np(pthread_t thread, size_t size,
                                               const cpu_set_t *set) {
  int saved = errno;
  int result = sched_setaffinity(pthread_gettid_np(thread), size, set);
  int error = result == -1 ? errno : 0;
  errno = saved;
  return error;
}
#define pthread_setaffinity_np bionic_pthread_setaffinity_np
#endif
#endif
#if !defined(HAVE_PTHREAD_GETAFFINITY_NP) || !HAVE_PTHREAD_GETAFFINITY_NP
#ifndef pthread_getaffinity_np
static inline int bionic_pthread_getaffinity_np(pthread_t thread, size_t size,
                                               cpu_set_t *set) {
  int saved = errno;
  int result = sched_getaffinity(pthread_gettid_np(thread), size, set);
  int error = result == -1 ? errno : 0;
  errno = saved;
  return error;
}
#define pthread_getaffinity_np bionic_pthread_getaffinity_np
#endif
#endif
#if !defined(HAVE_FOPENCOOKIE) || !HAVE_FOPENCOOKIE
#ifndef fopencookie
typedef ssize_t cookie_read_function_t(void *, char *, size_t);
typedef ssize_t cookie_write_function_t(void *, const char *, size_t);
typedef int cookie_seek_function_t(void *, off64_t *, int);
typedef int cookie_close_function_t(void *);
typedef struct {
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
struct bionic_cookie {
  void *cookie;
  cookie_io_functions_t io;
  int append;
};
static inline int bionic_cookie_read(void *ptr, char *buf, int size) {
  struct bionic_cookie *c = ptr;
  return c->io.read ? (int)c->io.read(c->cookie, buf, (size_t)size) : 0;
}
static inline int bionic_cookie_write(void *ptr, const char *buf, int size) {
  struct bionic_cookie *c = ptr;
  if (c->append && c->io.seek) {
    off64_t offset = 0;
    if (c->io.seek(c->cookie, &offset, SEEK_END) != 0) return -1;
  }
  return c->io.write ? (int)c->io.write(c->cookie, buf, (size_t)size) : size;
}
static inline fpos64_t bionic_cookie_seek(void *ptr, fpos64_t pos, int whence) {
  struct bionic_cookie *c = ptr;
  off64_t offset = pos;
  if (c->io.seek(c->cookie, &offset, whence) != 0) return -1;
  return offset;
}
static inline int bionic_cookie_close(void *ptr) {
  struct bionic_cookie *c = ptr;
  int result = c->io.close ? c->io.close(c->cookie) : 0;
  int saved = errno;
  free(c);
  errno = saved;
  return result;
}
static inline FILE *bionic_fopencookie(void *cookie, const char *mode,
                                      cookie_io_functions_t io) {
  if (!mode || (*mode != 'r' && *mode != 'w' && *mode != 'a')) {
    errno = EINVAL;
    return NULL;
  }
  struct bionic_cookie *c = malloc(sizeof(*c));
  if (!c) return NULL;
  c->cookie = cookie;
  c->io = io;
  c->append = *mode == 'a';
  int update = strchr(mode, '+') != NULL;
  FILE *fp = funopen64(c, (*mode == 'r' || update) ? bionic_cookie_read : NULL,
                      (*mode != 'r' || update) ? bionic_cookie_write : NULL,
                      io.seek ? bionic_cookie_seek : NULL, bionic_cookie_close);
  if (!fp) {
    int saved = errno;
    free(c);
    errno = saved;
  }
  return fp;
}
#define fopencookie bionic_fopencookie
#endif
#endif
#endif /* _GNU_SOURCE */
/* The companion wrapper defers to a real libintl.h when present. */
#if __has_include(<libintl.h>)
#include <libintl.h>
#else
#include "libintl.h"
#endif
#endif
#endif
