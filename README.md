# Claw

A lightweight terminal text editor written entirely in C.

Claw is being developed as part of a larger project to build a Linux-based operating system from scratch. The goal is to create a fast, modular, and extensible text editor while remaining simple, lightweight, and easy to understand.

---

## Current Status

Development Stage: Stable Plugin Platform

Current Version:

```text
v1.5
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
- Dynamic terminal resizing

### Search System
- Search prompt (Ctrl + F)
- Real-time search input
- Match discovery across document
- Search result highlighting
- Jump cursor to matched text
- Next match navigation (Ctrl + N)
- Previous match navigation (Ctrl + P)
- Search cancellation (Esc)
- Global search cancellation (Esc)
- Automatic search highlight clearing
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
- Plugin Dashboard rendering
- Plugin Manager rendering
- Filename display
- Modified file indicator
- Line and column display

### User Interface
- Welcome screen
- Status bar
- Search prompt
- Replace prompt
- Save-As filename prompt
- Plugin Dashboard
- Plugin Manager
- Plugin statistics display
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
- Alt key handling
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
- Git Unstage integration
- Git Blame integration
- Temporary Git view support
- Return-to-file workflow
- Git command execution layer
- Repository metadata refresh

### Plugin System

- Plugin API
- Runtime plugin loading
- Dynamic module support
- Plugin lifecycle management
- Plugin discovery system
- Plugin registration framework

- Startup hooks
- Save hooks
- Search hooks
- Exit hooks

- Custom command extensions
- Plugin configuration support
- Third-party plugin support

- Plugin enable/disable support
- Plugin autoload support
- Plugin event control support
- Plugin reload support

- Plugin statistics tracking
- Plugin health monitoring
- Plugin status reporting
- Plugin health score tracking

- Interactive Plugin Dashboard
- Plugin list view
- Plugin selection navigation
- Plugin Details View

- Plugin metadata display
- Plugin author information
- Plugin version information
- Plugin file information

- Loaded plugin counter
- Enabled plugin counter
- Disabled plugin counter

- Command execution statistics
- Startup hook statistics
- Save hook statistics
- Search hook statistics
- Exit hook statistics

- Dashboard keyboard controls
- Plugin health access
- Plugin reload controls
- Dashboard status messages

- Runtime plugin management
- Runtime plugin inspection
- Runtime plugin configuration

- Hello Plugin
- Statistics Plugin
- Format Plugin

## In Progress

### Current Focus — v1.6 Advanced Editing

- Auto indentation system
- Smart indentation engine
- Auto-closing brackets
- Bracket matching
- Tab-to-space conversion
- Duplicate line command
- Move line up/down
- Comment/Uncomment support
- Line join operation
- Line split operation
- Smart backspace handling
- Smart tab handling
- Indentation-aware new line creation
- Editing productivity shortcuts
- Improved cursor navigation
- Editing command framework

## Planned Features

### v1.7 — Productivity Features

- Multiple cursors
- Bookmarks
- Command palette
- Go To Line
- Recent files
- Quick file switching
- File explorer
- Symbol navigation
- Session restoration

### v1.8 — Themes & Customization

- Theme engine
- Color schemes
- User-defined themes
- Appearance profiles
- Layout customization
- Status bar customization
- Theme marketplace foundation

### v1.9 — Project Tools

- Workspace management
- Project-wide search
- Project-wide replace
- Build command integration
- Task runner support
- Project configuration files
- Workspace indexing

### v2.0 — IDE Features

- Multiple file buffers
- Split-screen editing
- Integrated terminal
- Mouse support
- Language Server Protocol (LSP)
- Code completion
- Diagnostics
- Code navigation
- Refactoring tools

### v2.1 — Git Workflow Expansion

- Git diff viewer
- Git log viewer
- Commit message prompt
- Branch switching
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

### v4.0 — Claw IDE

- Complete workspace environment
- Advanced code intelligence
- Full project navigation
- Integrated development toolkit
- Modern terminal-first IDE experience
- Extensible developer ecosystem
- Professional developer workflow
- Fully customizable platform

## Long-Term Vision

- Build a modern terminal-first code editor written entirely in C
- Maintain a lightweight, fast, and keyboard-driven editing experience
- Provide a powerful plugin ecosystem for extensibility and customization
- Deliver advanced text editing, navigation, and productivity tools
- Offer seamless Git integration and project-aware development workflows
- Support multiple programming languages through extensible syntax engines
- Introduce workspace management and project-wide development features
- Integrate Language Server Protocol (LSP) capabilities for modern coding workflows
- Provide intelligent code navigation, diagnostics, and refactoring support
- Enable efficient development of large-scale software projects
- Preserve low memory usage, fast startup times, and high performance
- Maintain a modular and maintainable architecture for long-term growth
- Support cross-platform operation across Linux, macOS, and other POSIX systems
- Build a flexible foundation for future IDE-level capabilities
- Encourage community-driven plugin and extension development
- Keep the interface clean, distraction-free, and productivity-focused
- Balance powerful features with simplicity and ease of use
- Evolve Claw into a complete developer platform without sacrificing speed or reliability
- Remain open-source, educational, and accessible to systems programmers
- Establish Claw as a modern lightweight alternative to heavyweight editors and IDEs

---

## Project Goals

### Primary Goals
- Build a lightweight terminal-based text editor in C
- Deliver a fast, responsive, and reliable editing experience
- Maintain a modular and scalable codebase architecture
- Provide essential developer-focused editing features
- Keep the editor simple, efficient, and easy to understand
- Support extensibility through a robust plugin system
- Create a keyboard-first workflow optimized for productivity

### Technical Goals
- Implement efficient text buffer and memory management
- Maintain low memory usage and fast startup performance
- Support configurable editor behavior through settings and keymaps
- Provide a flexible command and input handling architecture
- Expand syntax highlighting support for multiple programming languages
- Build a stable runtime plugin framework for third-party extensions
- Improve Git integration and repository-aware workflows
- Support large-file editing with smooth scrolling and navigation
- Preserve portability across POSIX-compatible operating systems
- Maintain clean, maintainable, and well-documented source code

### User Experience Goals
- Provide a distraction-free editing environment
- Deliver consistent and intuitive keyboard shortcuts
- Offer clear visual feedback and status information
- Ensure smooth navigation through large documents
- Provide reliable search, replace, and editing workflows
- Support customization without introducing unnecessary complexity
- Keep interface interactions simple and predictable
- Enable efficient development workflows directly from the terminal

### Plugin Ecosystem Goals
- Support dynamic plugin loading and unloading
- Enable custom editor commands through plugins
- Provide event-driven plugin hooks
- Allow third-party feature extensions without modifying core code
- Maintain plugin health monitoring and diagnostics
- Create a stable public plugin API
- Encourage community-driven plugin development

### Git Workflow Goals
- Provide seamless integration with Git repositories
- Support common version-control operations from inside the editor
- Improve repository awareness and status tracking
- Expand blame, diff, and history viewing capabilities
- Reduce context switching between editor and terminal tools

### Learning Objectives
- Systems programming in C
- Terminal and ANSI escape sequence programming
- Raw keyboard input processing
- Dynamic memory management
- Data structure and algorithm implementation
- Modular software architecture design
- File system interaction and persistence management
- Plugin architecture development
- Git integration and tooling
- Large-scale software project organization

### Long-Term Goals
- Support multiple file buffers and split-screen editing
- Implement Language Server Protocol (LSP) integration
- Provide intelligent code navigation and diagnostics
- Add project-wide development and workspace tools
- Build a complete plugin ecosystem
- Support multiple programming languages
- Introduce integrated terminal capabilities
- Deliver IDE-like functionality while remaining lightweight
- Establish Claw as a modern terminal-first development platform
- Continue prioritizing performance, simplicity, and extensibility

### Design Philosophy

- Lightweight over feature bloat
- Performance over unnecessary complexity
- Keyboard-first interaction and navigation
- Fast startup and low memory consumption
- Simplicity without sacrificing capability
- Modular and maintainable architecture
- Extensibility through plugins and configuration
- Clear separation of editor components
- Reliable and predictable editor behavior
- Terminal-first development experience
- Developer productivity through efficient workflows
- Native C implementation with minimal dependencies
- Clean, readable, and well-documented source code
- Cross-platform compatibility across POSIX systems
- User customization without overwhelming configuration
- Practical features driven by real development needs
- Stability and correctness before feature expansion
- Educational and open-source friendly development
- Git-aware and project-oriented workflows
- Continuous improvement through incremental releases
- Preserve the philosophy of doing more with less
- Evolve into a powerful developer platform while remaining lightweight

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
│   ├── plugin.h
│   ├── plugin_api.h
│   ├── plugin_commands.h
│   ├── rawmode.h
│   ├── render.h
│   ├── search.h
│   ├── statusbar.h
│   ├── utils.h
│   └── viewport.h
│
├── src/
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
│   ├── plugin/
│   │   ├── plugin.c
│   │   ├── plugin_loader.c
│   │   ├── plugin_manager.c
│   │   └── plugin_commands.c
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
├── plugins/
│   ├── hello_plugin.c
│   ├── stats_plugin.c
│   └── format_plugin.c
│
├── config/
│   ├── default.conf
│   └── claw.conf
│
├── keymaps/
│   ├── default.conf
│   ├── linux.conf
│   ├── mac.conf
│   └── windows.conf
│
├── tests/
│   ├── buffer_test.c
│   ├── file_test.c
│   ├── search_test.c
│   └── plugin_test.c
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
├── main.c                          Entry point and application startup
│
├── editor/
│   └── editor.c                    Core editor loop, modal handling, and command dispatch
│
├── input/
│   ├── input.c                     Keyboard input processing and key decoding
│   └── rawmode.c                   Terminal raw mode management
│
├── buffer/
│   ├── buffer.c                    Text insertion, deletion, and buffer management
│   ├── row.c                       Row insertion, deletion, splitting, and merging
│   └── cursor.c                    Cursor movement and position management
│
├── config/
│   └── config.c                    Configuration parser and settings management
│
├── command/
│   └── command.c                   Command execution and editor action routing
│
├── keymap/
│   ├── keymap.c                    Key binding registration and lookup
│   └── parser.c                    Keymap file parser and shortcut decoding
│
├── render/
│   └── render.c                    Screen rendering, syntax colors, UI views, and status display
│
├── highlight/
│   └── highlight.c                 Syntax highlighting engine and language definitions
│
├── fileio/
│   └── fileio.c                    File loading, saving, and persistence
│
├── viewport/
│   └── viewport.c                  Scrolling, viewport tracking, and terminal resizing
│
├── search/
│   └── search.c                    Search, replace, navigation, and match tracking
│
├── history/
│   └── history.c                   Undo/Redo system and edit history management
│
├── git/
│   └── git.c                       Git status, blame, add, restore, unstage, and repository awareness
│
├── plugin/
│   ├── plugin.c                    Plugin runtime and lifecycle management
│   ├── plugin_loader.c             Dynamic plugin loading and unloading
│   ├── plugin_manager.c            Plugin dashboard and management logic
│   └── plugin_commands.c           Plugin-related editor commands
│
├── ui/
│   └── statusbar.c                 Status bar and editor information display
│
├── utils/
│   └── string_utils.c              Shared helper and string utility functions
│
└── plugins/
    ├── hello_plugin.c              Example plugin
    ├── stats_plugin.c              Statistics plugin
    └── format_plugin.c             Formatting plugin
```

