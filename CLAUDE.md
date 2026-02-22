# CLAUDE.md — Motif 2.3.8 Codebase Guide

This file provides guidance for AI assistants working with the OpenMotif 2.3.8
source tree. Motif is a C-language UI toolkit built on X11/Xt that provides the
`libXm` widget library, the MWM window manager, the UIL compiler, and related
tools.

---

## Project Overview

| Item | Value |
|------|-------|
| Project | OpenMotif (open-source Motif) |
| Version | 2.3.8 (released 2017-12-05) |
| License | LGPL v2.1 |
| Language | C (Lex/Yacc for the UIL compiler grammar) |
| Build system | GNU Autotools (autoconf + automake + libtool) |
| X11 dependency | X11, Xt, Xext required; Xmu, Xft, libjpeg, libpng, Xp optional |
| Maintained by | The Open Group / Integrated Computer Solutions |
| Bug tracker | http://bugs.motifzone.net |

---

## Repository Layout

```
motif/
├── configure.ac          # Autoconf input — version, feature detection
├── Makefile.am           # Top-level Automake input; lists SUBDIRS
├── INSTALL.configure     # Human-readable build instructions
├── RELNOTES              # Release notes
├── COPYING               # LGPL v2.1 license
├── BUGREPORT             # Bug-reporting instructions
│
├── lib/
│   ├── Xm/               # Main Motif widget library (libXm) — ~537 C/H files
│   └── Mrm/              # MRM: Motif Resource Manager (reads UID files)
│
├── clients/
│   ├── mwm/              # MWM: Motif Window Manager (C binary)
│   ├── uil/              # UIL compiler (Lex/Yacc-based, produces UID files)
│   └── xmbind/           # Virtual key-binding tool
│
├── include/
│   └── Dt/               # CDE/Desktop headers (Editor, Help, Print, Term, etc.)
│
├── demos/
│   ├── lib/
│   │   ├── Exm/          # Example widget library used by demo programs
│   │   ├── Xmd/          # Xmd utility library for demos
│   │   └── Wsm/          # Workspace Manager demo library
│   └── programs/         # ~30 demo applications (hellomotif, drag_and_drop, etc.)
│       └── Exm/          # Exm widget demo applications (app_in_c, app_in_uil, simple_app)
│
├── tools/
│   └── wml/              # WML (Widget Meta Language) processor that generates
│                         # UIL keyword tables and Xm string definitions
│
├── doc/
│   └── man/
│       ├── man1/         # mwm, uil, xmbind man pages
│       ├── man3/         # Xm library API man pages
│       ├── man4/         # mwmrc config file man pages
│       └── man5/         # UIL file format man pages
│
├── bindings/             # Virtual key-binding files for ~25 hardware platforms
├── bitmaps/              # Built-in cursor/icon bitmaps
├── localized/            # Localization utilities
└── config/               # Autoconf/build utilities
```

### Key source files

| File | Purpose |
|------|---------|
| `configure.ac` | All feature detection and configure-time knobs |
| `lib/Xm/Makefile.am` | Defines which Xm sources are compiled into `libXm.la` |
| `lib/Mrm/Makefile.am` | MRM library build |
| `clients/uil/UilLexAna.c` | UIL lexer |
| `clients/uil/UilLexPars.c` | UIL parser (generated from Yacc) |
| `tools/wml/motif.wml` | Canonical WML source for all Motif widget data |
| `include/config.h.in` | Template for the generated `config.h` |

---

## Build System

Motif uses the **GNU Autotools** stack. The `configure` script is generated
from `configure.ac`; `Makefile` files are generated from `Makefile.am` files.

### Prerequisites

- C compiler (GCC recommended)
- GNU make
- X11 development headers and libraries (`libX11`, `libXt`, `libXext`)
- `lex`/`flex` and `yacc`/`byacc` (for UIL compiler)
- `libtool`, `automake`, `autoconf` (to regenerate the build system)

Optional libraries detected at configure time:
- `libXmu` — EditRes support
- `libXft` — anti-aliased font rendering
- `libjpeg` — JPEG image support
- `libpng` — PNG image support
- `libXp` — printing support

### Build steps

```sh
# If configure is missing, regenerate it first:
./autogen.sh

# Configure (see "Configure Options" below for knobs):
./configure

# Build everything:
make

# Run self-tests (if any exist):
make check

# Install to the configured prefix (default /usr or /usr/X11R6):
make install

# Clean object files:
make clean

# Remove all generated build files (allows reconfiguring):
make distclean

# Remove maintainer-generated files (requires autotools to regenerate):
make maintainer-clean
```

### Out-of-tree builds

Because `configure` uses `VPATH`, you can build out-of-tree:

```sh
mkdir build && cd build
../configure --prefix=/opt/motif
make
```

### Configure options

