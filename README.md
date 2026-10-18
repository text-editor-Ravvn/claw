# Claw

Claw is a modular terminal-first text editor and developer platform written in C.

Built on a scalable and extensible architecture, Claw delivers a fast, keyboard-driven editing experience while establishing the foundation for future developer tooling, workspace management, language services, and platform extensibility. The project is being developed as part of a larger initiative to build a Linux-based operating system from scratch.

The architecture is organized into independent systems responsible for text editing, rendering, file management, configuration, source control, extensibility, and future IDE services. This modular design enables Claw to evolve incrementally from a lightweight terminal editor into a complete developer platform while maintaining performance, simplicity, and maintainability.

Current development includes a complete editor foundation, productivity features, Git integration, and a runtime plugin framework. Future phases focus on advanced editing capabilities, workspace environments, project tooling, language intelligence, IDE services, and ecosystem expansion.

### Design Philosophy

> Provide modern developer capabilities while remaining lightweight, extensible, performant, and terminal-first.

---

## Current Status

|       Property     |                      Value                     |
|--------------------|------------------------------------------------|
| Current Version    | v1.8                                           |
| Active Phase       | Phase 7 — Project Tooling                      |
| Development Status | Active Development                             |
| Completed Phases   | Phase 1, Phase 2, Phase 3, Phase 4, Phase 5, Phase 6 |
| Current Focus      | Project configuration and tooling foundation    |

### Summary

Claw has completed its Core Editor Foundation, Productivity Foundation, Developer Platform, Advanced Editing Layer, Workspace Environment, and Customization Platform phases. Development is now entering Phase 7 (Project Tooling), beginning with project configuration discovery.

### Completed Architecture Phases

✓ Phase 1 — Core Editor Foundation

✓ Phase 2 — Productivity Foundation

✓ Phase 3 — Developer Platform

✓ Phase 4 — Advanced Editing Layer

✓ Phase 5 — Workspace Environment

### Completed Phase 6

✓ Phase 6 — Customization Platform

Current focus areas:

- Theme engine and runtime theme switching
- Built-in and user-defined themes
- Theme import and export
- Appearance and layout customization
- Status bar customization

### Active Development

◉ Phase 7 — Project Tooling

Current focus areas:

- Project configuration discovery
- Build command integration
- Task runner support
- Build output and diagnostics

### Implemented Systems

- Core Editor Engine
- Text Buffer Engine
- Cursor Engine
- Rendering Engine
- Viewport Engine
- File I/O Layer
- Search Engine
- Search & Replace Engine
- Undo / Redo Engine
- Configuration System
- Keymap System
- Syntax Highlighting System
- Git Integration System
- Runtime Plugin Platform
- Plugin Manager
- Plugin Dashboard
- Status Bar System
- Advanced Editing Engine (Phase 4)
- Workspace Environment Engine (Phase 5)
- Session Management & Persistence (Phase 5)
- File Explorer & Quick Switcher (Phase 5)
- Bookmark Navigation System (Phase 5)

### Upcoming Architecture Phases

✓ Phase 6 — Customization Platform

◉ Phase 7 — Project Tooling

○ Phase 8 — IDE Foundation

○ Phase 9 — Source Control Platform

○ Phase 10 — Language Ecosystem

○ Phase 11 — Intelligent IDE Services

○ Phase 12 — Advanced Platform Services

○ Phase 13 — Claw Ecosystem

## Current Objectives

### Completed Phase 6 — Customization Platform

#### Theme Engine
- Implement modular color theme loader and runtime palette manager
- Support 256-color and 24-bit TrueColor terminal rendering
- Provide built-in themes (claw-dark, monokai, solarized, nord, dracula)
- Support custom user-defined themes in `~/.config/claw/themes/`

#### UI Personalization
- Configurable status bar segments, formatting, and positions
- Customizable line number styling and gutter spacing
- Adjustable cursor styles (block, underline, bar)
- Runtime appearance switching without restarting the editor

#### Profile & Configuration System
- Named user configuration profiles (e.g., minimal, coding, presentation)
- Per-language style overrides and indentation definitions
- Hot-reloading of configuration files upon modification

### Phase 7 — Project Tooling

- Project configuration discovery from `.claw/project.conf`
- Project metadata and command model
- Build command integration (`Ctrl+Shift+B` / `project_build`)
- Named project tasks using `task.<name> = <command>`
- Captured build and task output view
- Compiler diagnostics parsing for errors, warnings, and notes
- Task runner support
- Build output and diagnostics

#### Phase 7 Foundation Notes
- Layout architecture for split panes and secondary views
- Integration points for project tooling and build outputs
- Modularity benchmarks for theme and customization performance

## Completed Features

### Core Editor Foundation
- Modular editor architecture
- Event-driven editor workflow
- Centralized command dispatch system
- Clean subsystem separation
- Extensible component design
- POSIX-compatible implementation
- Low-memory document management
- Fast startup and lightweight execution

### Character Editing
- Character insertion at cursor position
- Backspace deletion
- Forward deletion using Delete key
- In-line text editing
- Dynamic row resizing
- Cursor-aware editing operations
- Real-time document updates

### Cursor Management
- Horizontal cursor movement
- Vertical cursor movement
- Boundary-aware navigation
- Row transition handling
- Cursor position tracking
- Column preservation during navigation
- Automatic cursor correction

### Multi-Line Editing
- New line creation
- Line splitting at cursor position
- Line merging with Backspace
- Dynamic row creation
- Dynamic row removal
- Multi-line document support
- Row allocation and cleanup

### Smart Editing Features (v1.6 In Progress)
- Automatic indentation inheritance
- Smart indentation after opening braces
- Indentation-aware new line creation
- Foundation for bracket-aware editing
- Advanced editing architecture preparation
- Extensible editing command framework

### Text Buffer Management
- Dynamic memory allocation
- Row-based document storage
- Automatic buffer growth
- Efficient text manipulation
- Buffer modification tracking
- Active document management
- Document state preservation

