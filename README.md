# Claw

A lightweight terminal text editor written entirely in C.

Claw is being developed as part of a larger project to build a Linux-based operating system from scratch. The goal is to create a fast, modular, and extensible text editor while remaining simple, lightweight, and easy to understand.

---

## Current Status

Development Stage: Feature-Complete Editor Core

Current Version:

```text
v1.4
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
- Save-As prompt for unnamed buffers
- Automatic filename assignment after first save
- Syntax highlighting re-detection on save

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
- Match counting
- Current match tracking
- Search statistics

### Search & Replace System
- Replace prompt (Ctrl + R)
- Replace current match
- Replace all matches
- Automatic jump to next match after replacement
- Match counting
- Search statistics
- Replacement statistics
- Current match tracking
- Replace current occurrence
- Replace all occurrences in document
- Search result persistence after replacement

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

### Syntax Highlighting
- C language syntax highlighting
- Keyword highlighting
- Type keyword highlighting
- Number highlighting
- String highlighting
- Character literal highlighting
- Single-line comment highlighting
- Multi-line comment highlighting
- Preprocessor directive highlighting
- Operator highlighting
- File extension detection
- Language detection
- Lazy highlight caching
- ANSI color rendering

### Rendering
- Full-screen terminal rendering
- Cursor rendering
- Real-time screen refresh
- Status bar rendering
- Search match highlighting
- Syntax highlighting rendering
- Welcome screen rendering
- Filename display
- Modified file indicator
- Line and column display

### User Interface
- Welcome screen
- Status bar
- Search prompt
- Replace prompt
- Save-As filename prompt
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

### Git Integration
- Git repository detection
- Repository root discovery
- Current branch detection
- Branch display in status bar
- Repository-aware startup
- Git status parsing
- Modified file detection
- Staged file detection
- Untracked file detection
- Git Add integration
- Git Restore integration
- Git Blame integration
- Temporary Git view support
- Return-to-file workflow
- Git command execution layer
- Repository metadata refresh

## In Progress

### v1.5 — Plugin System

- Plugin loading framework
- Dynamic module support
- Plugin API design
- Editor event hooks
- Custom command extensions
- Runtime plugin discovery
- Plugin lifecycle management
- Plugin configuration support
- Extension sandbox architecture
- Third-party plugin support

### Current Focus

- Designing plugin architecture
- Building plugin loading engine
- Defining plugin API interfaces
- Creating editor event hook system
- Planning command extension framework
- Preparing runtime module discovery
- Designing plugin configuration format
- Improving editor extensibility

## Planned Features

### v1.6 — Advanced Editing

- Auto indentation
- Smart indentation
- Tab and space conversion
- Duplicate line command
- Move line up/down
- Line selection tools
- Bracket matching
- Auto-closing brackets
- Comment/uncomment selection
- Line join/split operations

### v1.7 — Productivity Features

- Multiple cursors
- Bookmarks
- Recent files list
- Command palette
- Go to line
- File explorer
- Quick file switching
- Symbol navigation
- Session restoration
- Workspace support

### v1.8 — Customization & Themes

- Theme engine
- Color scheme support
- User-defined themes
- Syntax theme customization
- Layout customization
- Status bar customization
- Editor appearance profiles

### v1.9 — Project Tools

- Project workspace management
- Project-wide search
- Project-wide replace
- Build command integration
- Task runner support
- Project configuration files
- Workspace indexing
- Project navigation tools

### v2.0 — IDE Features

- Multiple file buffers
- Split-screen editing
- Mouse support
- Integrated terminal
- Language Server Protocol (LSP) support
- Code completion
- Diagnostics and error highlighting
- Symbol indexing
- Code navigation
- Refactoring support

### v2.1 — Git Workflow Expansion

- Commit message prompt
- Git diff viewer
- Git log viewer
- Git branch switching
- Git stash support
- Inline change indicators
- Repository dashboard

### v2.2 — Language Support Expansion

- Python syntax highlighting
- Java syntax highlighting
- JavaScript syntax highlighting
- TypeScript syntax highlighting
- Rust syntax highlighting
- Go syntax highlighting
- Markdown support
- JSON support

### v3.0 — Modern Editor Platform

- Plugin marketplace
- Remote file editing
- Collaborative editing
- Integrated debugger
- AI-assisted editing
- Project templates
- Cross-platform packaging
- Lightweight IDE workflow

## Long-Term Vision

- Build a modern terminal-first code editor in C
- Remain lightweight, fast, and keyboard-driven
- Support extensibility through plugins
- Provide powerful project and Git workflows
- Offer advanced code navigation and editing tools
- Integrate Language Server Protocol (LSP) features
- Enable workspace-based development
- Maintain cross-platform compatibility
- Preserve low memory usage and fast startup times
- Evolve Claw into a lightweight IDE without sacrificing simplicity

---

## Project Goals

### Primary Goals

- Build a lightweight terminal-based text editor in C
- Create a fast and responsive editing experience
- Maintain a modular and maintainable codebase
- Provide essential developer-focused editing tools
- Keep the editor simple, extensible, and efficient

### Technical Goals

- Implement efficient text editing and navigation
- Maintain low memory usage and fast startup times
- Support configurable editor behavior through settings and keymaps
- Expand syntax highlighting for multiple programming languages
- Build a robust plugin architecture for future extensibility
- Improve Git integration and repository workflows
- Preserve portability across POSIX-compatible systems

### User Experience Goals

- Keyboard-first workflow
- Minimal and distraction-free interface
- Clear visual feedback and status information
- Consistent and intuitive shortcuts
- Reliable editing of large files
- Smooth search, replace, and navigation experience
- Easy customization without complexity

### Learning Objectives

- Systems programming in C
- Terminal and ANSI escape sequence programming
- Raw keyboard input processing
- Dynamic memory management
- Modular software architecture
- Data structure design
- File system and persistence management
- Version control integration
- Plugin architecture design

### Short-Term Goals

- Complete Plugin System (v1.5)
- Add runtime extension loading
- Create editor event hooks
- Define public plugin APIs
- Expand syntax highlighting language support
- Improve Git workflow features
- Add diff visualization support

### Long-Term Goals

- Support multiple file buffers
- Introduce split-screen editing
- Build a full plugin ecosystem
- Implement Language Server Protocol (LSP) support
- Add project-wide development tools
- Create workspace management features
- Develop integrated terminal support
- Provide IDE-like capabilities while remaining lightweight

### Design Philosophy

- Lightweight over feature bloat
- Performance over complexity
- Keyboard-first interaction
- Modular and extensible architecture
- Clear and maintainable code
- Educational and open-source friendly development
- Fast startup and low memory usage
- Features should enhance productivity without sacrificing simplicity

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
│   ├── git.h
│   ├── highlight.h
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
│   ├── git/
│   │   └── git.c
│   │
│   ├── highlight/
│   │   └── highlight.c
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
│   ├── file_test.c
│   └── search_test.c
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
│   └── render.c           Screen rendering, line numbers, syntax highlighting,
│                          search highlighting, and welcome screen
│
├── highlight/
│   └── highlight.c        Syntax highlighting engine, language definitions,
│                          color rules, and file type detection
│
├── fileio/
│   └── fileio.c           File loading, saving, and persistence
│
├── viewport/
│   └── viewport.c         Scrolling, viewport tracking, and gutter sizing
│
├── search/
│   └── search.c           Search prompt, match discovery, match navigation,
│                          replace current match, replace next match,
│                          and replace all matches
│
├── history/
│   └── history.c          Undo/Redo system and edit history management
│
├── utils/
│   └── string_utils.c     Shared string manipulation and parsing helpers
│
└── ui/
│   └── statusbar.c        Status bar and editor information display
│
├── git/
│   └── git.c              Git repository detection, status parsing,
│                          branch discovery, Git Add, Restore,
│                          Blame integration, and repository awareness
```