| Option | Default | Description |
|--------|---------|-------------|
| `--prefix=PATH` | `/usr` or `/usr/X11R6` | Installation root |
| `--enable-message-catalog` | disabled | Build NLS message catalogs |
| `--enable-themes` | disabled | Theme engine (source required) |
| `--disable-motif22-compatibility` | enabled | Binary compat with OpenMotif 2.2 |
| `--disable-utf8` | enabled | UTF-8 string support |
| `--disable-printing` | enabled | XpPrint extension support |
| `--enable-debug-themes` | disabled | Theme debugging output |
| `--x-includes=DIR` | auto | Override X11 include path |
| `--x-libraries=DIR` | auto | Override X11 library path |

Environment variables honored by `configure`:

```sh
CC=gcc CFLAGS="-O2" LDFLAGS="-L/opt/X11/lib" ./configure
```

### GCC compile flags (set automatically)

When GCC is detected, `configure.ac` appends:

```
-Wall -g -fno-strict-aliasing -Wno-unused -Wno-comment
```

GCC 4.x also adds `-fno-tree-ter` to work around a miscompilation.

### Platform notes

| Platform | Automatic adjustments |
|----------|-----------------------|
| FreeBSD | `-DCSRG_BASED -DXNO_MTSAFE_API -DXNO_MTSAFE_PWDAPI`; adds `/usr/local/lib` to `LDFLAGS` |
| Cygwin | `-DXNO_MTSAFE_DIRENTDAPI` |
| Solaris/SunOS | Disables motif22-compat, UTF-8, Xft by default; adds `/usr/X/lib` |

---

## Library Versioning

`libXm` uses libtool versioning set in `configure.ac`:

```
CURRENT=4  REVISION=4  AGE=0   → libXm.so.4.0.4
```

The `LIBTOOL_VERSION` variable (`CURRENT:REVISION:AGE`) is passed to
`libXm_la_LDFLAGS` in `lib/Xm/Makefile.am`.

---

## Components in Detail

### libXm (`lib/Xm/`)

The core Motif widget set. Sources follow the naming convention:

- `Foo.c` / `Foo.h` — public widget implementation and header
- `FooP.h` — private (internal) widget class record header
- `FooI.h` — internal implementation details shared across files
- `FooT.h` / `FooT.c` — trait implementations (e.g., `AccColorT`, `ActivatableT`)
- `*I.h` — internal helper headers not installed

Optional feature groups controlled by `AM_CONDITIONAL` in `configure.ac`:

| Conditional | Source files |
|-------------|-------------|
| `OM_XMU` | `EditresCom.c` |
| `OM_LIBJPEG` | `Jpeg.c` |
| `OM_LIBPNG` | PNG-related sources |
| `OM_XFT` | Xft font rendering |
| `PRINTING` | Print widget sources |
| `OM_XMTHEMES` | Theme engine |
| `OM22_COMPATIBILITY` | Motif 2.2 compatibility shims |

### libMrm (`lib/Mrm/`)

The Motif Resource Manager reads binary UID files produced by the UIL
compiler and instantiates widget hierarchies at runtime. Files are prefixed
`Mrm*` (public API) or `Mrm*I*` (internal).

### MWM (`clients/mwm/`)

The Motif Window Manager. Source files are prefixed `Wm*`. It has its own
`WmWsmLib/` subdirectory for workspace manager IPC.

### UIL Compiler (`clients/uil/`)

The User Interface Language compiler. It uses Lex (`UilLexAna.c`) and Yacc
(`UilLexPars.c`, generated from `UilLexPars.y`) to parse `.uil` source files
and produce `.uid` binary resource files consumed by MRM.

### xmbind (`clients/xmbind/`)

Loads virtual key-binding definitions from the platform-specific files in
`bindings/` into the X server's key-binding properties.

### WML Tool (`tools/wml/`)

The Widget Meta Language processor reads `motif.wml` and generates:
- UIL keyword tables for the UIL compiler
- `XmStrDefs.h` string definitions for libXm

Running the WML tool requires `wml.c` to be compiled and then executed against
`motif.wml`. This is done automatically during the build.

### Demo Library (`demos/lib/Exm/`)

An example of how to implement Motif widgets following current best practices
(traits, superclass architecture). Used by the `demos/programs/Exm/` apps.

---

## Coding Conventions

These conventions reflect the existing codebase and must be followed when
modifying it.

### File naming

- Widget implementation: `WidgetName.c`, `WidgetName.h`, `WidgetNameP.h`
- Internal helpers: `WidgetNameI.h`
- Trait definitions: `TraitNameT.c`, `TraitNameT.h`
- MRM files: `Mrm*.c`, `Mrm*.h`
- MWM files: `Wm*.c`, `Wm*.h`
- UIL files: `Uil*.c`, `Uil*.h`

### C style

- C89/C90 compatible code throughout (ANSI C prototypes are used but keep
  broad portability in mind)
- Avoid C99 features (`//` comments, declarations after statements,
  `<stdbool.h>`, etc.) unless you have verified the minimum compiler
  requirement has been raised
- Use `XtMalloc` / `XtFree` / `XtRealloc` for memory managed by Xt; use
  `XtNew` / `XtNewString` macros where appropriate