---

## Build Instructions

### Requirements
- GCC or Clang
- GNU Make
- POSIX-compatible terminal
- Linux or macOS
- Git (required for Git integration features)
- dlopen-compatible system libraries (for runtime plugins)

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

### Debug Build

```bash
make debug
```

Builds Claw with debugging symbols and additional compiler warnings.

### Run Empty Editor

```bash
./claw
```

or

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

If no filename is assigned, a Save-As prompt appears:

```text
Save as: _
```

Type a filename and press Enter.

```text
Esc -> Cancel Save
```

### Search

```text
Ctrl + F
```

### Navigate Search Results

```text
Ctrl + N    Next Match
Ctrl + P    Previous Match
Esc         Cancel Search
```

### Search & Replace

```text
Ctrl + R    Open Replace Prompt
Enter       Replace Current Match
Ctrl + A    Replace All Matches
Esc         Cancel Replace
```

Example:

```text
Find:    hello
Replace: world
```

### Undo / Redo

```text
Ctrl + Z    Undo
Ctrl + Y    Redo
```

### Git Integration

```text
Ctrl + G    Git Add Current File
Ctrl + U    Git Unstage Current File
Ctrl + T    Git Restore Current File
Ctrl + B    Open Git Blame View
Esc         Exit Git View
```