### File Operations
- Open existing files
- Create new files
- Save files to disk
- Save-As workflow
- Filename tracking
- Persistent storage management
- Unsaved change detection
- Exit confirmation protection
- Automatic syntax re-detection after Save-As
- New file creation workflow

### Viewport & Scrolling
- Vertical scrolling
- Horizontal scrolling
- Cursor-aware viewport tracking
- Dynamic viewport updates
- Large file navigation support
- Terminal resize handling
- Viewport synchronization

### Search System
- Interactive search prompt
- Real-time search input
- Match discovery across document
- Search result highlighting
- Match counting
- Current match tracking
- Jump-to-match navigation
- Next match navigation
- Previous match navigation
- Search state management
- Search statistics display
- Automatic highlight clearing

### Search & Replace
- Replace current match
- Replace next match workflow
- Replace all matches
- Search persistence after replacement
- Match tracking after replacement
- Replacement statistics
- Search result preservation
- Automatic navigation after replacement
- Document-wide replacement support

### Undo / Redo System
- Undo character insertion
- Undo character deletion
- Undo line split operations
- Redo character insertion
- Redo character deletion
- Redo line split operations
- Multi-line history support
- Edit action recording
- Separate undo and redo stacks
- History management architecture

### Syntax Highlighting
- C language syntax highlighting
- Language-aware rendering
- File extension detection
- Language detection system
- Keyword highlighting
- Type keyword highlighting
- Number highlighting
- String highlighting
- Character literal highlighting
- Operator highlighting
- Single-line comment highlighting
- Multi-line comment highlighting
- Preprocessor directive highlighting
- ANSI color rendering
- Highlight cache management
- Extensible language definition architecture

### Rendering Engine
- Full-screen terminal rendering
- Cursor rendering
- Real-time screen refresh
- Welcome screen rendering
- Status bar rendering
- Search highlight rendering
- Syntax highlight rendering
- Dynamic viewport rendering
- ANSI escape sequence rendering
- Efficient screen updates

### User Interface
- Welcome screen
- Status bar
- Search prompt
- Replace prompt
- Save-As filename prompt
- Context-aware status messages
- File information display
- Cursor position display
- Search statistics display
- Editor feedback messages
- Unsaved changes warning system

### Terminal Support
- Raw terminal mode
- Real-time keyboard input
- Escape sequence processing
- Arrow key handling
- Control key handling
- Alt key handling
- POSIX terminal compatibility
- Safe terminal restoration
- Dynamic terminal resize support

### Configuration System
- INI-style configuration parser
- User configuration support
- XDG configuration support
- Local configuration fallback
- Layered configuration architecture
- Startup preference loading
- Configurable tab width
- Toggleable line numbers
- Toggleable status bar
- Toggleable welcome screen
- Runtime configuration management

### Keymap System
- Custom key binding support
- Multiple keymap profiles
- Keymap parser
- Command lookup system
- Dynamic key binding loading
- Default keymap registration
- Configurable editor shortcuts
- Command routing architecture

### Git Integration
- Git repository detection
- Repository root discovery
- Current branch identification
- Repository-aware startup
- Git status parsing
- Modified file detection
- Staged file detection
- Untracked file detection
- Git metadata refresh
- Repository validation
- Git command execution layer

### Git Operations
- Git Add integration
- Git Restore integration
- Git Unstage integration
- Git Blame integration
- Repository status refresh
- Git-aware workflows
- Safe Git command execution

### Git Blame Viewer
- Temporary blame file generation
- Read-only blame view
- Line ownership display
- Commit information display
- Author information display
- Return-to-editor workflow
- Repository-aware blame execution

### Plugin Platform
- Runtime plugin loading
- Dynamic module support
- Plugin API
- Plugin lifecycle management
- Plugin discovery system
- Plugin registration framework
- Third-party plugin support
- Custom command extensions
- Startup hooks
- Save hooks
- Search hooks
- Exit hooks
- Plugin configuration support

### Plugin Management
- Interactive Plugin Manager
- Plugin Dashboard
- Plugin list view
- Plugin details view
- Plugin status indicators
- Plugin enable/disable controls
- AutoLoad support
- Event hook controls
- Plugin reload support
- Plugin health monitoring
- Plugin health checks
- Plugin statistics tracking
- Loaded plugin counter
- Enabled plugin counter
- Disabled plugin counter
- Command execution statistics
- Plugin status reporting

### Included Plugins
- Hello Plugin
- Statistics Plugin
- Format Plugin

### Architecture & Infrastructure
- Modular source tree
- Layered subsystem architecture
- Event-driven plugin framework
- Editor service abstraction
- Shared utility layer
- Reusable rendering pipeline
- Extensible command architecture
- Scalable project structure
- Foundation for future IDE capabilities

### Advanced Editing Layer (Phase 4)
- Smart indentation engine
- Automatic indentation inheritance
- Context-aware indentation after braces
- Bracket matching and visual highlighting
- Auto-closing brackets and delimiters
- Smart delimiter skip logic
- Tab-to-space conversion
- Duplicate line operations (`Alt+D`)
- Move line up/down commands (`Shift+↑` / `Shift+↓`)
- Comment and uncomment support (`Alt+/`)
- Line join (`Alt+J`) and line split operations
- Smart backspace handling (indentation zone aware)
- Smart tab and unindent behavior (`Tab` / `Shift-Tab`)
- Line boundary navigation (`Home` / `End`)
- Word-based navigation (`Ctrl+←` / `Ctrl+→`)
- Bracket jump navigation (`Alt+]`)
- Indentation-aware new line creation
- Cursor-aware editing workflows
- Advanced line manipulation tools
- Editing productivity shortcuts
- Advanced editing command framework

## Completed Features

### Phase 5 — Workspace Environment

- Workspace management system & dynamic root detection (.claw / .git)
- Workspace configuration system (`.claw/workspace.conf` & overrides)
- Recent files & workspace history tracker (MRU)
- Full session serialization & automatic restoration
- Go-to-line navigation workflow (`Alt+G` / `:line`)
- Comprehensive bookmark system with gutter indicator & cyclic jumping (`Alt+B`, `Alt+Down`, `Alt+Up`, `Alt+C`)
- Interactive Quick File Switcher modal (`Alt+Q`)
- Built-in Workspace File Explorer dashboard (`Alt+E`)
- Dynamic status bar workspace branding & bookmark statistics

