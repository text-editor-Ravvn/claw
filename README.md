# Claw

A lightweight terminal text editor written entirely in C.

Claw is being developed as part of a larger project to build a Linux-based operating system from scratch. The goal is to create a fast, modular, and extensible text editor while remaining simple, lightweight, and easy to understand.

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
- Backspace deletion
- Delete key support
- Multi-line editing
- Dynamic row creation
- Enter key support
- Line splitting
- Line merging
- Horizontal cursor navigation
- Vertical cursor navigation
- File loading
- File saving
- Filename tracking
- Modified buffer tracking
- Real file editing
- New file creation
- Existing file editing
- Unsaved changes detection
- Exit confirmation
- ANSI terminal rendering
- POSIX terminal support
- Global command support (claw)
- Status bar
- Current filename display
- Modified file indicator
- Line number display
- Column number display

In Progress:

- Status messages
- Screen refresh optimization
- Terminal resize handling
- Cursor rendering improvements
- Terminal compatibility improvements

Planned:

- Vertical scrolling
- Horizontal scrolling
- Viewport rendering
- Multiple keyboard profiles
- Configurable key bindings
- Syntax highlighting
- Search and replace
- Undo / Redo
- Git integration
- Plugin system

---

## Project Goals

Claw aims to provide:

- Lightweight architecture
- Native C implementation
- Fast and responsive editing
- Efficient text buffer management
- Reliable file handling
- Keyboard-driven workflow
- Multiple keyboard profiles
- Configurable key bindings
- Syntax highlighting
- Git integration
- Extensible plugin architecture
- Foundation for future operating system development

---

## Project Structure

```text
claw/

├── README.md
├── LICENSE
├── Makefile
├── .gitignore
│
├── docs/
│
├── assets/
│
├── tests/
│
├── include/
│   ├── editor.h
│   ├── input.h
│   ├── rawmode.h
│   ├── render.h
│   ├── buffer.h
│   ├── cursor.h
│   ├── row.h
│   ├── fileio.h
│   └── statusbar.h
│
├── src/
│   │
│   ├── main.c
│   │
│   ├── editor/
│   │   └── editor.c
│   │
│   ├── input/
│   │   ├── input.c
│   │   └── rawmode.c
│   │
│   ├── buffer/
│   │   ├── buffer.c
│   │   ├── row.c
│   │   └── cursor.c
│   │
│   ├── render/
│   │   └── render.c
│   │
│   ├── fileio/
│   │   └── fileio.c
│   │
│   └── ui/
│       └── statusbar.c
│
└── build/
```

---

## Current Source Structure

```text
## Current Source Structure

src/

├── main.c                 Entry point
│
├── editor/
│   └── editor.c           Core editor loop
│
├── input/
│   ├── input.c            Keyboard input handling
│   └── rawmode.c          Terminal raw mode management
│
├── buffer/
│   ├── buffer.c           Text buffer management
│   ├── row.c              Row operations
│   └── cursor.c           Cursor movement logic
│
├── render/
│   └── render.c           Screen rendering engine
│
├── fileio/
│   └── fileio.c           File loading and saving
│
└── ui/
    └── statusbar.c        Status bar rendering
```

---

## Build Instructions

### Requirements

- GCC or Clang
- Make
- POSIX-compatible terminal

### Compile

```bash
make
```

### Run Empty Editor

```bash
./claw
```

### Open Existing File

```bash
./claw notes.txt
```

### Save Changes

```text
Ctrl + S
```

### Exit Editor

```text
Ctrl + X
```

If unsaved changes exist:

```text
Press Ctrl + X twice to force quit
```

### Clean Build Files

```bash
make clean
```

### Rebuild

```bash
make rebuild
```

### Install Globally

```bash
sudo ln -sf "$(pwd)/claw" /usr/local/bin/claw
```

Verify installation:

```bash
which claw
```

Run from anywhere:

```bash
claw
```

Open a file from anywhere:

```bash
claw notes.txt
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
Printable Keys  -> Insert Character
Backspace       -> Delete Character Left
Delete          -> Delete Character Right
Enter           -> Create New Line
```

### File Operations

```text
Ctrl + S        -> Save File
Ctrl + X        -> Quit Editor

Ctrl + X twice  -> Force Quit
                  (when unsaved changes exist)
```

### Status Information

```text
Status Bar      -> Current File Information
[Modified]      -> Unsaved Changes Indicator
Ln              -> Current Line Number
Col             -> Current Column Number
```
---

## Architecture

```text
┌─────────────────┐
│    Terminal     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Input Engine   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Editor Core   │
└────────┬────────┘
         │
   ┌─────┴─────┐
   ▼           ▼
┌───────┐ ┌─────────┐
│Buffer │ │ File I/O│
└───┬───┘ └────┬────┘
    │          │
    ▼          ▼
┌─────────────────┐
│    Renderer     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Status Bar    │
└─────────────────┘
```

