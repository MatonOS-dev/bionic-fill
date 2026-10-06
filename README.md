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
| `strdupa` | GNU extension, absent in bionic | OSTree |
| `IFTODT` | BSD/glibc macro, absent in bionic | OSTree |
| `strverscmp` | glibc extension | OSTree |
| `le64toh` | via `<endian.h>`, guarded fallback | OSTree |
| `explicit_bzero` | absent in bionic | OSTree |
| `unshare`, `setns`, `syncfs` declarations | bionic declares them only under GNU feature macros | OSTree, bubblewrap |
| `get_current_dir_name` | GNU extension | bubblewrap |

Used by MatonOS to build its Flatpak APEX (recipes in
[MatonOS_apexs](https://github.com/MatonOS-dev/MatonOS_apexs)).

Licence: MIT, so it can be compiled into projects under any licence.
