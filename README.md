# Claw

A lightweight terminal text editor written entirely in C.

Claw is being developed as part of a larger project to build a Linux-based operating system from scratch. The goal is to create a fast, modular, and extensible text editor while remaining simple, lightweight, and easy to understand.

---

## Current Status

Development Stage: Functional Prototype

Current Version:

```text
v0.7
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

- Status bar
- Modified file tracking
- Current filename display
- Line and column display

- Vertical scrolling
- Horizontal scrolling
- Viewport management
- Terminal size detection
- Dynamic viewport rendering

- Welcome screen
- Centered startup screen
- Empty line (~) rendering
- Window resize support

- File statistics display

In Progress:

- Search system (Ctrl+F)
- Search navigation
- Match highlighting

Planned:

- Multiple keyboard profiles
- Linux keymap profile
- Git integration
- Syntax highlighting
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

├── Makefile
├── README.md

├── include/
│   ├── buffer.h
│   ├── cursor.h
│   ├── editor.h
│   ├── fileio.h
│   ├── input.h
│   ├── rawmode.h
│   ├── render.h
│   ├── statusbar.h
│   └── viewport.h

└── src/

    ├── main.c

    ├── buffer/
    │   ├── buffer.c
    │   └── row.c

    ├── cursor/
    │   └── cursor.c

    ├── editor/
    │   └── editor.c

    ├── fileio/
    │   └── fileio.c

    ├── input/
    │   ├── input.c
    │   └── rawmode.c

    ├── render/
    │   └── render.c

    ├── statusbar/
    │   └── statusbar.c

    └── viewport/
        └── viewport.c
```

---

## Current Source Structure

```text

src/

├── main.c                 Entry point and application startup
│
├── editor/
│   └── editor.c           Core editor loop and command handling
│
├── input/
│   ├── input.c            Keyboard input processing and key decoding
│   └── rawmode.c          Terminal raw mode management
│
├── buffer/
│   ├── buffer.c           Text buffer initialization and memory management
│   ├── row.c              Row insertion, deletion, splitting, and merging
│   └── cursor.c           Cursor movement and position management
│
├── render/
│   └── render.c           Screen rendering and viewport drawing
│
├── fileio/
│   └── fileio.c           File loading, saving, and persistence
│
├── statusbar/
│   └── statusbar.c        Status bar and editor information display
│
└── viewport/
    └── viewport.c         Scrolling, viewport tracking, and window sizing
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

### Viewport Navigation

```text
Arrow Keys      -> Navigate Through File

Auto Vertical Scroll

Auto Horizontal Scroll

Window Resize Support
```

### Status Information

```text
Status Bar      -> Current File Information

[Modified]      -> Unsaved Changes Indicator

Lines           -> Total Line Count

Ln              -> Current Line Number

Col             -> Current Column Number
```
---

## Architecture

```text
                     ┌──────────────┐
                     │   Keyboard   │
                     └──────┬───────┘
                            │
                            ▼
                     ┌──────────────┐
                     │ Input Engine │
                     └──────┬───────┘
                            │
                            ▼
                     ┌──────────────┐
                     │ Editor Core  │
                     └──────┬───────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
        ▼                   ▼                   ▼
 ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
 │   Cursor    │    │   Buffer    │    │  Viewport   │
 │   Engine    │    │   Engine    │    │   Engine    │
 └──────┬──────┘    └──────┬──────┘    └──────┬──────┘
        │                  │                  │
        └──────────┬───────┴──────────┬───────┘
                   │                  │
                   ▼                  ▼
           ┌─────────────┐    ┌─────────────┐
           │  File I/O   │    │ Status Bar  │
           │   Layer     │    │   System    │
           └──────┬──────┘    └──────┬──────┘
                  │                  │
                  └────────┬─────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  Renderer   │
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  Terminal   │
                    └─────────────┘
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

Cursor Engine

- Tracks cursor position
- Handles cursor movement
- Maintains row and column state
- Supports viewport-aware navigation

Text Buffer

- Stores document contents
- Manages rows and characters
- Supports insertion and deletion
- Tracks file modifications

Viewport Engine

- Tracks visible file area
- Handles vertical scrolling
- Handles horizontal scrolling
- Adapts to terminal resizing

Renderer

- Draws editor contents
- Renders the welcome screen
- Updates cursor position
- Refreshes terminal display

Status Bar System

- Displays file information
- Shows modification status
- Displays line count
- Shows cursor position

File I/O Layer

- Loads files into memory
- Saves buffer contents to disk
- Tracks active filename
- Preserves document structure

Terminal Layer

