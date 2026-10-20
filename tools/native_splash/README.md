# CLAW Native Terminal Splash

A native, dependency-free C11 terminal renderer based on the high-resolution art map from the supplied `files.zip`. It recreates the creature and cyan `CLAW` lettering using actual ASCII characters and adapts the artwork to the available terminal size.

## What this version does

- **Native runtime:** no Python, Pillow, Kitty, Tkinter, curses, image viewer, or third-party runtime package.
- **Cross-platform source:** Linux, macOS, BSD-like POSIX terminals, Windows Console, and Windows Terminal.
- **Resolution-aware art:** embeds the high-resolution luminance and blue-color map from the supplied ZIP rather than converting it to a low-resolution fixed block of characters.
- **Automatic fit:** scales to the current terminal dimensions and keeps the image's proportions, accounting for the taller shape of terminal character cells.
- **Live resize:** interactive mode redraws when terminal dimensions change.
- **Color fallback:** truecolor, 256-color, 16-color, or no-color modes; `NO_COLOR` and `TERM=dumb` are respected.
- **Safe output mode:** if output is redirected or the terminal is non-interactive, it draws once and exits.

Terminal fonts and character-cell dimensions differ, so no terminal implementation can guarantee pixel-identical output on every terminal. This renderer preserves the artwork map and its broad detail/color distribution; actual presentation depends on font and terminal size.

## Quick start on Linux / WSL

A statically linked Linux x86-64 executable is included at `bin/claw-splash-linux-x86_64`; it has no dynamic shared-library dependencies:

```sh
chmod +x bin/claw-splash-linux-x86_64
./bin/claw-splash-linux-x86_64
```

This prebuilt file is only for compatible 64-bit Linux systems (including WSL). macOS and Windows require their own native executables.

## Build and run

### Linux and macOS

Open a terminal in this project folder:

```sh
make
./claw-splash
```

For one frame:

```sh
./claw-splash --once
```

Keep it running and redraw on resize (the default for an interactive terminal):

```sh
./claw-splash --watch
```

Press `q`, `Esc`, `Enter`, or `Ctrl+C` to exit live mode.

Without Make, build directly:

```sh
cc -O2 -std=c11 -Wall -Wextra -Wpedantic -Iinclude \
  -o claw-splash src/main.c src/terminal.c src/renderer.c
```

### Windows

Run `build.bat` from a Developer Command Prompt for Visual Studio (MSVC) or a terminal where MinGW-w64 GCC is available:

```bat
build.bat
claw-splash.exe
```

The build script selects MSVC (`cl`) or MinGW (`gcc`). Windows Terminal or a recent ANSI-capable Windows Console is recommended for the colored live presentation. If terminal ANSI support is unavailable, the program falls back to plain output when possible.

### Options

```text
--once             Draw once, then exit
--watch            Stay open and redraw when the terminal is resized
--cols N           Override width (also enables one-frame mode)
--rows N           Override height (also enables one-frame mode)
--aspect X         Character-cell height/width ratio (default: detect, else 2.0)
--color MODE       auto, truecolor, 256, 16, or none
--help             Show options
```

Examples:

```sh
./claw-splash --once --cols 160 --rows 70 --color truecolor
./claw-splash --once --cols 120 --rows 45 --color none > claw.txt
NO_COLOR=1 ./claw-splash --once
```

`--cols` and `--rows` set the maximum available drawing area. The renderer may use fewer rows or columns to keep the source image's aspect ratio.

## Project structure

| File | Purpose |
|---|---|
| `src/main.c` | Options, one-frame/live modes, resize loop and clean shutdown |
| `src/terminal.c` | Platform-specific terminal sizing, key input, colors and raw-mode handling |
| `src/renderer.c` | Embedded-data decoder, adaptive ASCII renderer, scaling and color mapping |
| `include/claw_art_data.h` | High-resolution artwork, encoded into portable text data |
| `include/renderer.h` | Renderer interface |
| `include/terminal.h` | Terminal interface shared across platforms |
| `Makefile` | Linux/macOS build helper |
| `build.bat` | Windows build helper for MSVC or MinGW-w64 |
| `bin/claw-splash-linux-x86_64` | Prebuilt, statically linked Linux x86-64 executable |
| `tests/smoke.sh` | Small build/output sanity test for Linux and macOS |
| `.github/workflows/build-splash.yml` | CI workflow to generate native Linux, macOS and Windows artifacts |

## Integration into CLAW

Keep this module separate from editor and OS core code. Build it as a standalone splash executable or link its renderer/terminal modules into the startup layer after the platform abstraction is ready. This prevents Windows/POSIX terminal details from leaking into CLAW's editor core.

## Dependencies and verification

There are **no third-party runtime dependencies**. The supplied Linux x86-64 binary is statically linked. Building from source requires a C11-capable compiler; `make` is optional. Each target operating system needs a native executable built for that platform—one Linux executable cannot also run natively on macOS or Windows.

The source was compiled with both GCC-compatible `cc` and Clang on Linux, passed address/undefined-behavior sanitizer smoke checks, and its plain ASCII and truecolor output was compared against your uploaded Python renderer. The plain ASCII output matched character-for-character at five terminal sizes/aspect ratios; the truecolor output also matched byte-for-byte in a sample test. macOS and Windows builds still need to run on their respective native systems. The included `.github/workflows/build-splash.yml` builds platform-native artifacts for Linux, macOS, and Windows when this folder is pushed to GitHub; those artifacts can be downloaded without installing a compiler locally.