### Plugin Manager

```text
Ctrl + K    Open Plugin Manager
↑ / ↓       Navigate Plugins
Enter       Open Plugin Details
Space       Enable / Disable Plugin
A           Toggle AutoLoad
E           Toggle Event Hooks
H           Run Health Check
R           Reload Plugins
Esc         Back / Exit
```

### Configuration Files

Default Configuration:

```text
config/default.conf
```

User Configuration:

```text
~/.config/claw/claw.conf
```

Supported settings include:

```text
tab_width
show_line_numbers
show_status_bar
show_welcome_screen
keymap
```

### Keymap Profiles

Available keymaps:

```text
default
linux
mac
windows
```

Example:

```ini
keymap=mac
```

### Syntax Highlighting

Currently Supported:

```text
C
C++
Header Files
```

Additional language support is planned for future releases.

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
make rebuild
```

or

```bash
make clean
make
```

### Run Tests

```bash
make test
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

Open a file:

```bash
claw notes.txt
```

### Uninstall

```bash
sudo rm /usr/local/bin/claw
```

### Troubleshooting

Rebuild from scratch:

```bash
make clean
make
```

Check compiler version:

```bash
gcc --version
```

Check installation path:

```bash
which claw
```

Verify Git availability:

```bash
git --version
```

### Version