- Use `XmStackAlloc` / `XmStackFree` for small stack-eligible buffers
- `#ifdef` guards follow the pattern `#ifndef _Xm_FileName_h` / `#define _Xm_FileName_h`
- Public headers must be safe to include from C++ (`#ifdef __cplusplus` guards)
- Function names for public APIs follow `XmWidgetNameAction` (PascalCase after
  the `Xm` prefix); internal helpers are lowercase with underscores or use a
  leading `_Xm` prefix for quasi-private symbols

### Autotools conventions

- Add new source files to the relevant `Makefile.am` `*_SOURCES` variable —
  never edit generated `Makefile.in` directly
- New configure feature tests belong in `configure.ac`; use
  `AM_CONDITIONAL` to expose them to `Makefile.am` files
- After editing `configure.ac` or any `Makefile.am`, run `autoreconf -fi`
  (or `./autogen.sh`) and commit the regenerated files

### Header installation

Public headers for `libXm` are installed to `$(includedir)/Xm/`. The
`xm_HEADERS` variable in `lib/Xm/Makefile.am` controls what is installed.
Do not add internal `*I.h` or `*P.h` files to `xm_HEADERS` unless
intentionally making them part of the public API.

---

## Regenerating the Build System

If `configure.ac` or any `Makefile.am` changes:

```sh
cd /path/to/motif
autoreconf -fi
# or equivalently:
./autogen.sh
```

This runs `libtoolize`, `aclocal`, `autoconf`, `autoheader`, and `automake`
in the correct order.

---

## Testing

Motif does not have an automated unit-test suite. Verification is done by:

1. **Build success** — `make` must complete without errors.
2. **Demo programs** — Run demo binaries in `demos/programs/` against a live X
   display to exercise widget functionality.
3. **`make check`** — Runs whatever self-tests `Makefile.am` defines (currently
   minimal); always run it after changes.

For display-dependent tests you need a running X server. Set `DISPLAY`
appropriately, e.g.:

```sh
export DISPLAY=:0
./demos/programs/hellomotif/hellomotif
```

---

## Common Tasks for AI Assistants

### Adding a new widget to libXm

1. Create `lib/Xm/MyWidget.c`, `lib/Xm/MyWidget.h`, `lib/Xm/MyWidgetP.h`
   following the existing widget template (e.g., copy from `ArrowB.*`).
2. Add `.c` file to `libXm_la_SOURCES` and `.h` file to `xm_HEADERS` in
   `lib/Xm/Makefile.am`.
3. Export the public class record in the `.h` file; keep the private record in
   `P.h`.
4. Run `make` to verify.

### Modifying the UIL compiler grammar

1. Edit `clients/uil/UilLexPars.y` (Yacc source) or
   `clients/uil/UilLexAna.l` (Lex source).
2. Run `make` — the build regenerates `UilLexPars.c` and `UilLexAna.c`
   automatically via the `ylwrap` script.

### Adding a new configure option

1. Add `AC_ARG_ENABLE` or `AC_ARG_WITH` stanza in `configure.ac`.
2. Conditionally `AC_DEFINE` a preprocessor macro and/or create an
   `AM_CONDITIONAL`.
3. Use the conditional in the relevant `Makefile.am`.
4. Run `autoreconf -fi` to regenerate `configure`.

### Updating man pages

Man pages live under `doc/man/man{1,3,4,5}/`. They are plain troff files.
Add new pages to the `dist_man_MANS` variable in the relevant `doc/man/manN/Makefile.am`.

---

## Environment & Dependencies Quick Reference

```sh
# Debian/Ubuntu — install build dependencies
sudo apt-get install \
  build-essential autoconf automake libtool \
  libx11-dev libxt-dev libxmu-dev libxft-dev \
  libxp-dev libjpeg-dev libpng-dev \
  flex byacc

# Configure with common options
./configure --prefix=/usr/local \
            --enable-message-catalog \
            --disable-printing   # if libXp is unavailable
```

---

## Branch & Commit Conventions

This repository uses a single `master` branch for stable releases. Development
branches follow the pattern `claude/<description>-<session-id>`.

Commit messages should be in the imperative mood and describe **why** the
change was made, e.g.:

```
Fix memory leak in XmList when items are deleted rapidly

The previous code did not call XtFree on the internal item cache when
XmListDeleteItems removed the last item, causing a leak on each clear.
```

---

## File Generation — Do Not Edit Directly

The following files are generated by the build system and must not be edited
by hand:

| Generated file | Regenerated by |
|----------------|----------------|
| `configure` | `autoconf` / `autoreconf -fi` |
| `aclocal.m4` | `aclocal` |
| `*/Makefile.in` | `automake` |
| `include/config.h.in` | `autoheader` |
| `ltmain.sh`, `libtool` | `libtoolize` |
| `config.guess`, `config.sub` | `automake --add-missing` |
| `compile`, `install-sh`, `missing`, `depcomp` | `automake --add-missing` |
| `lib/Xm/xmstring.list` | `tools/wml/wml motif.wml` |
| `clients/uil/UilLexPars.c` | `ylwrap` wrapping `yacc` |
| `clients/uil/UilLexAna.c` | `ylwrap` wrapping `lex` |