---

## Build Instructions

### Requirements

- GCC or Clang
- GNU Make
- POSIX-compatible terminal
- Linux or macOS

### Clone Repository

```bash
git clone <repository-url>
cd claw
```

### Compile

```bash
make
```

This generates the editor executable:

```text
claw
```

### Run Empty Editor

```bash
./claw
```

or if installed globally:

```bash
claw
```

### Open Existing File

```bash
./claw notes.txt
```

or

```bash
claw notes.txt
```

### Create New File

```bash
./claw newfile.txt
```

If the file does not exist, Claw creates it when saved.

### Save Changes

```text
Ctrl + S
```

If no filename has been set (opened with `./claw`), a Save-As prompt appears:

```text
Save as: _
```

Type the desired filename and press Enter. The file is saved and subsequent
Ctrl+S saves directly without prompting. Press Escape to cancel.

### Search

```text
Ctrl + F
```

### Navigate Search Results

```text
Ctrl + N    Next Match
Ctrl + P    Previous Match
```

### Replace Current Match

```text
Ctrl + R
Enter replacement text
Press Enter
```

The current match is replaced and the cursor automatically moves to the next match.

### Replace All Matches

```text
Ctrl + R
Enter replacement text
Ctrl + A
```

Status message example:

```text
All matches replaced
```

