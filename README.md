# claw
A lightweight, Git-aware text editor written entirely in C for Linux-based operating systems.

Claw is being developed as part of a larger project to build a Linux-based operating system from scratch. The goal is to create a fast, modular, and extensible text editor that supports multiple keyboard layouts, Git integration, and future plugin support.

---

## Current Status

Development Stage: Early Prototype

Current Version:

```text
v0.3
```

Completed Features:

- Raw terminal mode
- Real-time keyboard input
- Arrow key navigation
- Cursor state management
- Screen rendering
- Text buffer initialization
- Character insertion
- Backspace deletion
- Basic editor loop

In Progress:

- Multi-line editing
- Enter key support
- Row management

Planned:

- File open/save
- Status bar
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
```

---

## Build Instructions

Compile:

```bash
make
```

Run:

```bash
./claw
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
Backspace      -> Delete Character
q              -> Quit Editor
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
Command Processing
    │
    ▼
Text Buffer
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

- Enter key support
- Multi-line editing
- Row management

Status:

```text
NEXT
```

---

### Milestone 5

- Open files
- Save files
- Create new files

Status:

```text
PLANNED
```

---

### Milestone 6

- Status bar
- File information
- Cursor information

Status:

```text
PLANNED
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

## Future Features

- Syntax highlighting
- Undo / Redo
- Search and replace
- Plugin support
- Theme support
- Integrated terminal
- Language Server Protocol (LSP)

---

## License

MIT License

---

## Author

Aryan Gupta

Project: Claw Text Editor
Language: C
Platform: Linux