## In Progress

### Phase 6 — Customization Platform

- Theme engine
- User-defined themes
- Color scheme management
- Appearance profiles
- Layout customization
- Status bar customization
- Syntax theme customization
- Runtime theme switching
- User preference profiles
- Advanced editor personalization

### Phase 7 — Project Tooling

- Project-wide search
- Project-wide replace
- Build command integration
- Task runner support
- Project configuration management
- Workspace indexing
- Project navigation tools
- Build output integration
- Development workflow automation
- Project diagnostics foundation

### Phase 8 — IDE Foundation

- Multiple file buffers
- Split-screen editing
- Integrated terminal
- Mouse support
- File explorer
- Symbol navigation
- Code navigation framework
- Editor panels and views
- Workspace-aware editing
- IDE infrastructure foundation

### Phase 9 — Source Control Platform

- Git diff viewer
- Git log viewer
- Commit workflow integration
- Branch management
- Git stash support
- Repository dashboard
- Inline change indicators
- Advanced blame integration
- Source control visualization
- Repository workflow tools

### Phase 10 — Language Ecosystem

- Python language support
- Java language support
- JavaScript language support
- TypeScript language support
- Rust language support
- Go language support
- Markdown support
- JSON support
- YAML support
- Extensible language definition framework

### Phase 11 — Intelligent IDE Services

- Language Server Protocol (LSP) integration
- Code completion
- Diagnostics and error reporting
- Definition lookup
- Reference search
- Symbol indexing
- Code actions
- Refactoring framework
- Intelligent code navigation
- Language-aware developer tools

### Phase 12 — Advanced Platform Services

- Remote file editing
- Workspace synchronization
- Cross-platform packaging
- Integrated debugger
- Advanced extension services
- Project template system
- Workspace automation
- Development environment provisioning
- Advanced tooling infrastructure
- Platform service framework

### Phase 13 — Claw Ecosystem

- Plugin marketplace
- Community extension platform
- Third-party integration ecosystem
- Plugin discovery services
- Plugin distribution platform
- Collaborative development features
- AI-assisted development workflows
- Ecosystem management tools
- Community-driven extension support
- Complete developer platform experience

## Long-Term Vision

- Build a modern terminal-first development platform written entirely in C
- Deliver IDE-level capabilities while preserving the simplicity of a text editor
- Maintain exceptional performance, low memory usage, and fast startup times
- Provide a fully keyboard-driven development experience
- Create a modular architecture that remains easy to extend and maintain
- Support complete workspace-centric software development workflows
- Establish a powerful customization and configuration ecosystem
- Provide advanced project management and build automation capabilities
- Deliver first-class Git and source control integration
- Support a wide range of programming languages and development environments
- Integrate intelligent code navigation, diagnostics, and developer assistance
- Enable Language Server Protocol (LSP) powered development workflows
- Build a scalable plugin and extension ecosystem
- Create a community-driven marketplace for extensions and integrations
- Support remote development and distributed development environments
- Provide advanced tooling for professional software engineering workflows
- Offer seamless project navigation across small and large codebases
- Enable collaborative and ecosystem-driven platform growth
- Preserve transparency through clean, readable, and educational source code
- Maintain cross-platform compatibility across POSIX-compliant systems
- Remain independent of heavy frameworks and unnecessary dependencies
- Serve as both a production-ready development tool and a systems programming learning platform
- Continuously evolve through sustainable architectural improvements
- Bridge the gap between lightweight editors and full-featured IDEs
- Establish Claw as a complete developer platform rather than a standalone text editor

### End Goal

Claw aims to evolve from a lightweight terminal text editor into a complete developer ecosystem that combines the speed and simplicity of traditional editors with the power, extensibility, and workflow capabilities of modern integrated development environments, while remaining lightweight, modular, performant, and entirely written in C.

---

## Project Goals

### Core Objectives

- Build a modern terminal-first development platform written entirely in C
- Deliver a fast, lightweight, and reliable editing experience
- Maintain a modular, scalable, and maintainable architecture
- Provide a keyboard-first workflow optimized for developer productivity
- Preserve simplicity without sacrificing capability
- Support extensibility through plugins, configuration, and platform services
- Establish a strong foundation for long-term ecosystem growth

### Architecture Objectives

- Develop a clean separation between editor subsystems
- Maintain a highly modular component-based architecture
- Enable independent evolution of editor services and platform features
- Support scalable expansion through clearly defined development phases
- Reduce coupling between core editor functionality and platform extensions
- Ensure long-term maintainability and code quality
- Preserve architectural consistency across all phases of development

### Performance Objectives

- Maintain low memory consumption
- Deliver near-instant startup times
- Support efficient editing of large files
- Optimize rendering and viewport performance
- Minimize runtime overhead across editor subsystems
- Preserve responsiveness during intensive editing operations
- Maintain lightweight execution characteristics across all supported platforms

### Developer Experience Objectives

- Provide efficient editing and navigation workflows
- Deliver reliable search, replace, and document management capabilities
- Support advanced editing operations without increasing complexity
- Enable workspace-oriented development environments
- Improve project navigation and organization workflows
- Reduce friction in day-to-day software development tasks
- Create a productive terminal-based development experience

### Extensibility Objectives

- Support a robust plugin architecture
- Provide a stable public extension API
- Enable event-driven editor extensions
- Allow runtime feature expansion without modifying core systems
- Support community-driven feature development
- Create a scalable extension ecosystem
- Establish a foundation for a future plugin marketplace

### Source Control Objectives

- Provide repository-aware development workflows
- Integrate Git directly into the editing experience
- Support common version-control operations
- Improve repository visibility and status awareness
- Expand source-control tooling through dedicated platform services
- Reduce context switching between development tools

### Platform Objectives

