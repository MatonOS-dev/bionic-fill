# bionic-fill

Fills the glibc-isms that Android's bionic libc lacks, so upstream Linux
projects (OSTree, bubblewrap, …) build for Android **without source patches**.

One header, force-included from the build flags:

```sh
CFLAGS="-include /path/to/bionic-fill.h"
```

Scope is deliberately tiny: each item exists only because a compiler error
demanded it, and is listed below with the project that needed it. Nothing is
added speculatively. If upstream or bionic gains the symbol, the item is
guarded (`#ifndef`) and becomes a no-op.

| Item | Why | Needed by |
|---|---|---|
| `values.h`: `MAXINT` | Legacy glibc limits header absent in bionic | Xwayland futex helper |
| `strdupa` | GNU extension, absent in bionic | OSTree |
| `IFTODT` | BSD/glibc macro, absent in bionic | OSTree |
| `strverscmp` | glibc extension | OSTree |
| `le64toh` | via `<endian.h>`, guarded fallback | OSTree |
| `explicit_bzero` | absent in bionic | OSTree |
| `unshare`, `setns`, `syncfs` declarations | bionic declares them only under GNU feature macros | OSTree, bubblewrap |
| `get_current_dir_name` | GNU extension | bubblewrap |
| `pthread_setaffinity_np`, `pthread_getaffinity_np` | pthread-to-TID adapters for sched affinity, errno-style result | PipeWire |
| `fopencookie`, `cookie_io_functions_t`, `cookie_read_function_t`, `cookie_write_function_t`, `cookie_seek_function_t`, `cookie_close_function_t` | Cookie streams via bionic funopen64 | PipeWire ALSA diagnostics |
| `gettext`, `dgettext`, `dcgettext`, `ngettext`, `dngettext`, `dcngettext`, `bindtextdomain`, `bind_textdomain_codeset`, `textdomain` | Untranslated fallback when libintl.h is absent | PipeWire, WirePlumber |
| `index`, `rindex` | obsolete BSD names, dropped by bionic | PipeWire RAOP sink |

Used by MatonOS to build its Flatpak APEX (recipes in
[MatonOS_apexs](https://github.com/MatonOS-dev/MatonOS_apexs)).

Licence: MIT, so it can be compiled into projects under any licence.

The new pthread and cookie fills are bionic-only. Set
`HAVE_PTHREAD_SETAFFINITY_NP`, `HAVE_PTHREAD_GETAFFINITY_NP`, or
`HAVE_FOPENCOOKIE` to 1 after a configure probe finds the native API;
preprocessor tests cannot discover function declarations automatically.
The companion `libintl.h` is needed because PipeWire and WirePlumber include
it unconditionally. Add this repository to the include path. It uses
`__has_include_next` to prefer a real libintl header, including on non-bionic hosts.

Cookie streams preserve seek offset updates, close results, NULL-read EOF,
NULL-write discard, read/write mode restrictions, and append seeks when a
seek callback exists. The adapter uses funopen64 (API 24+), whose callbacks
have int byte counts; calls are bounded by the underlying stdio int request.
GNU mode extensions (exclusive creation, close-on-exec, mmap) have no effect
for these descriptor-less cookie streams. As with glibc, callers must supply
valid live pthread handles and callbacks respecting their buffer bounds.

The libintl fallback does no translation and chooses the English singular
only for n == 1. Domain/directory/codeset setters return their argument but do
not retain configuration; queries return default values (codeset NULL).
They are build compatibility stubs, not a locale/catalog implementation.
