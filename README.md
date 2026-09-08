# Claw

A lightweight terminal text editor written entirely in C.

Claw is being developed as part of a larger project to build a Linux-based operating system from scratch. The goal is to create a fast, modular, and extensible text editor while remaining simple, lightweight, and easy to understand.

---

## Current Status

Development Stage: Feature-Complete Editor Core

Current Version:

```text
v1.0
```

## Completed Features

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

### Viewport & Scrolling

- Vertical scrolling
- Horizontal scrolling
- Cursor-aware viewport movement
- Dynamic viewport updates
- Large file navigation support

### Search System

- Search prompt (Ctrl + F)
- Real-time search input
- Match discovery across document
- Search result highlighting
- Jump cursor to matched text
- Next match navigation (Ctrl + N)
- Previous match navigation (Ctrl + P)
- Search cancellation (Esc)
- Search state management

### Undo / Redo System

- Undo character insertions
- Undo character deletions
- Undo line splits
- Redo character insertions
- Redo character deletions
- Redo line splits
- Multi-line history support
- Edit action tracking
- Separate undo and redo stacks
- History recording control

### Rendering

- Full-screen terminal rendering
- Cursor rendering
- Real-time screen refresh
- Status bar rendering
- Search match highlighting
- Welcome screen rendering
- Filename display
- Modified file indicator
- Line and column display

### User Interface

- Welcome screen
- Status bar
- Search prompt
- File information display
- Current line and column display
- Unsaved changes indicator
- Context-aware status messages

### Terminal Support

- Raw terminal mode
- Real-time keyboard input
- ANSI escape sequence rendering
- POSIX terminal support
- Arrow key processing
- Control key handling
- Dynamic terminal resizing

### Editor Configuration

- INI-style configuration file parser
- User configuration file support (XDG and local fallback)
- Configurable tab width (1–16 spaces)
- Toggleable line number display
- Toggleable welcome screen
- Toggleable status bar
- Startup preference loading
- Layered settings (defaults → user overrides)
- Custom key binding system
- Keymap file support with multiple profiles
- Command dispatch architecture
- Tab key insertion support

## In Progress

### v1.1 — Syntax Highlighting

- C language syntax highlighting
- Keyword highlighting
- Number highlighting
- String highlighting
- Comment highlighting
- File extension detection
- Colorized rendering engine

### Current Focus

- Designing syntax highlighting architecture
- Defining highlight rule format
- Implementing file type detection
- Building colorized rendering pipeline

## Planned Features

### v1.1 — Syntax Highlighting

- C language syntax highlighting
- Keyword highlighting
- Number highlighting
- String highlighting
- Comment highlighting
- File extension detection
- Colorized rendering engine

### v1.2 — Search & Replace

- Find and replace text
- Replace current match
- Replace all matches
- Case-sensitive search
- Whole-word search
- Search statistics

### v1.3 — Git Integration

- Git repository detection
- Current branch display
- Modified file indicators
- Git status integration
- Commit support
- Diff visualization

### v1.4 — Plugin System

- Plugin loading framework
- Dynamic module support
- Plugin API
- Custom commands
- Third-party extensions
- Plugin configuration support

### v1.5 — Advanced Editing

- Auto indentation
- Tab and space conversion
- Duplicate line command
- Move line up/down
- Line selection tools
- Bracket matching

### v1.6 — Productivity Features

- Multiple cursors
- Bookmarks
- Recent files list
- Command palette
- Go to line
- File explorer

### Long-Term Vision

- Multiple file buffers
- Split-screen editing
- Mouse support
- Theme engine
- Language Server Protocol (LSP) support
- Integrated terminal
- Project workspace management

---

## Project Goals

### Primary Goals

- Build a lightweight terminal-based text editor in C
- Gain hands-on experience with low-level systems programming
- Develop a modular and maintainable codebase
- Learn terminal control and raw mode programming
- Implement core text editing functionality from scratch

### Technical Goals

- Create a responsive full-screen terminal interface
- Support efficient text editing and navigation
- Implement dynamic memory management for document storage
- Provide reliable file loading and saving
- Maintain portability across POSIX-compatible systems

### User Experience Goals

- Simple and intuitive keyboard-driven workflow
- Fast startup and low resource usage
- Real-time editing without noticeable latency
- Clear status information and feedback
- Minimal and distraction-free interface

### Learning Objectives

- Terminal programming using ANSI escape sequences
- Raw keyboard input handling
- Dynamic data structures in C
- File I/O and persistence
- Modular software architecture
- Memory management and debugging
- Event-driven application design

### Short-Term Goals

- Complete editor configuration system
- Add syntax highlighting support
- Implement search and replace
- Improve customization options
- Enhance editing workflows