- Support workspace-centric development workflows
- Provide project management and development tooling
- Enable multi-language development environments
- Establish IDE-level capabilities through modular services
- Support Language Server Protocol (LSP) integration
- Deliver intelligent development assistance
- Create a complete developer platform while preserving editor performance

### Educational Objectives

- Demonstrate modern systems programming practices in C
- Showcase modular software architecture design
- Provide practical examples of terminal application development
- Explore text editor internals and development workflows
- Serve as an educational open-source project
- Encourage learning through readable and maintainable source code

### Strategic Objectives

- Bridge the gap between lightweight editors and traditional IDEs
- Build a sustainable long-term development roadmap
- Maintain cross-platform POSIX compatibility
- Preserve independence from heavy external frameworks
- Encourage open-source collaboration and community participation
- Establish Claw as a modern, extensible, terminal-first development platform

### Guiding Principles

- Performance before complexity
- Simplicity before feature bloat
- Architecture before implementation shortcuts
- Extensibility through modular design
- Reliability through predictable behavior
- Productivity through efficient workflows
- Continuous improvement through incremental development
- Long-term maintainability over short-term convenience

---

## Project Structure

```text
claw/
├── include/
│   ├── buffer/
│   │   ├── buffer.h
│   │   ├── cursor.h
│   │   └── history.h
│   │
│   ├── command/
│   │   └── command.h
│   │
│   ├── config/
│   │   ├── config.h
│   │   └── keymap.h
│   │
│   ├── editor/
│   │   └── editor.h
│   │
│   ├── fileio/
│   │   └── fileio.h
│   │
│   ├── git/
│   │   └── git.h
│   │
│   ├── input/
│   │   ├── input.h
│   │   └── rawmode.h
│   │
│   ├── plugin/
│   │   ├── plugin.h
│   │   ├── plugin_api.h
│   │   └── plugin_commands.h
│   │
│   ├── render/
│   │   ├── highlight.h
│   │   ├── render.h
│   │   ├── statusbar.h
│   │   └── viewport.h
│   │
│   ├── search/
│   │   └── search.h
│   │
│   └── utils/
│       └── utils.h
│
├── src/
│   ├── buffer/
│   │   ├── buffer.c
│   │   ├── cursor.c
│   │   ├── row.c
│   │   └── history.c
│   │
│   ├── command/
│   │   └── command.c
│   │
│   ├── config/
│   │   ├── config.c
│   │   ├── keymap.c
│   │   └── parser.c
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
│   ├── input/
│   │   ├── input.c
│   │   └── rawmode.c
│   │
│   ├── plugin/
│   │   ├── plugin.c
│   │   ├── plugin_loader.c
│   │   ├── plugin_manager.c
│   │   └── plugin_commands.c
│   │
│   ├── render/
│   │   ├── highlight.c
│   │   ├── render.c
│   │   ├── statusbar.c
│   │   └── viewport.c
│   │
│   ├── search/
│   │   └── search.c
│   │
│   ├── utils/
│   │   └── string_utils.c
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
├── themes/                     # Phase 6
│
├── workspaces/                 # Phase 5
│
├── projects/                   # Phase 7
│
├── language/
│   ├── c.lang
│   ├── cpp.lang
│   └── python.lang
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
│   ├── ROADMAP.md
│   ├── PHASES.md
│   └── CONTRIBUTING.md
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
│   └── editor.c                    Core editor lifecycle, event loop, subsystem coordination, and command dispatch
│
├── input/
│   ├── input.c                     Keyboard input processing, key decoding, and shortcut handling
│   └── rawmode.c                   Terminal raw mode initialization and management
│
├── buffer/
│   ├── buffer.c                    Document management, text insertion, deletion, and buffer operations
│   ├── row.c                       Row manipulation, line operations, indentation, and editing workflows
│   └── cursor.c                    Cursor movement, navigation, positioning, and viewport interaction
│
├── config/
│   └── config.c                    Configuration loading, parsing, validation, and settings management
│
├── command/
│   └── command.c                   Command registration, execution, and editor action routing
│
├── keymap/
│   ├── keymap.c                    Key binding registration, lookup, and shortcut management
│   └── parser.c                    Keymap parsing and configuration decoding
│
├── render/
│   └── render.c                    Screen rendering, editor views, UI drawing, and visual updates
│
├── highlight/
│   └── highlight.c                 Syntax highlighting engine, token processing, and language definitions
│
├── fileio/
│   └── fileio.c                    File loading, saving, persistence, and document management
│
├── viewport/
│   └── viewport.c                  Scrolling, viewport tracking, visibility management, and terminal resizing
│
├── search/
│   └── search.c                    Search engine, replace operations, match tracking, and navigation workflows
│
├── history/
│   └── history.c                   Undo/Redo engine, edit history management, and state restoration
│
├── git/
│   └── git.c                       Repository awareness, status tracking, blame integration, staging, and restore workflows
│
├── plugin/
│   ├── plugin.c                    Plugin runtime, lifecycle management, and extension framework
│   ├── plugin_loader.c             Dynamic plugin discovery, loading, unloading, and validation
│   ├── plugin_manager.c            Plugin dashboard, health monitoring, and management workflows
│   └── plugin_commands.c           Plugin-specific editor commands and actions
│
├── ui/
│   └── statusbar.c                 Status bar rendering, notifications, and editor information display
│
├── utils/
│   └── string_utils.c              Shared utility functions, helper routines, and common abstractions
│
└── plugins/
    ├── hello_plugin.c              Example plugin implementation
    ├── stats_plugin.c              Statistics and analytics plugin
    └── format_plugin.c             Formatting and code cleanup plugin
```

---

## Build Instructions

### Requirements

- GCC or Clang
- GNU Make
- POSIX-compatible terminal
- Linux or macOS
- Git (required for source-control features)
- dlopen-compatible system libraries (required for runtime plugins)

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

Builds Claw with debugging symbols and additional compiler diagnostics.

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

If the file does not exist, Claw automatically creates it when saved.

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
Esc    Cancel Save
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
Ctrl + G    Stage Current File
Ctrl + U    Unstage Current File
Ctrl + T    Restore Current File
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

