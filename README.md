# Claw Text Editor

Claw is a lightweight terminal text editor written in C for Linux. It is a
small, native codebase intended for experimentation with editor internals,
terminal input, file buffers, and future Git integration.

## Current Status

Version: `v0.5`
Stage: Functional prototype

Implemented:

- POSIX raw terminal input
- Arrow-key cursor navigation
- Character insertion and backspace deletion
- Forward Delete support
- Multi-line editing with line splitting and merging
- Dynamic text buffers and row resizing
- File opening and saving
- Long-line and embedded-NUL file handling
- Preservation of whether a file ends with a newline
- Visible cursor positioning
- Automated buffer and file I/O regression tests

Not yet implemented:

- Status bar and modified-file indicator
- Undo and redo
- Search and replace
- Syntax highlighting
- Configurable keymaps
- Git integration

## Requirements

- Linux or another POSIX terminal environment
- GCC
- GNU Make

## Build and Run

Build the editor:

```bash
make
```

Start with an empty buffer:

```bash
./claw
```

Open a file:

```bash
./claw notes.txt
```

Clean generated files and rebuild:

```bash
make clean
make rebuild
```

Build with AddressSanitizer and UndefinedBehaviorSanitizer:

```bash
make asan
```

Run the regression tests:

```bash
make test
```

## Controls

| Key | Action |
| --- | --- |
| Arrow keys | Move the cursor |
| Printable characters | Insert text |
| Enter | Split the current line |
| Backspace | Delete before the cursor or merge with the previous line |
| Delete | Delete after the cursor or merge with the next line |
| Ctrl-S | Save the current file |
| Ctrl-Q | Quit |

The editor exits when standard input reaches EOF. Files are written in binary
mode so line contents, including embedded NUL bytes, are preserved.

## Project Layout

```text
include/       Public C interfaces
src/buffer/    Rows, buffer mutations, and cursor movement
src/editor/    Editor initialization and input loop
src/fileio/    File loading and saving
src/input/     Key decoding and POSIX raw mode
src/render/    Terminal screen rendering
tests/         Regression tests
config/        Editor configuration drafts
keymaps/       Keyboard profile drafts
docs/          Design notes and roadmap
```

## Development Roadmap

1. Add a status bar with filename, cursor position, and save state.
2. Add viewport scrolling and terminal-size detection.
3. Add undo and redo.
4. Add search and replace.
5. Add configurable keymaps and Git-aware workflows.
6. Add syntax highlighting and plugin support.

## License

MIT License

## Author

Aryan Gupta