### Long-Term Goals

- Support multiple file buffers
- Add Git integration
- Introduce plugin architecture
- Enable split-screen editing
- Provide project-level workflows
- Evolve Claw into a powerful terminal code editor

### Design Philosophy

- Lightweight over feature bloat
- Performance over complexity
- Keyboard-first interaction
- Modular and extensible architecture
- Clear and maintainable code
- Educational and open-source friendly development

---

## Project Structure

```text
claw/

├── include/
│   ├── buffer.h
│   ├── command.h
│   ├── config.h
│   ├── cursor.h
│   ├── editor.h
│   ├── fileio.h
│   ├── history.h
│   ├── input.h
│   ├── keymap.h
│   ├── rawmode.h
│   ├── render.h
│   ├── search.h
│   ├── statusbar.h
│   ├── utils.h
│   └── viewport.h
│
├── src/
│
│   ├── buffer/
│   │   ├── buffer.c
│   │   ├── cursor.c
│   │   └── row.c
│   │
│   ├── command/
│   │   └── command.c
│   │
│   ├── config/
│   │   └── config.c
│   │
│   ├── editor/
│   │   └── editor.c
│   │
│   ├── fileio/
│   │   └── fileio.c
│   │
│   ├── history/
│   │   └── history.c
│   │
│   ├── input/
│   │   ├── input.c
│   │   └── rawmode.c
│   │
│   ├── keymap/
│   │   ├── keymap.c
│   │   └── parser.c
│   │
│   ├── render/
│   │   └── render.c
│   │
│   ├── search/
│   │   └── search.c
│   │
│   ├── ui/
│   │   └── statusbar.c
│   │
│   ├── utils/
│   │   └── string_utils.c
│   │
│   ├── viewport/
│   │   └── viewport.c
│   │
│   └── main.c
│
├── config/
│   ├── default.conf
│   └── claw.conf
│
├── keymaps/
│   ├── default.conf
│   ├── linus.conf
│   ├── mac.conf
│   └── windows.conf
│
├── tests/
│   ├── buffer_test.c
│   └── file_test.c
│
├── docs/
│   ├── README.md
│   ├── ARCHITECTURE.md
│   └── ROADMAP.md
│
├── Makefile
├── LICENSE
└── .gitignore
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
│   ├── buffer.c           Text insertion, deletion, and buffer management
│   ├── row.c              Row insertion, deletion, splitting, and merging
│   └── cursor.c           Cursor movement and position management
│
├── config/
│   └── config.c           Configuration file parser and settings management
│
├── command/
│   └── command.c          Command dispatch from key bindings to editor actions
│
├── keymap/
│   ├── keymap.c           Key binding table, defaults, and lookup
│   └── parser.c           Keymap file parser and key string decoder
│
├── render/
│   └── render.c           Screen rendering, line numbers, and highlighting
│
├── fileio/
│   └── fileio.c           File loading, saving, and persistence
│
├── viewport/
│   └── viewport.c         Scrolling, viewport tracking, and gutter sizing
│
├── search/
│   └── search.c           Search prompt, match detection, and navigation
│
├── history/
│   └── history.c          Undo/Redo system and edit history management
│
├── utils/
│   └── string_utils.c     Shared string manipulation and parsing helpers
│
└── ui/
    └── statusbar.c        Status bar and editor information display
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
↑              Move Cursor Up

↓              Move Cursor Down

←              Move Cursor Left

→              Move Cursor Right
```

### Editing

```text
Printable Keys  -> Insert Character

Tab             -> Insert Spaces (configurable width)

Backspace       -> Delete Character Left

Delete          -> Delete Character Right

Enter           -> Create New Line
```

### Search

```text
Ctrl + F        -> Open Search

Enter           -> Jump To Current Match

Ctrl + N        -> Next Match

Ctrl + P        -> Previous Match

Esc             -> Close Search
```

### Undo / Redo

```text
Ctrl + Z        -> Undo Last Action

Ctrl + Y        -> Redo Last Action
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
                    ┌─────────────────┐
                    │    Terminal     │
                    │     (User)      │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │  Input Engine   │
                    │    input.c      │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │  Keymap Engine  │
                    │   keymap.c      │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │ Command System  │
                    │   command.c     │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │   Editor Core   │
                    │    editor.c     │
                    └────────┬────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼

┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│  Text Buffer    │  │ Search Engine   │  │ History Engine  │
│   buffer.c      │  │   search.c      │  │   history.c     │
└────────┬────────┘  └────────┬────────┘  └────────┬────────┘
         │                    │                    │
         └──────────┬─────────┴─────────┬──────────┘
                    │                   │
                    ▼                   ▼

           ┌─────────────────┐  ┌─────────────────┐
           │ Viewport Engine │  │  File I/O Layer │
           │  viewport.c     │  │    fileio.c     │
           └────────┬────────┘  └─────────────────┘
                    │
                    ▼

           ┌─────────────────┐
           │ Rendering Engine│
           │    render.c     │
           └────────┬────────┘
                    │
                    ▼

           ┌─────────────────┐
           │   Status Bar    │
           │  statusbar.c    │
           └─────────────────┘

           ┌─────────────────┐
           │  Config Engine  │
           │   config.c      │
           └─────────────────┘
```