```text
Current Version: v1.5
Status: Stable Plugin Platform
```

---

## Current Controls

### Navigation

```text
↑              Move Cursor Up
↓              Move Cursor Down
←              Move Cursor Left
→              Move Cursor Right
Home           Move To Start Of Line
End            Move To End Of Line
```

### Editing

```text
Printable Keys Insert Character
Enter          Create New Line
Tab            Insert Spaces (Configurable Width)
Backspace      Delete Character Left
Delete         Delete Character Right
```

### Search

```text
Ctrl + F       Open Search Prompt
Ctrl + N       Next Match
Ctrl + P       Previous Match
Esc            Cancel Search
```

### Search & Replace

```text
Ctrl + R       Open Replace Prompt
Enter          Replace Current Match
Ctrl + A       Replace All Matches
Esc            Cancel Replace
```

### Undo / Redo

```text
Ctrl + Z       Undo Last Action
Ctrl + Y       Redo Last Action
```

### File Operations

```text
Ctrl + S       Save File
Ctrl + X       Quit Editor
Esc            Cancel Save-As Prompt
Ctrl + X       Confirm Quit (When Modified)
```

### Git Integration

```text
Ctrl + G       Git Add Current File
Ctrl + U       Git Unstage Current File
Ctrl + T       Git Restore Current File
Ctrl + B       Open Git Blame View
Esc            Exit Git View
```

### Plugin Manager

```text
Ctrl + K       Open Plugin Manager
↑ / ↓          Navigate Plugin List
Enter          Open Plugin Details
Space          Enable / Disable Plugin
A              Toggle AutoLoad
E              Toggle Event Hooks
H              Run Health Check
R              Reload Plugins
Esc            Back / Exit Plugin Manager
```

### Plugin Details View

```text
Space          Toggle Plugin Status
A              Toggle AutoLoad
E              Toggle Event Hooks
H              Run Health Check
R              Reload Plugin
Esc            Return To Dashboard
```

### Viewport & Scrolling

```text
Auto Vertical Scrolling
Auto Horizontal Scrolling
Cursor-Aware Viewport Tracking
Dynamic Terminal Resize Support
Large File Navigation
```

### Status Information

```text
Filename       Current Open File
Modified       Unsaved Changes Indicator
Language       Active Syntax Language
Branch         Current Git Branch
Lines          Total Line Count
Ln             Current Line Number
Col            Current Column Number
Search Info    Match Statistics
Plugin Info    Plugin Status Updates
Git Info       Repository Status
```

### Git View

```text
↑ / ↓          Navigate Blame Output
Esc            Return To Editor
```

### Save-As Prompt

```text
Type Filename  Enter Filename
Enter          Save File
Backspace      Delete Character
Esc            Cancel Save
```

### Search Prompt

```text
Type Query     Enter Search Text
Backspace      Delete Character
Enter          Execute Search
Esc            Cancel Search
```

### Replace Prompt

```text
Type Text      Enter Replacement
Enter          Replace Current Match
Ctrl + A       Replace All Matches
Backspace      Delete Character
Esc            Cancel Replace
```

### Startup & Configuration

```text
Config File      ~/.config/claw/claw.conf
Keymaps          Linux / macOS / Windows
Tab Width        Configurable (1–16)
Line Numbers     Toggleable
Status Bar       Toggleable
Welcome Screen   Toggleable
```

### Plugin Dashboard Statistics

```text
Loaded Plugins     Total Runtime Plugins
Enabled Plugins    Active Plugins
Disabled Plugins   Inactive Plugins
Health Score       Plugin Health Status
Command Runs       Plugin Command Statistics
Event Hooks        Startup/Save/Search/Exit Hooks
```

### Supported Views

