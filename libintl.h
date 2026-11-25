/* bionic-fill: untranslated libintl fallback. SPDX-License-Identifier: MIT */
#ifndef BIONIC_FILL_LIBINTL_H
#define BIONIC_FILL_LIBINTL_H
#if __has_include_next(<libintl.h>)
#include_next <libintl.h>
#else
#include <errno.h>
#include <stddef.h>
static inline char *gettext(const char *message) { return (char *)message; }
static inline char *dgettext(const char *domain, const char *message) {
  (void)domain; return gettext(message);
}
static inline char *dcgettext(const char *domain, const char *message, int category) {
  (void)category; return dgettext(domain, message);
}
static inline char *ngettext(const char *one, const char *many, unsigned long n) {
  return (char *)(n == 1 ? one : many);
}
static inline char *dngettext(const char *domain, const char *one,
                              const char *many, unsigned long n) {
  (void)domain; return ngettext(one, many, n);
}
static inline char *dcngettext(const char *domain, const char *one,
                               const char *many, unsigned long n, int category) {
  (void)category; return dngettext(domain, one, many, n);
}
static inline char *bindtextdomain(const char *domain, const char *directory) {
  if (!domain || !*domain) { errno = EINVAL; return NULL; }
  return (char *)(directory ? directory : "/usr/share/locale");
}
static inline char *bind_textdomain_codeset(const char *domain, const char *codeset) {
  if (!domain || !*domain) { errno = EINVAL; return NULL; }
  return (char *)codeset;
}
static inline char *textdomain(const char *domain) {
  return (char *)(domain && *domain ? domain : "messages");
}
#endif
#endif
