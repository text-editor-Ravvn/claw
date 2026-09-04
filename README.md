# Claw Text Editor

Claw is a lightweight terminal text editor written in C for Linux and other
POSIX systems. The project is a functional prototype with a small codebase
that is easy to study and extend.

## Quick Start

Requirements:

- Linux or another POSIX terminal
- GCC
- GNU Make

Build and run:

```bash
make
./claw
./claw notes.txt
```

Useful development commands:

```bash
make test       # Build and run buffer and file I/O tests
make asan       # Build with AddressSanitizer and UBSan
make debug      # Clean and build a debug executable
make rebuild    # Clean all generated files and rebuild
make clean      # Remove objects, executables, and test binaries
```

## Controls

| Key | Action |
| --- | --- |
| Arrow keys | Move the cursor |
| Printable characters | Insert text, including lowercase `q` |
| Enter | Split the current line |
| Backspace | Delete before the cursor or join with the previous line |
| Delete | Delete after the cursor or join with the next line |
| Ctrl-S | Save the current file |
| Ctrl-Q | Quit; press twice when edits are unsaved |

The editor uses raw terminal input and restores the terminal when it exits.
The status bar shows the current filename, saved or modified state, cursor
line, cursor column, and the latest save or quit message.

## Current Features

- Dynamic rows and character insertion
- Multi-line editing and row merging
- Cursor movement with bounds checking
- Forward Delete and Backspace
- Binary-safe file loading and saving
- Long-line support
- Embedded NUL-byte preservation
- Preservation of a file's final-newline state
- Transactional file loading
- Dirty-buffer tracking
- ANSI terminal rendering
- Buffer and file regression tests

## Repository Guide

### Root Files

- `README.md`: This project guide, including commands, controls, architecture,
  and roadmap.
- `Makefile`: GCC build rules for the editor, tests, debug builds, sanitizer
  builds, cleanup, and rebuilding.
- `LICENSE`: Project license file. Add the approved license text before release.
- `.gitignore`: Keeps object files, executables, test binaries, and temporary
  test files out of version control.
- `notes.txt`: Small sample text file for manual editor testing.
- `test.txt`: Additional sample text file for manual testing.
- `claw`: Generated editor executable; it is not source code and is ignored by
  Git when generated locally.

### Public Headers in `include/`

- `buffer.h`: Defines `Row` and `Buffer`, plus initialization, cleanup, and
  editing functions.
- `cursor.h`: Defines the zero-based `Cursor` and movement functions.
- `editor.h`: Declares editor startup, input-loop, shutdown, and status APIs.
- `fileio.h`: Declares binary-safe file open and save functions.
- `input.h`: Defines control-key conversion, decoded key values, and `readKey`.
- `rawmode.h`: Declares POSIX terminal raw-mode setup and restoration.
- `render.h`: Declares full-screen redraw support.
- `statusbar.h`: Declares status-bar rendering.
- `command.h`: Reserved for future command definitions and dispatch APIs.
- `config.h`: Reserved for future configuration APIs.
- `git.h`: Reserved for future Git status, diff, and commit APIs.
- `keymap.h`: Reserved for future configurable keyboard profiles.
- `utils.h`: Reserved for shared file, string, and logging utilities.

### Active Source Files in `src/`

- `main.c`: Program entry point. Initializes Claw, optionally opens the file
  passed on the command line, runs the editor, and shuts it down.
- `editor/editor.c`: Main editor lifecycle and command loop. Handles movement,
  insertion, deletion, saving, quit confirmation, and status messages.
- `input/input.c`: Reads stdin and decodes ordinary characters, arrow keys,
  Escape, and the Delete escape sequence.
- `input/rawmode.c`: Configures POSIX raw terminal mode and restores the
  original terminal settings.
- `buffer/buffer.c`: Creates the initial one-row buffer and frees all rows.
- `buffer/row.c`: Inserts characters, deletes characters, splits rows, joins
  rows, resizes allocations, and marks successful edits as modified.
- `buffer/cursor.c`: Moves the cursor while clamping it to valid rows and
  columns.
- `fileio/fileio.c`: Loads files dynamically and saves them without losing
  embedded NUL bytes or final-newline information.
- `render/render.c`: Clears the terminal, draws document rows, draws the status
  bar, and places the visible terminal cursor.
- `ui/statusbar.c`: Displays filename, saved/modified state, cursor position,
  and transient messages.

### Future Source Scaffolds

These files contain comments describing their intended responsibility but are
not currently compiled by the Makefile:

- `command/actions.c`: Future editor actions invoked by named commands.
- `command/command.c`: Future command definitions and metadata.
- `command/dispatcher.c`: Future command routing.
- `editor/editor_stauts.c`: Future extracted editor-status logic. The filename
  currently contains the historical `stauts` spelling.
- `fileio/load.c`: Future standalone loading module.
- `fileio/save.c`: Future standalone saving module.
- `git/git.c`: Shared Git repository operations.
- `git/status.c`: Git working-tree status.
- `git/diff.c`: Git diff display.
- `git/commit.c`: Git commit creation.
- `input/keyboard.c`: Higher-level keyboard mapping.
- `keymap/bindings.c`: Key-to-command binding storage.
- `keymap/keymap.c`: Keyboard profile management.
- `keymap/parser.c`: Keymap configuration parsing.
- `render/colors.c`: Terminal colors and themes.
- `render/screen.c`: Terminal-size and viewport helpers.
- `ui/help.c`: Interactive help screen.
- `ui/welcome.c`: Startup welcome screen.
- `utils/file_utils.c`: Shared filesystem helpers.
- `utils/logger.c`: Diagnostic and event logging.
- `utils/string_utils.c`: Shared string helpers.

### Configuration and Keymaps

- `config/claw.conf`: Reserved for user-specific settings.
- `config/default.conf`: Reserved for default settings.
- `keymaps/linus.conf`: Reserved for Linux-style bindings.
- `keymaps/mac.conf`: Reserved for macOS-style bindings.
- `keymaps/windows.conf`: Reserved for Windows-style bindings.

### Documentation Files

- `docs/api.md`: Reserved for public C API notes.
- `docs/architecture.md`: Describes future module data flow and boundaries.
- `docs/keymaps.md`: Reserved for the keymap file format.
- `docs/roadmap.md`: Points to the active roadmap in this README.

### Tests

- `tests/buffer_test.c`: Tests insertion, line splitting, Backspace, Forward
  Delete, and row merging.
- `tests/file_test.c`: Tests embedded NUL bytes and exact file round trips.
- `tests/input_test.c`: Reserved for future key-decoding tests.
- `tests/git_test.c`: Reserved for future Git integration tests.

## Architecture

The runtime flow is:

```text
main
  -> editorInit
  -> openFile (optional)
  -> editorRun
       -> readKey
       -> buffer/cursor operation
       -> refreshScreen
  -> editorShutdown
```

The `Buffer` owns an array of `Row` values. Each row owns a byte array and a
length, so file data can contain NUL bytes without using C string length as the
source of truth. The editor owns the command loop, while rendering reads the
current buffer and cursor state without mutating document data.

## Roadmap

1. Add terminal-size detection and viewport scrolling.
2. Improve status messages and add a modified-file indicator workflow.
3. Add undo and redo.
4. Add search and replace.
5. Implement configurable keymaps.
6. Add Git status and diff integration.
7. Add syntax highlighting and plugin support.

## License

The repository contains a `LICENSE` placeholder. Add the final approved license
text before distributing the project.

## Author

Aryan Gupta & Pranav Chauhan