```text
Editor View        Default Editing Workspace
Git Blame View     Repository Line History
Plugin Dashboard   Plugin Overview
Plugin Details     Detailed Plugin Information
Search Mode        Search Interface
Replace Mode       Search And Replace Interface
Save-As Mode       Filename Entry Interface
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
                              │   input.c       │
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
                              │  command.c      │
                              └────────┬────────┘
                                       │
                                       ▼
                              ┌─────────────────┐
                              │   Editor Core   │
                              │   editor.c      │
                              └────────┬────────┘
                                       │
        ┌───────────────┬──────────────┼──────────────┬──────────────┬──────────────┐
        │               │              │              │              │              │
        ▼               ▼              ▼              ▼              ▼              ▼
┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
│Text Buffer  │ │Search Engine│ │History Engine││Highlighting │ │ Git Engine  │ │Plugin System│
│ buffer.c    │ │ search.c    │ │ history.c    ││highlight.c  │ │   git.c     │ │ plugin.c    │
└──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘
       │               │               │               │               │               │
       └───────┬───────┴───────┬───────┴───────┬───────┴───────┬───────┴───────┬───────┘
               │               │               │               │               │
               ▼               ▼               ▼               ▼               ▼
      ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
      │ File I/O    │ │ Viewport    │ │ Config      │ │ Rendering   │ │ Status Bar  │
      │ fileio.c    │ │viewport.c   │ │ config.c    │ │ render.c    │ │statusbar.c  │
      └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘
             │               │               │               │               │
             └───────────────┴───────────────┴───────────────┴───────────────┘
                                       │
                                       ▼
                              ┌─────────────────┐
                              │     UI Layer    │
                              │ Prompts & Views │
                              └────────┬────────┘
                                       │
                   ┌───────────────────┼───────────────────┐
                   │                   │                   │
                   ▼                   ▼                   ▼
          ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
          │ Search Prompt   │ │ Git Blame View  │ │ Plugin Manager  │
          │ Replace Prompt  │ │ Git Dashboard   │ │ Plugin Details  │
          │ Save-As Prompt  │ │ Repository Info │ │ Health Monitor  │
          └─────────────────┘ └─────────────────┘ └─────────────────┘
                                       │
                                       ▼
                              ┌─────────────────┐
                              │  Plugin API     │
                              │ plugin_api.h    │
                              └────────┬────────┘
                                       │
                                       ▼
                              ┌─────────────────┐
                              │ Third-Party     │
                              │ Plugins (.so)   │
                              └─────────────────┘
```

## Component Responsibilities

### Editor Core (`editor.c`)

- Main application lifecycle management
- Editor initialization and shutdown
- Central event loop
- Modal state management
- Save-As prompt handling
- Search and replace workflow coordination
- Git view management
- Plugin Manager integration
- Plugin Details View handling
- Status message management
- Subsystem coordination

### Input Engine (`input.c`, `rawmode.c`)

- Raw terminal mode management
- Keyboard input decoding
- Escape sequence processing
- Modifier key detection
- Arrow key handling
- Control key handling
- Alt key handling
- Terminal restoration on exit

### Keymap Engine (`keymap.c`, `parser.c`)

- Key binding registration
- Command lookup
- Custom shortcut loading
- Keymap profile management
- Multiple keyboard layouts
- Keymap file parsing
- Shortcut conflict resolution

### Command System (`command.c`)

- Command dispatching
- Action execution
- Keymap integration
- Editor action routing
- Centralized command handling
- Extensible command architecture

### Config Engine (`config.c`)

- Configuration file parsing
- User preference management
- XDG configuration support
- Default settings loading
- Layered configuration system
- Runtime settings access
- Editor customization support

### Buffer Engine (`buffer.c`, `row.c`, `cursor.c`)

- Document storage
- Character insertion
- Character deletion
- Line creation and removal
- Line splitting and merging
- Dynamic memory management
- Cursor positioning
- Document modification tracking

### Search Engine (`search.c`)

- Search prompt handling
- Match discovery
- Match navigation
- Match counting
- Search highlighting
- Current match tracking
- Replace current match
- Replace all matches
- Search statistics generation

### History Engine (`history.c`)

- Undo operations
- Redo operations
- Edit history tracking
- Action recording
- Undo stack management
- Redo stack management
- Multi-line history support
- History state control

### Highlight Engine (`highlight.c`)

- Language detection
- Syntax highlighting
- Keyword recognition
- Type highlighting
- Number highlighting
- String highlighting
- Comment highlighting
- Preprocessor highlighting
- Highlight cache management

### File I/O Layer (`fileio.c`)

- File loading
- File saving
- New file creation
- Save-As support
- Filename tracking
- Persistent storage
- Unsaved change detection
- File metadata management

### Viewport Engine (`viewport.c`)

- Vertical scrolling
- Horizontal scrolling
- Cursor visibility management
- Viewport tracking
- Window resize adaptation
- Large file navigation
- Screen position calculations

### Rendering Engine (`render.c`)

- Full-screen rendering
- Cursor rendering
- Syntax color rendering
- Search highlighting rendering
- Status message rendering
- Welcome screen rendering
- Plugin Dashboard rendering
- Plugin Details rendering
- Git View rendering
- ANSI escape sequence output