### Undo / Redo

```text
Ctrl + Z    Undo
Ctrl + Y    Redo
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

### Rebuild Project

```bash
make clean
make
```

or

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

Expected output:

```text
/usr/local/bin/claw
```

Run from anywhere:

```bash
claw
```

Open a file from anywhere:

```bash
claw notes.txt
```

### Uninstall Global Command

```bash
sudo rm /usr/local/bin/claw
```

---

## Current Controls

### Navigation

```text
↑              -> Move Cursor Up
↓              -> Move Cursor Down
←              -> Move Cursor Left
→              -> Move Cursor Right
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

### Replace

```text
Ctrl + R        -> Open Replace Prompt

Enter           -> Replace Current Match

Enter           -> Replace Current Match And Jump To Next Match

Ctrl + A        -> Replace All Matches

Esc             -> Cancel Replace
```

### Undo / Redo

```text
Ctrl + Z        -> Undo Last Action

Ctrl + Y        -> Redo Last Action
```

### File Operations

```text
Ctrl + S        -> Save File

                   (prompts for filename if unnamed)

Esc             -> Cancel Save-As Prompt

Ctrl + X        -> Quit Editor

Ctrl + X twice  -> Force Quit

                  (when unsaved changes exist)
```

### Git Integration

```text
Ctrl + G        -> Git Add Current File

Ctrl + U        -> Git Unstage Current File

Ctrl + T        -> Git Restore Current File

Ctrl + B        -> Open Git Blame View

Esc             -> Exit Git View
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

Language        -> Detected Syntax Language

Lines           -> Total Line Count

Ln              -> Current Line Number

Col             -> Current Column Number

Search Info     -> Match Count And Navigation Status

Replace Info    -> Replacement Status Messages
```

### Git View

```text
Git Blame View  -> Line Ownership Information

Author          -> Commit Author

Commit Hash     -> Commit Reference

Commit Date     -> Last Modification Date

Esc             -> Return To Editor
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
     ┌───────────────────┼───────────────────┬───────────────────┬───────────────────┐
     │                   │                   │                   │                   │
     ▼                   ▼                   ▼                   ▼                   ▼

┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│  Text Buffer    │ │ Search Engine   │ │ History Engine  │ │ Highlight Engine│ │   Git Engine    │
│   buffer.c      │ │   search.c      │ │   history.c     │ │  highlight.c    │ │     git.c       │
└────────┬────────┘ └────────┬────────┘ └────────┬────────┘ └────────┬────────┘ └────────┬────────┘
         │                   │                   │                   │                   │
         └──────────┬────────┴─────────┬─────────┴─────────┬─────────┴─────────┬─────────┘
                    │                  │                   │                   │
                    ▼                  ▼                   ▼                   ▼

          ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
          │ Viewport Engine │ │  File I/O Layer │ │  Config Engine  │ │ Git Integration │
          │  viewport.c     │ │    fileio.c     │ │   config.c      │ │ Git Commands    │
          └────────┬────────┘ └─────────────────┘ └─────────────────┘ │ Git Status      │
                   │                                                  │ Git Blame       │
                   ▼                                                  │ Git Restore     │
          ┌─────────────────┐                                         └────────┬────────┘
          │ Rendering Engine│                                                  │
          │    render.c     │                                                  │
          └────────┬────────┘                                                  │
                   │                                                           │
                   ▼                                                           ▼
          ┌─────────────────┐                                         ┌─────────────────┐
          │   Status Bar    │                                         │ Git Views / UI  │
          │  statusbar.c    │                                         │ Blame Viewer    │
          └─────────────────┘                                         └─────────────────┘
```

