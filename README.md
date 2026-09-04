# claw
A lightweight, Git-aware text editor written entirely in C for Linux-based operating systems.

Claw is being developed as part of a larger project to build a Linux-based operating system from scratch. The goal is to create a fast, modular, and extensible text editor that supports multiple keyboard layouts, Git integration, and future plugin support.

---

## Current Status

Development Stage: Functional Prototype

Current Version:

```text
v0.5
```

Completed Features:

- Raw terminal mode
- Real-time keyboard input
- Arrow key navigation
- Cursor state management
- Screen rendering
- Dynamic text buffer
- Character insertion
- Character deletion
- Multi-line editing
- Dynamic row creation
- Enter key support
- Line splitting
- Line merging
- File loading
- File saving
- Filename tracking
- Real file editing

In Progress:

- Status bar
- Modified file tracking
- File information display

Planned:

- Multiple keyboard profiles
- Git integration
- Syntax highlighting
- Plugin system

---

## Project Goals

Claw aims to provide:

- Lightweight architecture
- Native C implementation
- Git awareness
- Windows-style shortcuts
- macOS-style shortcuts
- Linux-style shortcuts
- Configurable key bindings
- Extensible plugin architecture

---

## Project Structure

```text
claw/
│
├── README.md
├── LICENSE
├── Makefile
├── .gitignore
│
├── docs/
├── assets/
├── config/
├── keymaps/
├── tests/
├── include/
├── src/
├── plugins/
└── build/
```

---

## Current Source Structure

```text
src/

main.c

editor/
    editor.c

input/
    input.c
    rawmode.c

buffer/
    buffer.c
    row.c
    cursor.c

render/
    render.c

fileio/
    fileio.c
```

---

## Build Instructions

Compile:

```bash
make
```

Run Empty Editor:

```bash
./claw
```

Open Existing File:

```bash
./claw notes.txt
```

Clean Build Files:

```bash
make clean
```

Rebuild:

```bash
make rebuild
```

---

## Current Controls

### Navigation

```text
↑  Move Cursor Up
↓  Move Cursor Down
←  Move Cursor Left
→  Move Cursor Right
```

### Editing

```text
Printable Keys -> Insert Character
Delete          -> Delete Character
Return          -> New Line
```

### File Operations

```text
Ctrl + S        -> Save File
q               -> Quit Editor
```

---

## Architecture

```text
Keyboard
    │
    ▼
Input Engine
    │
    ▼
Editor Core
    │
    ▼
Text Buffer
    │
    ▼
File I/O Layer
    │
    ▼
Renderer
    │
    ▼
Terminal
```

---

## Development Roadmap

### Milestone 1

- Project setup
- Editor loop
- Rendering engine

Status:

```text
COMPLETED
```

---

### Milestone 2

- Raw mode
- Keyboard input
- Cursor movement
- Arrow key navigation

Status:

```text
COMPLETED
```

---

### Milestone 3

- Text buffer
- Character insertion
- Character deletion

Status:

```text
COMPLETED
```

---

### Milestone 4

Features:

- Multi-line editing
- Dynamic row creation
- Enter key support
- Vertical cursor navigation
- Line splitting
- Line merging

Status:

```text
COMPLETED
```

---

### Milestone 5

Features:

- Open existing files
- Save files
- Create new files
- File buffer loading
- Filename tracking
- Real file editing

Status:

```text
COMPLETED
```

---

### Milestone 6

Features:

- Status bar
- Current filename display
- Modified (*) indicator
- Line number display
- Column number display

Status:

```text
NEXT
```

---

### Milestone 7

- Keymap engine
- Windows profile
- macOS profile
- Linux profile

Status:

```text
PLANNED
```

---

### Milestone 8

- Git integration
- Git status
- Git diff

Status:

```text
PLANNED
```

---

## Implemented Buffer Features

### Character Editing

- Insert characters at cursor position
- Delete characters using Backspace/Delete
- Dynamic row resizing

### Multi-Line Editing

- Create new lines using Enter
- Split lines at cursor position
- Merge lines using Backspace

### File Operations

- Open files into editor buffer
- Save editor buffer to disk
- Preserve multi-line file structure
- Track active filename

---

## Future Features

- Syntax highlighting
- Undo / Redo
- Search and replace
- Plugin support
- Theme support
- Integrated terminal
- Language Server Protocol (LSP)

---

## Development Progress

| Version | Status | Description |
|----------|----------|----------|
| v0.1 | Complete | Project setup |
| v0.2 | Complete | Raw terminal mode |
| v0.3 | Complete | Text buffer editing |
| v0.4 | Complete | Multi-line editing |
| v0.5 | Complete | File open/save |
| v0.6 | Next | Status bar |
| v0.7 | Planned | Multi-keymap support |
| v0.8 | Planned | Git integration |

---

## License

MIT License

---

## Author

Aryan Gupta

Project: Claw Text Editor
Language: C
Platform: Linux