### Status Bar (`statusbar.c`)

- Current filename display
- Modification indicator
- Language display
- Line and column information
- Git branch display
- Git status information
- Search status reporting
- Plugin status reporting
- User feedback messages

### Git Engine (`git.c`)

- Repository detection
- Repository root discovery
- Branch tracking
- Git status parsing
- Modified file detection
- Staged file detection
- Untracked file detection
- Git Add integration
- Git Restore integration
- Git Unstage integration
- Git Blame integration
- Repository metadata refresh

### Git View System

- Read-only Git Blame viewer
- Temporary file rendering
- Repository information display
- Git operation feedback
- View lifecycle management
- Return-to-editor workflow

### Plugin Engine (`plugin.c`)

- Runtime plugin loading
- Dynamic module management
- Plugin registration
- Plugin lifecycle control
- Plugin enable/disable support
- Plugin reload system
- Plugin discovery
- Plugin validation
- Plugin statistics tracking
- Plugin health monitoring

### Plugin API (`plugin_api.h`)

- Extension interface definition
- Event hook registration
- Command registration
- Editor integration layer
- Plugin communication interface
- Third-party extension support

### Plugin Manager

- Plugin Dashboard
- Plugin list navigation
- Plugin selection handling
- Plugin Details View
- Plugin status management
- AutoLoad configuration
- Event Hook configuration
- Plugin health checks
- Runtime plugin reloading
- Plugin statistics monitoring

### Utility Layer (`string_utils.c`)

- String manipulation helpers
- Configuration parsing helpers
- Path handling utilities
- Boolean conversion utilities
- Shared reusable functions
- Internal helper routines

---

## Development Roadmap

### Milestone 1 — Editor Foundation
**Status:** Complete

- Project structure setup
- Modular source organization
- Main editor loop
- Initial rendering engine
- Build system integration

---

### Milestone 2 — Terminal Input System
**Status:** Complete

- Raw terminal mode
- Keyboard input handling
- Arrow key support
- Escape sequence processing
- Safe terminal restoration

---

### Milestone 3 — Text Buffer Engine
**Status:** Complete

- Dynamic text buffer
- Character insertion
- Character deletion
- Row storage system
- Memory management

---

### Milestone 4 — Cursor Navigation
**Status:** Complete

- Horizontal navigation
- Vertical navigation
- Boundary handling
- Cursor tracking
- Position preservation

---

### Milestone 5 — Multi-Line Editing
**Status:** Complete

- New line creation
- Line splitting
- Line merging
- Multi-line navigation
- Dynamic row management

---

### Milestone 6 — File Management
**Status:** Complete

- File loading
- File saving
- Filename tracking
- Persistent storage
- Modification tracking

---

### Milestone 7 — Viewport & Scrolling
**Status:** Complete

- Vertical scrolling
- Horizontal scrolling
- Dynamic viewport updates
- Large file navigation
- Terminal resize support

---

### Milestone 8 — Search System
**Status:** Complete

- Search prompt
- Match discovery
- Search highlighting
- Match navigation
- Search statistics

---

### Milestone 9 — Undo / Redo System
**Status:** Complete

- Undo support
- Redo support
- Edit history tracking
- Multi-line history
- Action recording system

---

### Milestone 10 — Configuration & Keymaps
**Status:** Complete

- Configuration parser
- User settings
- XDG configuration support
- Keymap profiles
- Command dispatch architecture

---

### Milestone 11 — Syntax Highlighting
**Status:** Complete

- C language highlighting
- Keyword highlighting
- String highlighting
- Comment highlighting
- Language detection

---

### Milestone 12 — Search & Replace
**Status:** Complete

- Replace current match
- Replace all matches
- Match tracking
- Search integration
- Replacement statistics

---

### Milestone 13 — Save-As Workflow
**Status:** Complete

- Save-As prompt
- Filename validation
- Automatic filename assignment
- Syntax re-detection
- Save workflow improvements

---

### Milestone 14 — Git Integration
**Status:** Complete

- Repository detection
- Branch tracking
- Git status parsing
- Git Add support
- Git Restore support
- Git Unstage support
- Git Blame viewer
- Repository-aware workflows

---

### Milestone 15 — Plugin Platform
**Status:** Complete

- Runtime plugin loading
- Dynamic module support
- Plugin API
- Event hooks
- Custom commands
- Plugin Manager
- Plugin Dashboard
- Plugin Details View
- Plugin enable/disable support
- AutoLoad configuration
- Event Hook configuration
- Plugin reload system
- Plugin health monitoring
- Plugin statistics tracking
- Third-party plugin support