Additional language support will be introduced during the Language Ecosystem phases.

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

### Current Development Status

```text
Current Phase: Phase 7 — Project Tooling

Status: Active Development

Completed:
✓ Phase 1 — Core Editor Foundation
✓ Phase 2 — Productivity Foundation
✓ Phase 3 — Developer Platform
✓ Phase 4 — Advanced Editing Layer
✓ Phase 5 — Workspace Environment
✓ Phase 6 — Customization Platform

In Progress:
• Project configuration discovery
• Build command integration
• Task runner support
• Build output and diagnostics
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

### Search & Navigation

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

### Undo & History

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

### Project Tooling

Create `.claw/project.conf` in the workspace root:

```ini
name = claw
type = c
build = mingw32-make
run = claw
test = mingw32-make test
task.check = mingw32-make test
```

```text
Ctrl + Shift + B  Run the configured project build command
Ctrl + Shift + T  Run the configured project test task
Ctrl + Shift + O  Open the latest project output
```

The build command and named tasks run from the detected project root. CLAW
reports whether the command succeeded or failed in the status bar. The
`build`, `run`, and `test` fields are also available as built-in task names.
Command output is captured in `.claw/project-output.log` and can be reviewed
in the project output view. Press `Esc` to close the view.

Compiler-style diagnostics in the captured output are parsed into structured
file, line, column, severity, and message records for the diagnostics
navigation features planned in Phase 7.6.

### Plugin Platform

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

### Git View

```text
↑ / ↓          Navigate Blame Output
Esc            Return To Editor
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

### Configuration Controls

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
Editor View        Primary Editing Workspace
File Explorer      Workspace Directory Browser
Quick Switcher     Interactive Recent & Project File Search
Git Blame View     Repository Line History
Plugin Dashboard   Plugin Overview
Plugin Details     Plugin Information & Controls
Search Mode        Search Interface
Replace Mode       Search & Replace Interface
Go-to-Line Mode    Line & Column Jumping Interface
Save-As Mode       Filename Entry Interface
```

### Workspace & Navigation Controls (Phase 5)

```text
Alt + E       Open Workspace File Explorer
Alt + Q       Open Quick File Switcher
Alt + G       Go to Line Prompt (line or line:col)
Alt + B       Toggle Line Bookmark
Alt + ↓       Jump to Next Bookmark (Cyclic)
Alt + ↑       Jump to Previous Bookmark (Cyclic)
Alt + C       Clear All Bookmarks
Alt + W       Workspace Status & Root Refresh
Alt + S       Save Current Session State
Alt + R       Restore Last Session State
```

### Workspace File Explorer

```text
↑ / ↓          Navigate Directory Entries
Enter          Open Selected File / Enter Subdirectory
Backspace / ←  Navigate to Parent Directory
Esc            Close File Explorer & Return to Editor
```

### Quick File Switcher

```text
Type Filter    Substring Filter Candidates in Real Time
↑ / ↓          Navigate Matched File List
Enter          Open Selected File Instantly
Backspace      Delete Character from Query
Esc            Cancel Switcher
```

### Go-To-Line Prompt

```text
Type Line      Enter Line Number or line:col (e.g., 42 or 42:10)
Enter          Execute Jump & Center Viewport
Backspace      Delete Character
Esc            Cancel Jump
```

### Upcoming Controls (Phase 6)

```text
Theme Switching
Custom Color Schemes
Status Bar Customization
UI Layout Profiles
Gutter Width & Appearance Overrides
Runtime Profile Selection
```

---

## Architecture

```text
┌─────────────────────────────────────────────────────────────┐
│                         Terminal UI                         │
│                   Keyboard • Views • Prompts               │
└─────────────────────────────┬───────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Input & Command Layer                    │
│                                                             │
│  Input Engine • Keymaps • Commands • Event Routing          │
└─────────────────────────────┬───────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                       Editor Core Layer                     │
│                                                             │
│  Buffer • Rows • Cursor • History • Search • Editing        │
└─────────────────────────────┬───────────────────────────────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
          ▼                   ▼                   ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│ File System     │ │ Render Engine   │ │ Config System   │
│                 │ │                 │ │                 │
│ File I/O        │ │ Viewport        │ │ Settings        │
│ Persistence     │ │ Status Bar      │ │ Keymaps         │
│ Document State  │ │ Syntax Engine   │ │ User Profiles   │
└────────┬────────┘ └────────┬────────┘ └────────┬────────┘
         │                   │                   │
         └──────────┬────────┴──────────┬────────┘
                    │                   │
                    ▼                   ▼
┌─────────────────────────┐ ┌─────────────────────────┐
│   Source Control Layer  │ │    Extension Layer      │
│                         │ │                         │
│ Git Integration         │ │ Plugin Runtime          │
│ Repository Awareness    │ │ Plugin Manager          │
│ Status Tracking         │ │ Plugin API             │
│ Blame / Diff Systems    │ │ Event Hooks            │
└─────────────┬───────────┘ └─────────────┬───────────┘
              │                           │
              └─────────────┬─────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Workspace Environment                    │