## Component Responsibilities

### Editor Core (`editor.c`)

- Main application loop
- Subsystem initialization
- Configuration and keymap loading
- Editor lifecycle management
- Save-As prompt handling
- Search and replace workflow management
- Git view state management
- Coordination between editor subsystems

### Input Engine (`input.c`, `rawmode.c`)

- Raw terminal input processing
- Keyboard event decoding
- Escape sequence handling
- Control key processing
- Terminal mode management

### Keymap Engine (`keymap.c`, `parser.c`)

- Key-to-command binding management
- Default keymap registration
- Keymap file parsing
- Custom shortcut support
- Key lookup and dispatch preparation

### Command System (`command.c`)

- Command dispatch architecture
- Action routing
- Editor command execution
- Keymap integration
- Centralized editor actions

### Config Engine (`config.c`)

- INI-style configuration parsing
- User preference loading
- XDG configuration support
- Default settings management
- Layered configuration overrides

### Text Buffer (`buffer.c`, `row.c`, `cursor.c`)

- Document storage
- Character insertion
- Character deletion
- Line insertion and removal
- Line splitting and merging
- Cursor movement and tracking
- Dynamic memory management

### Search Engine (`search.c`)

- Search prompt handling
- Match discovery
- Match counting
- Search navigation
- Current match tracking
- Search result highlighting
- Replace current match
- Replace next match
- Replace all matches
- Search statistics and status reporting

### History Engine (`history.c`)

- Undo operations
- Redo operations
- Edit action recording
- Multi-line history tracking
- Undo stack management
- Redo stack management

### Highlight Engine (`highlight.c`)

- Syntax highlighting
- Language detection
- File type recognition
- Keyword highlighting
- String highlighting
- Number highlighting
- Comment highlighting
- Highlight cache management
- ANSI color generation

### Git Engine (`git.c`)

- Git repository detection
- Repository root discovery
- Current branch identification
- Git status parsing
- Modified file detection
- Staged file detection
- Untracked file detection
- Git add integration
- Git unstage integration
- Git restore integration
- Git blame integration
- Repository metadata caching
- Git command execution

### Viewport Engine (`viewport.c`)

- Vertical scrolling
- Horizontal scrolling
- Cursor visibility management
- Viewport tracking
- Window size adaptation
- Line number gutter calculation

### File I/O Layer (`fileio.c`)

- File loading
- File saving
- New file creation
- Filename tracking
- Persistence management
- Unsaved change detection

### Rendering Engine (`render.c`)

- Full-screen rendering
- Cursor rendering
- Welcome screen rendering
- Line number rendering
- Syntax highlighting rendering
- Search match highlighting
- Git information rendering
- Status message rendering
- ANSI escape sequence output

### Status Bar (`statusbar.c`)

- Current filename display
- Modified state indicator
- Language display
- Line and column tracking
- Git branch display
- Git repository status display
- Search status messages
- Replace status messages
- Editor feedback messages

### Utility Layer (`string_utils.c`)

- String manipulation helpers
- Configuration parsing helpers
- Boolean conversion utilities
- Shared reusable functions

### Git View System

- Git blame viewer
- Temporary Git file rendering
- Repository information display
- Git operation feedback
- Return-to-editor workflow
- Git view lifecycle management

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
- Match count display
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

- INI-style configuration parser
- Custom key binding system
- Keymap file support
- Configurable tab width
- Toggleable line numbers
- Toggleable welcome screen
- Toggleable status bar
- XDG configuration support
- Layered settings architecture

---

### Milestone 11 — Syntax Highlighting

**Status:** Complete