---

### Milestone 16 — Advanced Editing Tools
**Status:** In Progress

- Auto indentation
- Smart indentation engine
- Bracket matching
- Auto-closing brackets
- Duplicate line command
- Move line up/down
- Comment/Uncomment support
- Line join operations
- Tab/Space conversion tools

---

### Milestone 17 — Productivity Features
**Status:** Planned

- Multiple cursors
- Bookmarks
- Command palette
- Go To Line
- Recent files
- Quick file switching
- File explorer
- Symbol navigation
- Session restoration

---

### Milestone 18 — Themes & Customization
**Status:** Planned

- Theme engine
- Color schemes
- User-defined themes
- Appearance profiles
- Layout customization
- Status bar customization
- Theme marketplace foundation

---

### Milestone 19 — Project Tools
**Status:** Planned

- Workspace management
- Project-wide search
- Project-wide replace
- Build command integration
- Task runner
- Project configuration files
- Workspace indexing

---

### Milestone 20 — IDE Features
**Status:** Planned

- Multiple file buffers
- Split-screen editing
- Integrated terminal
- Mouse support
- Language Server Protocol (LSP)
- Code completion
- Diagnostics
- Code navigation
- Refactoring tools

---

### Milestone 21 — Git Workflow Expansion
**Status:** Planned

- Git diff viewer
- Git log viewer
- Commit message prompt
- Branch switching
- Git stash support
- Inline change indicators
- Repository dashboard

---

### Milestone 22 — Language Support Expansion
**Status:** Planned

- Python highlighting
- Java highlighting
- JavaScript highlighting
- TypeScript highlighting
- Rust highlighting
- Go highlighting
- Markdown support
- JSON support

---

### Milestone 23 — Modern Editor Platform
**Status:** Vision

- Plugin marketplace
- Remote file editing
- Collaborative editing
- Integrated debugger
- AI-assisted editing
- Project templates
- Cross-platform packaging
- Lightweight IDE workflow

---

### Milestone 24 — Claw IDE
**Status:** Vision

- Complete workspace environment
- Advanced code intelligence
- Full project navigation
- Integrated development toolkit
- Modern terminal-first IDE experience
- Extensible developer ecosystem

---

## Implemented Features

### Editor Foundation

- Modular project architecture
- Main editor loop
- Terminal-based interface
- Cross-platform POSIX support
- GCC and Clang compatibility
- Clean build system

### Terminal Input System

- Raw terminal mode
- Real-time keyboard input
- Escape sequence processing
- Arrow key handling
- Control key shortcuts
- Alt key support
- Safe terminal restoration

### Text Editing

- Character insertion
- Character deletion
- Forward deletion
- Dynamic row resizing
- Cursor-aware editing
- Real-time text manipulation
- Large document support

### Cursor Navigation

- Horizontal movement
- Vertical movement
- Boundary-aware navigation
- Position tracking
- Column preservation
- Cursor correction logic

### Multi-Line Editing

- New line creation
- Line splitting
- Line merging
- Dynamic row allocation
- Dynamic row removal
- Multi-line navigation

### Buffer Management

- Dynamic memory allocation
- Row-based document storage
- Automatic buffer growth
- Active document tracking
- Modification tracking
- Efficient text operations

### File Operations

- Open existing files
- Create new files
- Save files
- Save-As workflow
- Filename tracking
- Persistent storage
- Unsaved change detection
- Exit confirmation protection
- Automatic syntax re-detection

### Viewport & Scrolling

- Vertical scrolling
- Horizontal scrolling
- Cursor-aware viewport tracking
- Dynamic viewport updates
- Large file navigation
- Dynamic terminal resize handling

### Search System

- Search prompt
- Match discovery
- Match counting
- Match highlighting
- Current match tracking
- Next match navigation
- Previous match navigation
- Search cancellation
- Search statistics

### Search & Replace

- Replace current match
- Replace all matches
- Match tracking
- Replacement statistics
- Search integration
- Automatic next-match navigation
- Document-wide replacement

### Undo / Redo

- Undo support
- Redo support
- Edit history tracking
- Multi-line history support
- Action recording
- Separate undo and redo stacks
- History state management

### Configuration System

- INI-style configuration parser
- User configuration support
- XDG configuration support
- Layered configuration loading
- Configurable tab width
- Toggleable line numbers
- Toggleable status bar
- Toggleable welcome screen
- Runtime settings access

### Keymap System

- Custom key bindings
- Multiple keymap profiles
- Keymap file support
- Keyboard layout abstraction
- Command lookup engine
- Shortcut customization

### Command System