### Component Responsibilities

#### Editor Core (`editor.c`)

- Main application loop
- Subsystem initialization
- Configuration and keymap loading
- Subsystem coordination

#### Input Engine (`input.c`, `rawmode.c`)

- Raw terminal input
- Key decoding
- Escape sequence handling
- Terminal mode management

#### Keymap Engine (`keymap.c`, `parser.c`)

- Key-to-action binding table
- Default binding registration
- Keymap file loading and parsing
- Key code lookup

#### Command System (`command.c`)

- Action-to-function dispatch
- Central command routing
- Editor function invocation

#### Config Engine (`config.c`)

- INI-style configuration file parser
- Settings defaults and typed accessors
- XDG and local config file loading
- Layered configuration (defaults → user overrides)

#### Text Buffer (`buffer.c`, `row.c`, `cursor.c`)

- Document storage
- Character insertion and deletion
- Multi-line editing
- Cursor movement and tracking

#### Search Engine (`search.c`)

- Search prompt handling
- Match discovery
- Search navigation
- Result highlighting

#### History Engine (`history.c`)

- Undo operations
- Redo operations
- Action recording
- Edit history management

#### Viewport Engine (`viewport.c`)

- Vertical scrolling
- Horizontal scrolling
- Cursor visibility tracking
- Window size and gutter management

#### File I/O Layer (`fileio.c`)

- File loading
- File saving
- Filename tracking
- Persistence management

#### Rendering Engine (`render.c`)

- Screen drawing
- Welcome screen rendering
- Line number gutter rendering
- Search highlighting
- Cursor positioning

#### Status Bar (`statusbar.c`)

- File information display
- Modified state indicator
- Line and column tracking
- Editor status messages

#### Utility Layer (`string_utils.c`)

- String trimming
- Boolean parsing
- Shared helper functions

---

## Development Roadmap

### Milestone 1 — Editor Foundation
**Status:** Complete

- Project structure setup
- Modular source organization
- Main editor loop
- Basic screen rendering
- Initial build system

---

### Milestone 2 — Terminal Input System
**Status:** Complete

- Raw terminal mode
- Keyboard input handling
- Arrow key support
- Escape sequence processing
- Terminal restoration on exit

---

### Milestone 3 — Text Buffer Engine
**Status:** Complete

- Dynamic text buffer
- Character insertion
- Character deletion
- Memory management
- Row storage system

---

### Milestone 4 — Cursor Navigation
**Status:** Complete

- Horizontal navigation
- Vertical navigation
- Boundary handling
- Cursor tracking
- Position management

---

### Milestone 5 — Multi-Line Editing
**Status:** Complete

- New line creation
- Line splitting
- Line merging
- Multi-line navigation
- Dynamic row allocation

---

### Milestone 6 — File Management
**Status:** Complete

- File loading
- File saving
- Filename tracking
- Persistent storage
- Modified-state tracking

---

### Milestone 7 — Viewport & Scrolling
**Status:** Complete

- Vertical scrolling
- Horizontal scrolling
- Cursor-aware viewport
- Dynamic window sizing
- Large file navigation

---

### Milestone 8 — Search System
**Status:** Complete

- Search prompt
- Search input handling
- Match discovery
- Search highlighting
- Next match navigation
- Previous match navigation
- Cursor jump to result

---

### Milestone 9 — Undo / Redo System
**Status:** Complete

- Undo support
- Redo support
- Edit history tracking
- Multi-line undo
- Multi-line redo
- Action recording control

---

### Milestone 10 — Configuration System
**Status:** Complete

- INI-style configuration file parser
- Custom key binding system with keymap files
- Configurable tab width
- Toggleable line number display
- Toggleable welcome screen
- Toggleable status bar
- XDG config loading with local fallback
- Layered settings (defaults → user overrides)
- Command dispatch architecture
- Tab key insertion support

---

### Milestone 11 — Syntax Highlighting
**Status:** Next

- C language highlighting
- Keyword detection
- String highlighting
- Number highlighting
- Comment highlighting
- File type detection