- C language syntax highlighting
- Keyword highlighting
- Type keyword highlighting
- String highlighting
- Character literal highlighting
- Number highlighting
- Single-line comment highlighting
- Multi-line comment highlighting
- Preprocessor directive highlighting
- Operator highlighting
- File extension detection
- Language name display
- Highlight caching
- ANSI color theme support

---

### Milestone 12 — Search & Replace

**Status:** Complete

- Replace current match
- Replace next match
- Replace all matches
- Match count reporting
- Search result highlighting
- Current match tracking
- Search navigation integration
- Replacement status messages
- Automatic jump to next match after replacement

---

### Milestone 13 — Save-As Prompt

**Status:** Complete

- Interactive filename prompt for unnamed buffers
- Save-As triggered on Ctrl+S when no filename is set
- Escape to cancel save
- Automatic filename assignment after successful save
- Syntax highlighting re-detection from new filename extension
- Empty filename validation

---

### Milestone 14 — Git Integration

**Status:** Complete

- Git repository detection
- Repository root discovery
- Current branch display
- Modified file detection
- Staged file detection
- Untracked file detection
- Git add support
- Git unstage support
- Git restore support
- Git blame integration
- Repository-aware status bar
- Git view management

---

### Milestone 15 — Plugin System

**Status:** Planned

- Plugin API
- Dynamic module loading
- Custom editor commands
- Third-party extensions
- Plugin configuration support

---

### Milestone 16 — Advanced Editing

**Status:** Planned

- Auto indentation
- Smart indentation
- Tab-to-space conversion
- Duplicate line command
- Move line up/down
- Bracket matching
- Auto-closing brackets
- Comment/uncomment support

---

### Milestone 17 — Productivity Features

**Status:** Planned

- Multiple cursors
- Bookmarks
- Recent files list
- Command palette
- Go to line
- File explorer
- Quick file switching
- Session restoration

---

### Milestone 18 — Themes & Customization

**Status:** Planned

- Theme engine
- Color scheme support
- User-defined themes
- Status bar customization
- Layout customization

---

### Milestone 19 — Project Tools

**Status:** Planned

- Project workspace management
- Project-wide search
- Project-wide replace
- Build command integration
- Task runner support
- Project configuration files

---

### Milestone 20 — IDE Features

**Status:** Planned

- Multiple file buffers
- Split-screen editing
- Mouse support
- Integrated terminal
- Language Server Protocol (LSP)
- Code completion
- Diagnostics and error highlighting
- Code navigation
- Refactoring support

---

## Implemented Features**

### Git Integration**

- Git repository detection
- Current branch identification
- Repository root discovery
- Repository-aware file tracking
- Git status integration
- Modified file detection
- Staged file detection
- Untracked file detection
- Git metadata refresh system
- Branch display in status bar

### Git Commands**

- Git Add (Ctrl + G)
- Git Restore (Ctrl + T)
- Git Blame View (Ctrl + B)
- Repository status refresh
- Git command execution layer
- Repository validation before execution
- Status-aware command handling
- Automatic Git state updates

### Git Blame Viewer**

- Inline Git blame generation
- Temporary blame file creation
- Read-only blame view
- Original file preservation
- Return to editor using Esc
- Repository-aware blame execution
- Safe blame view switching

### Editor Configuration**

- INI-style configuration file parser
- User configuration support (XDG and local fallback)
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

### Character Editing**

- Character insertion at cursor position
- Backspace deletion
- Forward deletion using Delete key
- Real-time text editing
- Dynamic row resizing
- Cursor-aware editing operations

### Cursor Navigation**

- Horizontal cursor movement
- Vertical cursor movement
- Boundary-aware navigation
- Position tracking
- Automatic cursor correction

### Multi-Line Editing**

- New line creation
- Line splitting at cursor position
- Line merging with Backspace
- Dynamic row creation
- Dynamic row removal
- Multi-line document support

### Text Buffer Management**

- Dynamic memory allocation
- Row-based document storage
- Automatic buffer growth
- Efficient text manipulation
- Modification tracking

### File Operations**

- Open existing files
- Create new files
- Save files to disk
- Save-As prompt for unnamed buffers
- Automatic filename assignment after first save
- Syntax highlighting re-detection on save
- Filename tracking
- Unsaved change detection
- Exit confirmation protection
- Persistent document storage