- Command dispatch architecture
- Centralized action routing
- Keymap integration
- Editor action execution
- Extensible command framework

### Syntax Highlighting

- C language highlighting
- Keyword highlighting
- Type highlighting
- Number highlighting
- String highlighting
- Character highlighting
- Single-line comments
- Multi-line comments
- Preprocessor directives
- Operator highlighting
- File type detection
- Language detection
- Highlight caching
- ANSI color rendering

### Rendering Engine

- Full-screen rendering
- Real-time screen refresh
- Cursor rendering
- Line number rendering
- Search highlighting rendering
- Syntax highlighting rendering
- Welcome screen rendering
- Status bar rendering
- ANSI escape sequence rendering

### User Interface

- Welcome screen
- Status bar
- Search prompt
- Replace prompt
- Save-As prompt
- Context-aware status messages
- File information display
- Cursor position display
- Modification indicators

### Git Integration

- Git repository detection
- Repository root discovery
- Branch tracking
- Git status parsing
- Modified file detection
- Staged file detection
- Untracked file detection
- Git Add integration
- Git Restore integration
- Git Unstage integration
- Repository metadata refresh

### Git View System

- Git Blame viewer
- Read-only Git view
- Temporary blame file generation
- Repository-aware workflows
- Return-to-editor workflow
- Git operation feedback

### Plugin Platform

- Runtime plugin loading
- Dynamic module support
- Plugin API
- Plugin discovery
- Plugin registration
- Plugin lifecycle management
- Startup hooks
- Save hooks
- Search hooks
- Exit hooks
- Custom command extensions
- Plugin configuration support
- Third-party plugin support

### Plugin Manager

- Interactive Plugin Dashboard
- Plugin list navigation
- Plugin Details View
- Plugin enable/disable support
- Plugin reload system
- Plugin health monitoring
- Plugin statistics tracking
- Loaded plugin counters
- Enabled plugin counters
- Disabled plugin counters
- Command execution statistics
- Plugin status indicators

### Plugin Controls

- Runtime plugin toggling
- AutoLoad configuration
- Event Hook configuration
- Plugin reload controls
- Plugin health checks
- Dashboard status messages
- Plugin state monitoring

### Included Plugins

- Hello Plugin
- Statistics Plugin
- Format Plugin

---

## Development Progress

| Version | Status | Description |
|----------|----------|----------|
| v0.1 | Complete | Project setup, modular architecture, build system, and editor framework |
| v0.2 | Complete | Raw terminal mode, keyboard input handling, and terminal control |
| v0.3 | Complete | Dynamic text buffer, character insertion, deletion, and memory management |
| v0.4 | Complete | Multi-line editing, row management, and advanced cursor navigation |
| v0.5 | Complete | File loading, file saving, persistence, and document management |
| v0.6 | Complete | Status bar, file information display, and modification tracking |
| v0.7 | Complete | Viewport engine, scrolling system, and large-file navigation |
| v0.8 | Complete | Search engine, match highlighting, navigation, and statistics |
| v0.9 | Complete | Undo/Redo system with edit history and multi-line support |
| v1.0 | Complete | Configuration system, keymaps, command architecture, and customization |
| v1.1 | Complete | Syntax highlighting, language detection, and color rendering |
| v1.2 | Complete | Search & Replace with current-match and global replacement support |
| v1.3 | Complete | Save-As workflow, filename prompts, and automatic syntax re-detection |
| v1.4 | Complete | Git integration, repository awareness, status tracking, and blame viewer |
| v1.5 | Complete | Extensible plugin platform with runtime loading, event hooks, and integrated plugin management |
| v1.6 | In Progress | Advanced editing capabilities focused on intelligent automation and developer productivity |
| v1.7 | Planned | Productivity features including bookmarks, command palette, file explorer, and session management |
| v1.8 | Planned | Themes, appearance customization, editor profiles, and UI personalization |
| v1.9 | Planned | Project tools, workspace management, build integration, and task automation |
| v2.0 | Planned | IDE features including split views, multiple buffers, LSP integration, and code intelligence |
| v2.1 | Planned | Advanced Git workflows including diff viewer, log viewer, stash support, and repository dashboard |
| v2.2 | Planned | Multi-language syntax highlighting and language support expansion |
| v3.0 | Vision | Modern editor platform with plugin marketplace, remote editing, collaboration, and AI-assisted workflows |
| v4.0 | Vision | Full Claw IDE ecosystem with advanced development tooling and extensible workspace platform |

---

## License

MIT License

---

## Author

Aryan Gupta & Pranav Chauhan 

Project: Claw Text Editor
Language: C
Platform: Linux