---

### Milestone 12 — Search & Replace
**Status:** Planned

- Replace current match
- Replace all matches
- Case-sensitive search
- Whole-word matching
- Search statistics

---

### Milestone 13 — Git Integration
**Status:** Planned

- Repository detection
- Branch display
- File status indicators
- Commit integration
- Diff visualization

---

### Milestone 14 — Plugin System
**Status:** Planned

- Plugin API
- Dynamic module loading
- Custom editor commands
- Third-party extensions
- Plugin configuration
i
```

---

## Implemented Features

### Editor Configuration

- INI-style configuration file parser
- User config support with XDG and local fallback
- Configurable tab width (1–16 spaces)
- Toggleable line number display
- Toggleable welcome screen
- Toggleable status bar
- Startup preference loading
- Layered settings (defaults → user overrides)
- Custom key binding system
- Keymap file support with multiple profiles
- Command dispatch from key bindings to actions
- Tab key insertion support

### Character Editing

- Character insertion at cursor position
- Backspace deletion
- Forward deletion using Delete key
- Real-time text editing
- Dynamic row resizing
- Cursor-aware editing operations

### Cursor Navigation

- Horizontal cursor movement
- Vertical cursor movement
- Boundary-aware navigation
- Position tracking
- Automatic cursor correction

### Multi-Line Editing

- New line creation
- Line splitting at cursor position
- Line merging with Backspace
- Dynamic row creation
- Dynamic row removal
- Multi-line document support

### Text Buffer Management

- Dynamic memory allocation
- Row-based document storage
- Automatic buffer growth
- Efficient text manipulation
- Modification tracking

### File Operations

- Open existing files
- Create new files
- Save files to disk
- Filename tracking
- Unsaved change detection
- Exit confirmation protection
- Persistent document storage

### Viewport & Scrolling

- Vertical scrolling
- Horizontal scrolling
- Cursor-aware viewport tracking
- Dynamic viewport updates
- Large file navigation support
- Terminal resize handling

### Search System

- Search prompt (Ctrl + F)
- Search query input
- Document-wide match detection
- Search result highlighting
- Jump to matching text
- Next match navigation (Ctrl + N)
- Previous match navigation (Ctrl + P)
- Search cancellation
- Search state management

### Undo / Redo System

- Undo character insertions
- Undo character deletions
- Undo line splits
- Redo character insertions
- Redo character deletions
- Redo line splits
- Multi-line history support
- Edit action recording
- Separate undo and redo stacks
- History state management

### Rendering Engine

- Full-screen terminal rendering
- Real-time screen refresh
- Cursor rendering
- Line number gutter rendering
- Search result highlighting
- Welcome screen display
- Dynamic viewport rendering
- ANSI escape sequence rendering

### Status Bar

- Current filename display
- Modified file indicator
- Total line count
- Current line number display
- Current column number display
- Editor status messages

### User Interface

- Welcome screen
- Status bar
- Search prompt
- File information display
- Cursor position display
- Unsaved changes warning
- Context-sensitive status messages

### Terminal Support

- Raw terminal mode
- Real-time keyboard input
- Arrow key processing
- Control key shortcuts
- ANSI terminal compatibility
- POSIX terminal support
- Safe terminal restoration on exit

---

## Development Progress

| Version | Status | Description |
|----------|----------|----------|
| v0.1 | Complete | Project setup, modular architecture, and editor framework |
| v0.2 | Complete | Raw terminal mode and real-time keyboard input handling |
| v0.3 | Complete | Dynamic text buffer, character insertion, and deletion |
| v0.4 | Complete | Multi-line editing and advanced cursor navigation |
| v0.5 | Complete | File loading, saving, filename tracking, and persistence |
| v0.6 | Complete | Status bar, modification tracking, and file information display |
| v0.7 | Complete | Viewport engine, vertical scrolling, and horizontal scrolling |
| v0.8 | Complete | Search system with highlighting, match navigation, and cursor jump |
| v0.9 | Complete | Full Undo/Redo system with edit history and multi-line support |
| v1.0 | Complete | Configuration system, editor settings, and custom key bindings |
| v1.1 | Next | Syntax highlighting and language-aware rendering |
| v1.2 | Planned | Search and replace functionality |
| v1.3 | Planned | Git integration and repository awareness |
| v1.4 | Planned | Plugin system and editor extensibility |
| v1.5 | Planned | Advanced editing tools and productivity enhancements |
| v2.0 | Vision | Multi-buffer editing, split views, and IDE-like capabilities |

---

## License

MIT License

---

## Author

Aryan Gupta & Pranav Chauhan 

Project: Claw Text Editor
Language: C
Platform: Linux