### Component Overview

Input Engine
- Captures keyboard input
- Processes control keys
- Handles arrow key navigation
- Manages terminal raw mode

Editor Core
- Controls the editor event loop
- Coordinates editor subsystems
- Processes editing commands
- Maintains editor state

Text Buffer
- Stores document contents
- Manages rows and characters
- Supports insertion and deletion
- Tracks file modifications

Renderer
- Draws editor contents
- Updates cursor position
- Renders status bar
- Refreshes terminal display

File I/O Layer
- Loads files into memory
- Saves buffer contents to disk
- Tracks active filename
- Preserves document structure

Terminal Layer
- Provides user interaction
- Displays editor output
- Receives keyboard events
```

---

### Development Roadmap

### Milestone 1

Features:

- Project setup
- Editor loop
- Rendering engine

Status:

```text
COMPLETED
```

---

### Milestone 2

Features:

- Raw terminal mode
- Keyboard input
- Cursor movement
- Arrow key navigation

Status:

```text
COMPLETED
```

---

### Milestone 3

Features:

- Dynamic text buffer
- Character insertion
- Character deletion
- Cursor state management

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
- Line splitting
- Line merging
- Vertical cursor navigation

Status:

```text
COMPLETED
```

---

### Milestone 5

Features:

- File loading
- File saving
- Filename tracking
- Real file editing
- Modified buffer tracking

Status:

```text
COMPLETED
```

---

### Milestone 6

Features:

- Status bar
- Current filename display
- Modified file indicator
- Line number display
- Column number display
- Exit confirmation

Status:

```text
COMPLETED
```

---

### Milestone 7

Features:

- Scrolling and viewport support
- Large file handling
- Terminal resize handling
- Screen refresh optimization

Status:

```text
NEXT
```

---

### Milestone 8

Features:

- Search and replace
- Search all files
- Go to line
- Navigation improvements

Status:

```text
PLANNED
```

---

### Milestone 9

Features:

- Undo / Redo
- Persistent history
- Advanced editing operations

Status:

```text
PLANNED
```

---

### Milestone 10

Features:

- Multiple keyboard profiles
- Configurable key bindings
- Linux keymap profile

Status:

```text
PLANNED
```

---

### Milestone 11

Features:

- Syntax highlighting
- Tree-sitter integration
- Language detection

Status:

```text
PLANNED
```

---

### Milestone 12

Features:

- Git integration
- Git status
- Git diff
- Commit workflow

Status:

```text
PLANNED
```

---

### Milestone 13

Features:

- Plugin system
- Python plugins
- Custom scripting support

Status:

```text
PLANNED
```

---

## Implemented Buffer Features

### Character Editing

- Insert characters at cursor position
- Delete characters using Backspace
- Delete characters using Delete key
- Dynamic row resizing
- In-line text editing
- Cursor-aware insertion

### Cursor Management

- Horizontal cursor movement
- Vertical cursor movement
- Cursor position tracking
- Row boundary handling
- Column preservation during navigation

### Multi-Line Editing

- Create new lines using Enter
- Split lines at cursor position
- Merge lines using Backspace
- Dynamic row creation
- Dynamic row deletion
- Multi-line buffer navigation

### Buffer Management

- Dynamic memory allocation
- Dynamic row storage
- Automatic buffer growth
- Modification tracking
- Active document management

### File Operations

- Open files into editor buffer
- Save editor buffer to disk
- Create new files
- Edit existing files
- Preserve multi-line file structure
- Track active filename
- Detect unsaved changes
- Exit confirmation for modified files

### Rendering

- Full-screen terminal rendering
- Cursor rendering
- Real-time screen refresh
- Status bar rendering
- Filename display
- Modified file indicator
- Line and column display

### Terminal Support

- Raw terminal mode
- Real-time keyboard input
- ANSI escape sequence rendering
- POSIX terminal support
- Arrow key processing
- Control key handling

---

## Development Progress

| Version | Status | Description |
|----------|----------|----------|
| v0.1 | Complete | Project setup and editor framework |
| v0.2 | Complete | Raw terminal mode and keyboard input |
| v0.3 | Complete | Dynamic text buffer and editing |
| v0.4 | Complete | Multi-line editing and cursor navigation |
| v0.5 | Complete | File loading, saving, and filename tracking |
| v0.6 | Complete | Status bar, modified tracking, and file information display |
| v0.7 | Next | Scrolling and viewport support |
| v0.8 | Planned | Search and replace |
| v0.9 | Planned | Undo / Redo system |
| v1.0 | Planned | Multiple keyboard profiles and configurable key bindings |
| v1.1 | Planned | Syntax highlighting |
| v1.2 | Planned | Git integration |
| v1.3 | Planned | Plugin system |

---

## License

MIT License

---

## Author

Aryan Gupta & Pranav Chauhan 

Project: Claw Text Editor
Language: C
Platform: Linux