### Viewport & Scrolling**

- Vertical scrolling
- Horizontal scrolling
- Cursor-aware viewport tracking
- Dynamic viewport updates
- Large file navigation support
- Terminal resize handling

### Search System**

- Search prompt (Ctrl + F)
- Search query input
- Document-wide match detection
- Match count reporting
- Search result highlighting
- Jump to matching text
- Next match navigation (Ctrl + N)
- Previous match navigation (Ctrl + P)
- Search cancellation
- Search state management

### Search & Replace**

- Replace current match
- Automatic jump to next match after replacement
- Replace next match workflow
- Replace all matches
- Replacement status messages
- Match tracking across document
- Search and replace integration
- Document-wide replacement support

### Undo / Redo System**

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

### Rendering Engine**

- Full-screen terminal rendering
- Real-time screen refresh
- Cursor rendering
- Line number gutter rendering
- Search result highlighting
- Syntax highlighting rendering
- Welcome screen display
- Dynamic viewport rendering
- ANSI escape sequence rendering

### Status Bar**

- Current filename display
- Modified file indicator
- Total line count
- Current line number display
- Current column number display
- Detected language display
- Current Git branch display
- Git file status display
- Search status messages
- Replace status messages
- Editor feedback messages

### User Interface**

- Welcome screen
- Status bar
- Search prompt
- Replace prompt
- Save-As filename prompt
- Git status feedback messages
- Git blame viewer
- File information display
- Cursor position display
- Unsaved changes warning
- Context-sensitive status messages

### Terminal Support**

- Raw terminal mode
- Real-time keyboard input
- Arrow key processing
- Control key shortcuts
- ANSI terminal compatibility
- POSIX terminal support
- Safe terminal restoration on exit

### Syntax Highlighting**

- C language syntax highlighting (.c, .h, .cc, .cpp, .hpp)
- Keyword highlighting
- Type keyword highlighting
- String literal highlighting
- Character literal highlighting
- Number highlighting
- Single-line comment highlighting
- Multi-line comment highlighting
- Preprocessor directive highlighting
- Operator highlighting
- File type detection from filename extension
- Language name display in status bar
- Lazy per-row highlight caching
- ANSI 256-color dark theme support
- Extensible language definition architecture

---

**## Development Progress**

| Version | Status | Description |
|----------|----------|----------|
| v0.1 | Complete | Project setup, modular architecture, and editor framework |
| v0.2 | Complete | Raw terminal mode and real-time keyboard input handling |
| v0.3 | Complete | Dynamic text buffer, character insertion, and deletion |
| v0.4 | Complete | Multi-line editing and advanced cursor navigation |
| v0.5 | Complete | File loading, saving, filename tracking, and persistence |
| v0.6 | Complete | Status bar, modification tracking, and file information display |
| v0.7 | Complete | Viewport engine, vertical scrolling, horizontal scrolling, and large-file navigation |
| v0.8 | Complete | Search system with highlighting, match navigation, match counting, and cursor jump |
| v0.9 | Complete | Full Undo/Redo system with edit history and multi-line support |
| v1.0 | Complete | Configuration system, editor settings, custom key bindings, and command dispatch |
| v1.1 | Complete | Syntax highlighting, language-aware rendering, file type detection, and theme support |
| v1.2 | Complete | Search & Replace with match tracking, navigation, and bulk replacement |
| v1.3 | Complete | Save-As prompt for unnamed buffers with filename input and syntax re-detection |
| v1.4 | Complete | Git integration, repository awareness, and version-control workflows |
| v1.5 | Planned | Plugin system and editor extensibility |
| v1.6 | Planned | Advanced editing tools and productivity enhancements |
| v1.7 | Planned | Multiple cursors, bookmarks, command palette, and project navigation |
| v2.0 | Vision | Multi-buffer editing, split views, integrated terminal, LSP support, and IDE-like capabilities |

---

## License

MIT License

---

## Author

Aryan Gupta & Pranav Chauhan 

Project: Claw Text Editor
Language: C
Platform: Linux