│                                                             │
│ Projects • Sessions • Workspaces • Navigation              │
└─────────────────────────────┬───────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     IDE Foundation Layer                    │
│                                                             │
│ Multiple Buffers • Split Views • Terminal Integration      │
└─────────────────────────────┬───────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Language Ecosystem Layer                 │
│                                                             │
│ Syntax Packages • Language Definitions • Toolchains        │
└─────────────────────────────┬───────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                 Intelligent Development Layer               │
│                                                             │
│ LSP • Diagnostics • Completion • Navigation • Refactoring  │
└─────────────────────────────┬───────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  Advanced Platform Services                 │
│                                                             │
│ Remote Development • Workspace Services • Automation       │
└─────────────────────────────┬───────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                       Claw Ecosystem                        │
│                                                             │
│ Marketplace • Community Plugins • Extensions • Services    │
└─────────────────────────────────────────────────────────────┘
```

## Architectural Principles

```text
- Layered architecture with clear subsystem boundaries
- Modular component design
- Separation of editor core and platform services
- Extensibility through plugins and public APIs
- Workspace-oriented development workflows
- Terminal-first user experience
- Performance-focused implementation
- Incremental phase-driven development
- Cross-platform POSIX compatibility
- Long-term maintainability and scalability
```

## Component Responsibilities

### Core Editor Foundation

#### Editor Core

- Application startup and shutdown
- Global editor state management
- Event loop execution
- Subsystem coordination
- Mode switching and workflow management
- Runtime service orchestration
- Editor lifecycle control
- Platform integration management

#### Input & Interaction Layer

- Raw terminal processing
- Keyboard event handling
- Escape sequence decoding
- Shortcut processing
- Modifier key detection
- Input normalization
- Cross-platform keyboard compatibility
- User interaction routing

#### Command Framework

- Command registration
- Command dispatching
- Action execution
- Context-aware command routing
- Command abstraction layer
- Editor workflow integration
- Extensible command architecture
- Future plugin command integration

#### Configuration Platform

- Configuration management
- User preference loading
- Runtime settings access
- Configuration validation
- Profile management
- Environment customization
- Keymap integration
- User experience personalization

#### Rendering Engine

- Screen rendering
- ANSI terminal output
- Incremental redraw logic
- Cursor rendering
- Highlight rendering
- Search result rendering
- View rendering
- UI state presentation
- Performance optimization

#### Syntax Platform

- Language detection
- Syntax highlighting
- Keyword recognition
- Token classification
- Theme integration foundation
- Highlight caching
- Language definition management
- Future language package support

#### Viewport Engine

- Vertical scrolling
- Horizontal scrolling
- Cursor tracking
- Screen calculations
- Window resize handling
- Large file navigation
- Rendering optimization
- Future split-view support

#### Status System

- Editor status reporting
- File information display
- Position tracking
- Search feedback
- Plugin feedback
- Git feedback
- Runtime notifications
- User messaging

#### File System Platform

- File loading
- File saving
- Save-As workflows
- Filename management
- Document persistence
- Change tracking
- File metadata management
- Future workspace persistence

---

### Productivity Foundation

#### Buffer Engine

- Document storage
- Row management
- Character insertion
- Character deletion
- Line insertion
- Line removal
- Line splitting
- Line merging
- Dynamic memory allocation
- Document state tracking

#### Cursor Engine

- Cursor movement
- Position tracking
- Navigation logic
- Screen position mapping
- Viewport interaction
- Selection foundation
- Editing workflow integration

#### Search Platform

- Text search
- Match discovery
- Match navigation
- Match statistics
- Search highlighting
- Search result tracking
- Replace operations
- Future project-wide search support

#### History System

- Undo operations
- Redo operations
- Edit tracking
- Action recording
- History management
- Recovery operations
- Multi-line action support
- Future persistent history support

---

### Developer Platform

#### Plugin Runtime

- Dynamic plugin loading
- Plugin discovery
- Plugin registration
- Runtime management
- Plugin lifecycle control
- Plugin validation
- Plugin statistics
- Plugin monitoring

#### Plugin API

- Extension interfaces
- Command registration
- Event registration
- Editor integration
- Runtime communication
- Third-party extension support
- Plugin compatibility layer
- Future marketplace integration

#### Plugin Management System

- Plugin dashboard
- Plugin monitoring
- Plugin configuration
- Plugin health checks
- Runtime reload support
- AutoLoad management
- Event hook management
- Plugin diagnostics

---

### Advanced Editing Layer

#### Advanced Editing System

- Smart indentation
- Indentation-aware editing
- Bracket intelligence
- Auto-closing delimiters
- Smart backspace handling
- Smart tab processing
- Duplicate line operations
- Line movement operations
- Comment workflows
- Editing productivity automation

---

### Source Control Platform

#### Git Integration

- Repository detection
- Repository awareness
- Branch tracking
- Status monitoring
- File state analysis
- Git operations
- Blame integration
- Repository metadata management

#### Git Workspace Services

- Git dashboard
- Blame viewer
- Repository reporting
- Change visualization
- Workflow feedback
- Source-control integration
- Future diff viewer support
- Future history explorer support

---

### Workspace Environment

#### Workspace Services

- Session management
- Project awareness
- Workspace tracking
- File navigation foundation
- Environment state management
- Project organization support
- Future explorer integration
- Future workspace automation

---

### Customization Platform

#### Personalization Services

- Theme management
- Appearance customization
- Editor profiles
- Layout preferences
- User-defined workflows
- Shortcut customization
- Visual customization
- Environment personalization

---

### Project Tooling

#### Development Services

- Project management
- Build workflow integration
- Task execution framework
- Project navigation
- Project metadata management
- Development automation
- Toolchain integration
- Workspace tooling

---

### IDE Foundation

#### IDE Services

- Multi-buffer architecture
- Split views
- Workspace coordination
- Terminal integration
- Navigation systems
- Development workflows
- Project-centric editing
- IDE infrastructure

---

### Language Ecosystem

#### Language Services

- Language definitions
- Syntax packages
- Parser integrations
- Toolchain integration
- Language-specific workflows
- Ecosystem management
- Language extensibility
- Cross-language support

---

### Intelligent IDE Services

#### Intelligence Layer

- LSP integration
- Code completion
- Diagnostics
- Symbol navigation
- Refactoring support
- Intelligent analysis
- Development assistance
- Productivity services

---

### Advanced Platform Services

#### Platform Infrastructure

- Remote development
- Workspace synchronization
- Automation services
- Platform APIs
- Service integrations
- Distributed workflows
- Advanced tooling
- Infrastructure management

---

### Claw Ecosystem

#### Ecosystem Services

- Plugin marketplace
- Community extensions
- Package distribution
- Extension discovery
- Ecosystem management
- Developer tooling
- Community infrastructure
- Long-term platform growth

---

## Development Roadmap

### Phase 1 — Core Editor Foundation

**Status:** Complete

- Project architecture and modular source layout
- Editor lifecycle and event loop
- Terminal raw mode support
- Keyboard input processing
- Cursor navigation system
- Dynamic text buffer engine
- Multi-line editing support
- File loading and saving
- Viewport and scrolling engine
- Search platform
- Undo / Redo history system
- Configuration management
- Keymap architecture
- Syntax highlighting framework
- Search & Replace workflows
- Save-As workflow

---

### Phase 2 — Productivity Foundation

**Status:** Complete

- Search and navigation workflows
- Search & Replace operations
- Undo / Redo system
- Edit history management
- Configuration platform
- Keymap customization
- Command framework
- Editing productivity workflows
- User workflow optimization
- Productivity-focused editing foundation

---

### Phase 3 — Developer Platform

**Status:** Complete

- Runtime plugin loading
- Dynamic module support
- Plugin API
- Event hook architecture
- Custom command registration
- Plugin Manager
- Plugin Dashboard
- Plugin Details View
- Plugin enable / disable controls
- AutoLoad configuration
- Event Hook configuration
- Plugin reload system
- Plugin health monitoring
- Plugin statistics tracking
- Third-party extension support

---

### Phase 4 — Advanced Editing Layer

**Status:** Completed

- Smart indentation engine
- Indentation-aware new line creation
- Auto-closing brackets
- Bracket matching and visual highlighting
- Jump-to-bracket navigation (`Alt+]`)
- Word-based navigation (`Ctrl+←` / `Ctrl+→`)
- Line boundary navigation (`Home` / `End`)
- Smart tab and unindent handling (`Tab` / `Shift-Tab`)
- Smart backspace handling
- Duplicate line operations (`Alt+D`)
- Move line up/down (`Shift+↑` / `Shift+↓`)
- Comment / Uncomment support (`Alt+/`)
- Line join operations (`Alt+J`)
- Line split operations
- Editing productivity commands
- Advanced cursor workflows
- Editing automation framework

---

### Phase 5 — Workspace Environment

**Status:** Completed

- Session management & state restoration
- Workspace tracking & dynamic root detection
- Recent files & workspace history (MRU)
- Project navigation & Go-To-Line workflows
- Built-in workspace file explorer
- Workspace persistence & auto-save
- Quick file switching modal
- Full bookmark system with gutter indicator & cyclic jumping
- Workspace configuration files (.claw/workspace.conf)

---

### Phase 6 — Customization Platform

**Status:** Complete

- Theme engine
- Color schemes
- User-defined themes
- Appearance profiles
- Layout customization
- Status bar customization
- User personalization workflows
- Theme package support

---

### Phase 7 — Project Tooling

**Status:** In Progress

- Project configuration foundation
- Project management
- Workspace indexing
- Project-wide search
- Project-wide replace
- Build integration
- Task execution framework
- Project configuration support
- Development workflow tooling

---

### Phase 8 — IDE Foundation

**Status:** Planned

- Multiple buffers
- Split views
- Integrated terminal
- Workspace coordination
- Project-centric editing
- Advanced navigation systems
- IDE infrastructure layer

---

### Phase 9 — Source Control Platform

**Status:** Planned

- Git diff viewer
- Git log viewer
- Commit workflows
- Branch switching
- Git stash support
- Inline change indicators
- Repository dashboard
- Advanced repository tooling

---

### Phase 10 — Language Ecosystem

**Status:** Planned

- Python support
- Java support
- JavaScript support
- TypeScript support
- Rust support
- Go support
- Markdown support
- JSON support
- Language package architecture

---

### Phase 11 — Intelligent IDE Services

**Status:** Planned

- Language Server Protocol (LSP)
- Code completion
- Diagnostics
- Symbol navigation
- Refactoring tools
- Intelligent code analysis
- Development assistance services

---

### Phase 12 — Advanced Platform Services

**Status:** Planned

- Remote development
- Workspace synchronization
- Automation services
- Platform integrations
- Distributed development workflows
- Advanced developer tooling

---

### Phase 13 — Claw Ecosystem

**Status:** Vision

- Plugin marketplace
- Community extensions
- Extension discovery platform
- Package distribution system
- Developer ecosystem
- Collaborative development features
- AI-assisted workflows
- Modern terminal-first development platform

---

## Implemented Features

### Phase 1 — Core Editor Foundation

#### Editor Core

- Modular editor architecture
- Central editor lifecycle management
- Main event loop
- Application startup and shutdown workflows
- Terminal-first editing environment
- POSIX-compliant implementation
- GCC and Clang compatibility
- Portable build system

#### Input System

- Raw terminal mode
- Real-time keyboard input processing
- Escape sequence decoding
- Arrow key navigation
- Control-key shortcuts
- Alt-key support
- Safe terminal restoration
- Terminal state recovery

#### Buffer Engine

- Dynamic text buffer
- Row-based document model
- Character insertion and deletion
- Forward deletion support
- Dynamic row allocation and removal
- Automatic buffer growth
- Efficient memory management
- Document modification tracking

#### Cursor System

- Horizontal and vertical navigation
- Boundary-aware movement
- Position tracking
- Column preservation
- Cursor correction logic
- Viewport-aware positioning

#### Multi-Line Editing

- New line creation
- Line splitting and merging
- Multi-line navigation
- Dynamic row management
- Cursor-aware line operations

#### File System

- Open existing files
- Create new files
- Save files
- Save-As workflow
- Filename tracking
- Persistent storage
- Unsaved change detection
- Exit confirmation protection
- Automatic syntax re-detection

#### Viewport Engine

- Vertical scrolling
- Horizontal scrolling
- Dynamic viewport updates
- Cursor-aware viewport tracking
- Large-file navigation
- Terminal resize adaptation
- Screen position calculations

#### Rendering Engine

- Full-screen rendering
- Real-time screen refresh
- Cursor rendering
- Line number rendering
- Search highlighting rendering
- Syntax highlighting rendering
- Welcome screen rendering
- Status bar rendering
- ANSI terminal output

#### User Interface

- Welcome screen
- Status bar
- Search prompt
- Replace prompt
- Save-As prompt
- Context-aware status messages
- File information display
- Cursor position display
- Modification indicators

#### Syntax Platform

- C syntax highlighting
- Header file highlighting
- Keyword highlighting
- Type highlighting
- Number highlighting
- String highlighting
- Character highlighting
- Operator highlighting
- Single-line comments
- Multi-line comments
- Preprocessor directives
- Language detection
- Highlight caching
- ANSI color rendering

---

### Phase 2 — Productivity Foundation

#### Search & Navigation

- Search prompt
- Match discovery
- Match highlighting
- Current match tracking
- Match counting
- Search statistics
- Next-match navigation
- Previous-match navigation
- Search cancellation

#### Search & Replace

- Replace current match
- Replace all matches
- Match tracking
- Replacement statistics
- Automatic next-match navigation
- Document-wide replacement

#### History System

- Undo support
- Redo support
- Edit history tracking
- Multi-line history support
- Action recording
- Undo stack management
- Redo stack management
- History state control

#### Configuration Platform

- INI-style configuration parser
- User configuration support
- XDG configuration support
- Layered configuration loading
- Runtime settings access
- Configurable tab width
- Toggleable line numbers
- Toggleable status bar
- Toggleable welcome screen

#### Keymap Platform

- Custom key bindings
- Multiple keymap profiles
- Keymap file support
- Keyboard layout abstraction
- Command lookup engine
- Shortcut customization

#### Command Framework

- Command dispatch architecture
- Centralized action routing
- Keymap integration
- Editor action execution
- Extensible command system

---

### Phase 3 — Developer Platform

#### Plugin Runtime

- Runtime plugin loading
- Dynamic module support
- Plugin discovery
- Plugin registration
- Plugin lifecycle management
- Plugin API
- Third-party plugin support

#### Event System

- Startup hooks
- Save hooks
- Search hooks
- Exit hooks
- Event dispatch integration

#### Plugin Management

- Interactive Plugin Dashboard
- Plugin Details View
- Plugin enable / disable controls
- Runtime plugin reloading
- Plugin health monitoring
- Plugin statistics tracking
- Plugin state management

#### Plugin Controls

- Runtime plugin toggling
- AutoLoad configuration
- Event Hook configuration
- Plugin reload controls
- Plugin health checks
- Dashboard status reporting

#### Included Plugins

- Hello Plugin
- Statistics Plugin
- Format Plugin

#### Git Integration

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

#### Git Workspace Features

- Git Blame viewer
- Read-only Git view
- Temporary blame-file generation
- Repository-aware workflows
- Git operation feedback
- Return-to-editor workflow

---

### Phase 4 — Advanced Editing Layer

#### Completed

- Auto indentation and smart indentation engine
- Indentation-aware new line creation
- Auto-closing brackets and delimiter skip logic
- Bracket matching and visual highlighting
- Smart backspace handling (indentation zone aware)
- Smart tab and unindent behavior (`Tab` / `Shift-Tab`)
- Duplicate line operations (`Alt+D`)
- Move line up/down (`Shift+↑` / `Shift+↓`)
- Comment / Uncomment support (`Alt+/`)
- Line join operations (`Alt+J`) and line split
- Word-based navigation (`Ctrl+←` / `Ctrl+→`)
- Line boundary navigation (`Home` / `End`)
- Jump to matching delimiter (`Alt+]`)

---

### Phase 5 — Workspace Environment

#### Completed

- Dynamic workspace root detection (`.claw`, `.git`, or working directory)
- Workspace configuration files (`.claw/workspace.conf` with cascading overrides)
- Recent files & workspace history tracker (MRU ordering & persistence)
- Editor session serialization & automatic restoration (`session.claw`)
- Go-to-line navigation workflow (`Alt+G` / `:line`)
- Comprehensive bookmark system with gutter indicator & cyclic jumping (`Alt+B`, `Alt+Down`, `Alt+Up`, `Alt+C`)
- Interactive Quick File Switcher modal (`Alt+Q`)
- Built-in Workspace File Explorer dashboard (`Alt+E`)
- Status bar workspace branding & active bookmark indicators

---

### Phase 6 — Customization Platform

#### In Progress

- Theme engine and color scheme definitions
- Custom user theme loading from `~/.config/claw/themes/`
- Status bar layout and segment customization
- User appearance profiles and runtime switching

---

## Phase Progress

|  Phase   |        Focus Area           | Status      |                     Description                            |
|----------|-----------------------------|-------------|------------------------------------------------------------|
| Phase 1  | Core Editor Foundation      | Complete    | Core editing architecture and terminal interaction systems |
| Phase 2  | Productivity Foundation     | Complete    | Essential productivity and workflow features               |
| Phase 3  | Developer Platform          | Complete    | Git integration, plugins, and developer tooling            |
| Phase 4  | Advanced Editing Layer      | Complete    | Intelligent editing and automation capabilities            |
| Phase 5  | Workspace Environment       | Complete    | Project workspaces, file explorer, session & navigation    |
| Phase 6  | Customization Platform      | Complete    | Themes, profiles, and editor personalization               |
| Phase 7  | Project Tooling             | In Progress | Development automation and project utilities               |
| Phase 8  | IDE Foundation              | Planned     | Multi-buffer editing and IDE infrastructure                |
| Phase 9  | Source Control Expansion    | Planned     | Advanced repository management capabilities                |
| Phase 10 | Language Ecosystem          | Planned     | Multi-language support and tooling                         |
| Phase 11 | Intelligent IDE Services    | Planned     | Code intelligence and developer assistance                 |
| Phase 12 | Advanced Platform Services  | Planned     | Platform integrations and remote workflows                 |
| Phase 13 | Claw Ecosystem              | Vision      | Community-driven extension ecosystem                       |

---

## LICENSE

MIT License

Copyright (c) 2026 Aryan Gupta & Pranav Chauhan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## Authors

Aryan Gupta
Pranav Chauhan

Project: Claw
Language: C
Platform: Linux