- Provides user interaction
- Displays editor output
- Receives keyboard events
- Hosts the editor interface
```

---

## Development Roadmap

### Milestone 1 — Foundation

**Status:** Completed

Features:

- Project structure setup
- Build system configuration
- Editor startup sequence
- Basic screen rendering

---

### Milestone 2 — Input System

**Status:** Completed

Features:

- Terminal raw mode
- Keyboard input processing
- Special key detection
- Arrow key navigation

---

### Milestone 3 — Text Buffer

**Status:** Completed

Features:

- Dynamic text buffer
- Character insertion
- Character deletion
- Memory-safe row management

---

### Milestone 4 — Multi-Line Editing

**Status:** Completed

Features:

- New line creation
- Line splitting
- Line merging
- Multi-line cursor navigation

---

### Milestone 5 — File Operations

**Status:** Completed

Features:

- Open existing files
- Save files
- Create new files
- Filename tracking
- Modified state tracking

---

### Milestone 6 — User Interface

**Status:** Completed

Features:

- Status bar
- Current filename display
- Modified indicator
- Cursor position display
- Line statistics

---

### Milestone 7 — Viewport System

**Status:** Completed

Features:

- Vertical scrolling
- Horizontal scrolling
- Viewport tracking
- Window resize support
- Welcome screen
- Tilde rendering

---

### Milestone 8 — Search System

**Status:** In Progress

Features:

- Ctrl + F search
- Search prompt
- Find next match
- Find previous match
- Match highlighting

---

### Milestone 9 — Editing Enhancements

**Status:** Planned

Features:

- Undo support
- Redo support
- Clipboard operations
- Improved text navigation

---

### Milestone 10 — Syntax Highlighting

**Status:** Planned

Features:

- C syntax highlighting
- Keyword detection
- Comment highlighting
- String highlighting

---

### Milestone 11 — Productivity Features

**Status:** Planned

Features:

- Go to line
- Command palette
- Recent files
- Search and replace

---

### Milestone 12 — Git Integration

**Status:** Planned

Features:

- Git status
- Git diff viewer
- Branch information
- Change indicators

---

### Milestone 13 — Plugin System

**Status:** Planned

Features:

- Plugin API
- External extensions
- Custom commands
- Editor customization

---

### Milestone 14 — Claw v1.0

**Status:** Future Goal

Features:

- Stable release
- Optimized performance
- Large file support
- Complete documentation
- Production-ready architecture
```

---

# Implemented Features

## Character Editing

- Insert characters at cursor position
- Delete characters using Backspace
- Delete characters using Delete key
- Dynamic row resizing
- In-line text editing
- Cursor-aware insertion

## Cursor Management

- Horizontal cursor movement
- Vertical cursor movement
- Cursor position tracking
- Row boundary handling
- Column preservation during navigation

## Multi-Line Editing

- Create new lines using Enter
- Split lines at cursor position
- Merge lines using Backspace
- Dynamic row creation
- Dynamic row deletion
- Multi-line buffer navigation

## Buffer Management

- Dynamic memory allocation
- Dynamic row storage
- Automatic buffer growth
- Modification tracking
- Active document management

## File Operations

- Open files into editor buffer
- Save editor buffer to disk
- Create new files
- Edit existing files
- Preserve multi-line file structure
- Track active filename
- Detect unsaved changes
- Exit confirmation for modified files

## Viewport System

- Vertical scrolling
- Horizontal scrolling
- Viewport tracking
- Dynamic visible-area rendering
- Window resize support
- Cursor-follow scrolling
- Terminal dimension detection

## Rendering

- Full-screen terminal rendering
- Cursor rendering
- Real-time screen refresh
- Status bar rendering
- Welcome screen rendering
- Tilde (`~`) line rendering
- Filename display
- Modified file indicator
- Line count display
- Line and column display

## Status Bar

- Current filename display
- Modified state indicator
- Total line count
- Current line number
- Current column number

## Terminal Support

- Raw terminal mode
- Real-time keyboard input
- ANSI escape sequence rendering
- POSIX terminal support
- Arrow key processing
- Control key handling

## User Interface

- Centered welcome screen
- Version display
- Keyboard shortcut hints
- Responsive layout
- Dynamic viewport updates

---

# Development Progress

| Version | Status | Description |
|----------|----------|----------|
| v0.1 | Complete | Project setup and editor framework |
| v0.2 | Complete | Raw terminal mode and keyboard input |
| v0.3 | Complete | Dynamic text buffer and editing |
| v0.4 | Complete | Multi-line editing and cursor navigation |
| v0.5 | Complete | File loading, saving, and filename tracking |
| v0.6 | Complete | Status bar, modified tracking, and file information display |
| v0.7 | Complete | Viewport system, vertical/horizontal scrolling, welcome screen, and editor polish |
| v0.8 | In Progress | Search system (Ctrl + F) |
| v0.9 | Planned | Undo / Redo system |
| v1.0 | Planned | Multiple keyboard profiles and configurable key bindings |
| v1.1 | Planned | Syntax highlighting |
| v1.2 | Planned | Search and Replace |
| v1.3 | Planned | Git integration |
| v1.4 | Planned | Plugin system |

---

## License

MIT License

---

## Author

Aryan Gupta & Pranav Chauhan 

Project: Claw Text Editor
Language: C
Platform: Linux
