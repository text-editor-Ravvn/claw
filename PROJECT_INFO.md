# Claw: Project Architecture, Technical Reference & Developer Guide

## 1. Executive Summary & Architecture Overview

**Claw** is a modular, high-performance, terminal-first text editor and developer environment implemented in pure **C11** (with POSIX.1-2008 extensions). It delivers modern IDE-grade workflows directly inside the terminal: multi-buffer management, dynamic horizontal/vertical split-pane layouts, full syntax highlighting, Git source control, language server client services (LSP), debugger adapter protocol (DAP), an integrated virtual terminal shell, workspace management, project scaffolding templates, and a digitally signed package ecosystem (.cpkg).

### Core Design Principles
- **Zero-Dependency Core**: Compiles with a single C compiler (`gcc` or `clang`) with zero third-party GUI or ncurses dependencies.
- **Pure Terminal Architecture**: All rendering uses direct ANSI/VT100 escape codes with double-buffering and incremental redrawing to prevent screen tearing.
- **Strict Memory Safety**: Zero leaks, bounded allocations, explicit ownership semantics, and verified with AddressSanitizer and UndefinedBehaviorSanitizer (`-fsanitize=address,undefined`).
- **Phase-Modular Hierarchy**: Developed across 13 evolutionary phases, each maintaining standalone unit test coverage and clean API boundaries.

---

## 2. Directory Layout & File Map

```
claw/
├── include/                   # Public subsystem C header definitions
│   ├── bookmark.h             # Line bookmark navigation and toggling
│   ├── brackets.h             # Smart delimiter matching & pair insertion
│   ├── buffer.h               # Core document text buffer structures & row arrays
│   ├── buffer_manager.h       # Multi-buffer registry, cycling, and global sync
│   ├── command.h              # Key-to-command dispatch engine
│   ├── config.h               # Editor runtime configuration settings
│   ├── cursor.h               # Cursor coordinates and navigation functions
│   ├── dap.h                  # Debug Adapter Protocol (DAP) client & session
│   ├── editor.h               # Editor core loop, prompt modals & lifecycle
│   ├── explorer.h             # Workspace interactive file explorer modal
│   ├── file_switcher.h        # Quick fuzzy/prefix file switcher modal
│   ├── fileio.h               # File reading and binary atomic saving
│   ├── git.h                  # Git CLI wrappers, staging, status & gutter
│   ├── highlight.h            # Lexical syntax highlighter & token engines
│   ├── history.h              # Transactional Undo/Redo stack engine
│   ├── indent.h               # Auto-indentation & smart backspace
│   ├── input.h                # Key event decoding & terminal input parser
│   ├── keymap.h               # Custom & default keybinding definitions
│   ├── layout.h               # Status bar & screen UI toggle settings
│   ├── lsp.h                  # Language Server Protocol client & diagnostics
│   ├── marketplace.h          # Extension marketplace & package registry
│   ├── package.h              # .cpkg bundle parser, SHA-256 & HMAC signatures
│   ├── plugin.h               # Dynamic plugin loader & event dispatcher
│   ├── plugin_api.h           # Safe C API exposed to plugins
│   ├── plugin_commands.h      # Plugin shell commands & diagnostics
│   ├── project.h              # Project build/test runner & diagnostics parser
│   ├── rawmode.h              # POSIX termios raw terminal mode control
│   ├── recent.h               # MRU recent files and workspace tracking
│   ├── remote.h               # Remote VFS parser, SSH/cURL fetch & save
│   ├── render.h               # ANSI terminal rendering & layout composer
│   ├── search.h               # Whole-word search, replace & replace-all
│   ├── session.h              # Workspace session auto-save & recovery
│   ├── split.h                # Split-screen panes (horizontal & vertical)
│   ├── statusbar.h            # Status bar formatting and layout
│   ├── string_utils.h         # Memory-safe string manipulation helpers
│   ├── symbols.h              # Symbol indexer & fuzzy navigation modal
│   ├── templates.h            # Project scaffolding generator (C, Rust, Go, Python)
│   ├── terminal.h             # Built-in interactive pseudo-terminal panel
│   ├── theme.h                # Color themes, custom palette editor & import/export
│   └── viewport.h             # Scrolling coordinates, screen rows & gutter sizing
├── src/                       # Subsystem implementations (C11 source files)
│   ├── main.c                 # Entry point, argument parsing & editor bootstrap
│   ├── buffer/                # Buffer, Row, Cursor, BufferManager
│   ├── command/               # Command dispatch switch
│   ├── dap/                   # DAP protocol, stepping, variables, views
│   ├── editor/                # Editor lifecycle, loop, modals
│   ├── fileio/                # File reading/writing
│   ├── git/                   # Git operations and Git views
│   ├── highlight/             # Syntax definitions and tokenizers
│   ├── history/               # Undo/Redo delta records
│   ├── input/                 # Rawmode & key sequence parsing
│   ├── keymap/                # Keymap bindings & parser
│   ├── layout/                # Layout settings & split engine
│   ├── lsp/                   # LSP client, protocol, views
│   ├── marketplace/           # Package engine, hash, crypto, views
│   ├── plugin/                # Plugin engine & APIs
│   ├── remote/                # Remote protocol & VFS
│   ├── render/                # ANSI terminal screen rendering
│   ├── search/                # Search & replace engine
│   ├── templates/             # Project template generator
│   ├── terminal/              # Integrated terminal panel
│   ├── theme/                 # Themes, Palette Maker & Manager
│   ├── ui/                    # Status bar
│   ├── utils/                 # String utilities
│   └── workspace/             # Workspace, Session, Explorer, Switcher, Symbols
├── tests/                     # 14+ automated unit & integration test suites
│   ├── buffer_test.c          # Buffer row insertion, deletion, newline tests
│   ├── file_test.c            # File loading, saving, CRLF preserving tests
│   ├── input_test.c           # Key sequence parsing & modifier tests
│   ├── workspace_test.c       # Workspace root detection & config tests
│   ├── session_test.c         # Session persistence & document restoration
│   ├── bookmark_test.c        # Bookmark toggle & navigation tests
│   ├── phase5_integration_test.c # Workspace explorer & switcher integration
│   ├── project_test.c         # Build task execution & error line parsing
│   ├── phase8_test.c          # BufferManager, SplitPanes, Terminal, Symbols
│   ├── git_test.c             # Git branch, stash, diff, gutter indicators
│   ├── phase10_test.c         # Syntax highlighting & theme tests
│   ├── phase11_test.c         # LSP completion, hover, definition, diagnostics
│   ├── phase12_test.c         # DAP debugging, remote VFS, project templates
│   ├── phase13_test.c         # Package manager, SHA256, HMAC, marketplace
│   └── bugfixes_test.c        # Regression tests for all resolved bugs
├── Makefile                   # High-level build system with ASan and test targets
├── BUGS_AND_MALFUNCTIONS.md   # Complete catalog of all fixed bugs across phases
├── TESTING_MANUAL.md          # Step-by-step interactive testing guide
└── PROJECT_INFO.md            # Central reference documentation (this file)
```

---

## 3. Subsystem Breakdown & Architecture

### 3.1 Terminal Input & Raw Mode (`src/input/`)
- **Raw Mode**: Disables `ECHO`, `ICANON`, `ISIG`, and `IEXTEN` via `termios`. Sets `VMIN = 0`, `VTIME = 1` for 100ms non-blocking polling.
- **Escape Sequence Decoder**: Parses ANSI/VT escape sequences into normalized `KeyEvent` structs containing:
  - `key`: Normalized key code (`ARROW_UP`, `ARROW_DOWN`, `HOME_KEY`, `END_KEY`, `PAGE_UP`, `PAGE_DOWN`, `DELETE_KEY`, `BACKTAB`, `KEY_F1`-`KEY_F12`, ASCII).
  - `modifiers`: Bitfield flag (`MOD_CTRL = 1`, `MOD_ALT = 2`, `MOD_SHIFT = 4`).

### 3.2 Text Buffer & Row Engine (`src/buffer/`)
- **Row Representation (`Row`)**: Dynamic character array (`chars`), size in bytes (`size`), syntax highlight token array (`hl`), and highlight cache length (`hlSize`).
- **Buffer (`Buffer`)**: Collection of rows (`rows`), total row count (`numRows`), modified flag (`modified`), and trailing newline indicator (`endsWithNewline`).
- **Cursor (`Cursor`)**: Zero-indexed `(x, y)` coordinate tracking document column and row. Clamped to `[0, row->size]` and `[0, numRows - 1]`.

### 3.3 Transactional Undo/Redo Engine (`src/history/`)
- **History Record**: Records delta operations (`HIST_INSERT`, `HIST_DELETE`, `HIST_REPLACE`) with row, column, character string, and cursor position.
- **Smart Grouping**: Consecutive character typing in the same word is grouped into single atomic undo steps. Smart backspace indentation deletion is recorded cleanly without stack corruption.

### 3.4 Viewport & ANSI Rendering Engine (`src/viewport/`, `src/render/`)
- **Viewport**: Tracks terminal window dimensions (`screenRows`, `screenCols`), scrolling offsets (`rowOffset`, `colOffset`), and gutter column width (`gutterWidth`).
- **Incremental Refresh**: Draws only dirty lines using `\033[K` (erase to end of line). Forces a complete screen re-clear (`\033[2J\033[H`) only upon resize or modal entry/exit (`renderForceClear()`).
- **Gutter Indicators**: Supports multi-priority gutter flags: DAP execution pointer (`>`), DAP breakpoint (`B`), LSP error (`E`), LSP warning (`W`), Bookmark (`*`), Git added (`+`), Git modified (`~`), Git deleted (`_`), or line numbers.

### 3.5 Multi-Buffer Manager (`src/buffer/buffer_manager.c`)
- **Buffer Registry (`BufferManager`)**: Manages up to 64 open file buffers (`FileBuffer`).
- **Synchronization**: Two-way synchronization between global active editor state (`buffer`, `cursor`, `viewport`, `currentFile`) and buffer slots (`bufferManagerSyncFromGlobal()`, `bufferManagerSyncToGlobal()`).
- **Pane Isolation**: When split panes are active, buffer cycling (`Ctrl+Tab` / `Ctrl+Shift+Tab`) skips buffers currently displayed in the other pane.

### 3.6 Dynamic Split Layout (`src/layout/split.c`)
- **Split Modes**:
  - `SPLIT_NONE`: Full-window editor pane.
  - `SPLIT_VERTICAL`: Left (Pane 0) and Right (Pane 1) panes separated by a vertical `│` divider.
  - `SPLIT_HORIZONTAL`: Top (Pane 0) and Bottom (Pane 1) panes separated by an inverse-video horizontal status line.
- **Pane Independence**: Each pane tracks its own `bufferIndex`, `cursorX`, `cursorY`, `rowOffset`, and `colOffset`.
- **Divider Integrity**: Tab characters (`\t`) are expanded visually to 4-column stops with column clamping to guarantee zero divider bleed or line wrap.

### 3.7 Integrated Terminal Panel (`src/terminal/terminal.c`)
- **Bottom Panel**: Toggled via ``Ctrl+` ``. Allocates a configurable bottom strip of the terminal (default 8 rows).
- **Built-in Shell Commands**: Builtins include `pwd`, `cd`, `build`, `test`, `clear`, `exit`, `help`. All external commands are executed via standard shell pipes with output captured directly into the terminal scrollback buffer.
- **Key Navigation**: Full command history navigation (`Up`/`Down`), line editing (`Left`/`Right`/`Home`/`End`), page scrolling (`PageUp`/`PageDown`), and cancel (`Ctrl+C`).

### 3.8 Git Source Control Platform (`src/git/`)
- **Status Engine (`gitRefreshRepo`)**: Analyzes repository root, current branch, upstream tracking, ahead/behind count, stash count, and porcelain status. Accurately identifies dual-state staged/unstaged files (`MM`, `AM`, `MD`) and untracked files (`??`).
- **Interactive Modals**:
  - **Git Dashboard (`Ctrl+Shift+G` / `Alt+5`)**: Interactive stage/unstage (`Space`), commit (`c`), push/pull (`p`/`P`), and branch/stash shortcuts.
  - **Diff Viewer (`Ctrl+Shift+K`)**: Color-coded side-by-side / unified diff view for working tree or staged changes.
  - **Commit Log (`Ctrl+Shift+L`)**: Browse commit history with author, date, and commit diff inspector (`Enter`).
  - **Branch Manager (`Ctrl+Shift+R`)**: Create (`n`), switch (`Enter`), and delete (`d`) branches.
  - **Stash Manager (`Ctrl+Shift+Z`)**: Push (`p`), apply (`a`), pop (`Enter`), or drop (`d`) stashes.
  - **Commit Dialog (`Ctrl+Shift+C`)**: Multi-line commit message input with immediate validation.

### 3.9 Intelligent IDE Services (LSP) (`src/lsp/`)
- **Capabilities**: Client abstraction for Language Server Protocol.
- **Code Completion (`Ctrl+Space`)**: Triggered at cursor, filters symbol proposals, inserts completions on `Enter`.
- **Go to Definition (`Ctrl+]`)**: Resolves symbol declaration location and opens the target file and line.
- **Hover Documentation (`Alt+K`)**: Displays type signatures and markdown docstrings in an overlay box.
- **Find References (`Alt+Shift+F`)**: Gathers all symbol usages across the workspace into a jump list.
- **Code Actions (`Alt+Enter`)**: Offers quick-fix refactorings and source fixes.
- **Diagnostics**: Real-time error/warning gutter markers and status bar notifications.

### 3.10 Advanced Platform Services (DAP, Remote, Templates) (`src/dap/`, `src/remote/`, `src/templates/`)
- **Debugger Adapter Protocol (DAP)**: Breakpoints (`F9`), launch/continue (`F5`), stop (`Shift+F5`), pause (`F6`), step-over (`F10`), step-into (`F11`), step-out (`Shift+F11`), and full debugger dashboard (`Alt+V`).
- **Remote VFS Access (`Ctrl+Shift+U`)**: Opens remote files via URI (`ssh://user@host:path` or `https://...`), caches locally, tracks remote edits, and syncs back.
- **Project Scaffolding Wizard (`Alt+N`)**: Interactive modal generator creating complete project scaffolding for C (Makefile, include, src), Python, Rust (Cargo.toml, src/main.rs), Go (go.mod, main.go), or Web.

### 3.11 Extension Ecosystem & Marketplace (`src/marketplace/`)
- **Package Archive (.cpkg)**: Binary archive format containing a JSON manifest and package payload.
- **Digital Signatures**: Cryptographically verified using SHA-256 digests and HMAC-SHA256 signatures.
- **Marketplace Modal (`Ctrl+Shift+M`)**: Interactive extension catalogue with search, category filtering (themes, languages, tools, linters), install, upgrade, and uninstall lifecycles.

---

## 4. Complete Core Data Structures

### Editor & Buffer Core (`include/buffer.h`, `include/cursor.h`, `include/viewport.h`)
```c
typedef struct {
    int size;               /* Length of chars in bytes */
    char *chars;            /* Dynamically allocated string */
    unsigned char *hl;      /* Syntax highlight token array */
    int hlSize;             /* Allocated length of hl array */
    int hlOpenState;        /* Multi-line comment/string open state */
} Row;

typedef struct {
    int numRows;            /* Total rows in buffer */
    Row *rows;              /* Dynamic array of Row */
    int modified;           /* Dirty flag (unsaved edits) */
    int endsWithNewline;    /* Preserves original EOF newline style */
} Buffer;

typedef struct {
    int x;                  /* Column offset (0-indexed) */
    int y;                  /* Row offset (0-indexed) */
} Cursor;

typedef struct {
    int screenRows;         /* Terminal height in characters */
    int screenCols;         /* Terminal width in characters */
    int rowOffset;          /* Topmost visible row index */
    int colOffset;          /* Leftmost visible column index */
    int gutterWidth;        /* Left gutter width for line numbers/markers */
} Viewport;
```

### Multi-Buffer & Split Layout (`include/buffer_manager.h`, `include/split.h`)
```c
typedef struct {
    int id;                                /* Unique buffer ID */
    int active;                            /* Active slot flag */
    char filename[512];                    /* Full path or empty for [Untitled] */
    char displayName[128];                 /* Base name for status bar / tabs */
    Buffer doc;                            /* Complete document rows */
    Cursor cursor;                         /* Cursor position */
    int rowOffset;                         /* Viewport row offset */
    int colOffset;                         /* Viewport col offset */
} FileBuffer;

typedef struct {
    FileBuffer buffers[BUFFER_MAX_COUNT];  /* Max 64 open buffers */
    int count;                             /* Total count of buffers */
    int activeIndex;                       /* Current active buffer index */
} BufferManager;

typedef enum {
    SPLIT_NONE = 0,
    SPLIT_HORIZONTAL,
    SPLIT_VERTICAL
} SplitMode;

typedef struct {
    int x, y;                              /* Pane screen starting position */
    int width, height;                     /* Pane dimensions */
    int bufferIndex;                       /* Index of FileBuffer displayed */
    int cursorX, cursorY;                  /* Pane-isolated cursor coordinates */
    int rowOffset, colOffset;              /* Pane-isolated scroll offsets */
} EditorPane;

typedef struct {
    SplitMode mode;                        /* SPLIT_NONE, HORIZONTAL, VERTICAL */
    EditorPane panes[2];                   /* Pane 0 and Pane 1 */
    int activePane;                        /* 0 or 1 */
} SplitLayout;
```

### Key Event & Keymap (`include/input.h`, `include/keymap.h`)
```c
typedef struct {
    int key;                               /* Key code (ASCII or special enum) */
    int modifiers;                         /* MOD_CTRL | MOD_ALT | MOD_SHIFT */
} KeyEvent;

typedef struct {
    int action;                            /* EditorAction enum (CMD_*) */
    int key;                               /* Trigger key */
    int modifiers;                         /* Required modifier flags */
} KeyBinding;
```

---

## 5. Complete Default Keybindings Reference

### File & Core Operations
| Shortcut | Action Enum | Description |
|---|---|---|
| `Ctrl+S` | `CMD_SAVE` | Save current file (or open Save-As dialog if untitled) |
| `Ctrl+X` | `CMD_QUIT` | Exit Claw (prompts confirmation if unsaved changes exist) |
| `Alt+W` | `CMD_WORKSPACE_OPEN` | Detect & open workspace root from current directory |
| `Alt+S` | `CMD_SESSION_SAVE` | Explicitly serialize current session to `.claw/session.claw` |
| `Alt+R` | `CMD_SESSION_RESTORE` | Restore previous workspace session & open files |

### Text Navigation & Editing
| Shortcut | Action Enum | Description |
|---|---|---|
| `Arrow Keys` | — | Move cursor up, down, left, right |
| `Ctrl+Left` | `CMD_WORD_LEFT` | Jump cursor one word to the left |
| `Ctrl+Right` | `CMD_WORD_RIGHT` | Jump cursor one word to the right |
| `Home` | `CMD_HOME` | Jump to start of line |
| `End` | `CMD_END` | Jump to end of line |
| `Alt+G` | `CMD_GOTO_LINE` | Open Go-To-Line prompt modal (`Ln:Col`) |
| `Alt+]` | `CMD_GOTO_BRACKET` | Jump cursor to matching delimiter `()`, `[]`, `{}` |
| `Ctrl+Z` | `CMD_UNDO` | Undo last editing action |
| `Ctrl+Y` | `CMD_REDO` | Redo previously undone action |
| `Alt+D` | `CMD_DUPLICATE_LINE` | Duplicate current line directly below |
| `Shift+Up` | `CMD_MOVE_LINE_UP` | Swap current line upward |
| `Shift+Down` | `CMD_MOVE_LINE_DOWN` | Swap current line downward |
| `Alt+/` | `CMD_COMMENT_LINE` | Toggle single-line / block comment on current line |
| `Alt+J` | `CMD_JOIN_LINES` | Join current line with subsequent line |
| `Tab` | — | Smart indentation (4 spaces / tab stop) |
| `Shift+Tab` | `CMD_UNINDENT` | Outdent / remove leading indentation spaces |
| `Backspace` | — | Delete previous char or smart backspace indent |
| `Delete` | — | Delete forward character |

### Search & Replace
| Shortcut | Action Enum | Description |
|---|---|---|
| `Ctrl+F` | `CMD_SEARCH` | Open interactive search prompt (Esc cancels) |
| `Ctrl+R` | `CMD_REPLACE` | Open interactive replace prompt |
| `Ctrl+A` (in Replace) | `CMD_REPLACE_ALL` | Replace all occurrences in entire document |
| `Ctrl+N` | `CMD_NEXT_MATCH` | Jump to next search match |
| `Ctrl+P` | `CMD_PREV_MATCH` | Jump to previous search match |
| `Esc` | — | Clear search highlight & exit search prompt |

### Multi-Buffer & Split Panes (IDE Foundation)
| Shortcut | Action Enum | Description |
|---|---|---|
| `Ctrl+\` | `CMD_SPLIT_VERTICAL` | Open / toggle vertical split (left/right) |
| `Ctrl+-` | `CMD_SPLIT_HORIZONTAL` | Open / toggle horizontal split (top/bottom) |
| `Alt+O` | `CMD_PANE_SWITCH` | Switch focus between Pane 0 and Pane 1 |
| `Ctrl+W` | `CMD_BUFFER_CLOSE` | Close active split pane (or close buffer if unsplit) |
| `Ctrl+Tab` | `CMD_BUFFER_NEXT` | Cycle to next open buffer in active pane |
| `Ctrl+Shift+Tab` | `CMD_BUFFER_PREV` | Cycle to previous open buffer in active pane |
| `Ctrl+1` .. `Ctrl+9` | `CMD_BUFFER_1..9` | Jump directly to buffer slot 1 through 9 |
| ``Ctrl+` `` | `CMD_TERMINAL_TOGGLE`| Toggle integrated bottom terminal shell panel |
| `Ctrl+Shift+S` | `CMD_SYMBOL_SEARCH` | Open Symbol Navigator modal (jump to functions/types) |

### Workspace & Project Navigation
| Shortcut | Action Enum | Description |
|---|---|---|
| `Alt+Q` | `CMD_QUICK_SWITCH` | Open quick file switcher modal with fuzzy filter |
| `Alt+E` | `CMD_FILE_EXPLORER` | Open workspace file explorer modal |
| `Alt+B` | `CMD_BOOKMARK_TOGGLE`| Toggle bookmark indicator (`*`) on current line |
| `Alt+Down` | `CMD_BOOKMARK_NEXT` | Jump to next bookmarked line |
| `Alt+Up` | `CMD_BOOKMARK_PREV` | Jump to previous bookmarked line |
| `Alt+C` | `CMD_BOOKMARK_CLEAR`| Clear all bookmarks across document |
| `Ctrl+Shift+B` | `CMD_PROJECT_BUILD` | Run workspace build task (`make` / `cargo build`) |
| `Ctrl+Shift+T` | `CMD_PROJECT_TEST` | Run workspace test task (`make test` / `cargo test`) |
| `Ctrl+Shift+O` | `CMD_PROJECT_OUTPUT`| View build/task console output log |
| `Ctrl+Shift+D` | `CMD_PROJECT_DIAGNOSTICS`| Open build compiler error diagnostics navigator |
| `Ctrl+Down` | `CMD_PROJECT_DIAGNOSTICS_NEXT`| Jump to next build error |
| `Ctrl+Up` | `CMD_PROJECT_DIAGNOSTICS_PREVIOUS`| Jump to previous build error |
| `Ctrl+Shift+P` | `CMD_PROJECT_DASHBOARD`| Open project task & configuration dashboard |

### Source Control (Git Platform)
| Shortcut | Action Enum | Description |
|---|---|---|
| `Ctrl+Shift+G` / `Alt+5`| `CMD_GIT_DASHBOARD` | Open full interactive Git repository dashboard |
| `Ctrl+Shift+K` | `CMD_GIT_DIFF` | Open visual diff viewer modal (working tree / staged) |
| `Ctrl+Shift+L` | `CMD_GIT_LOG` | Open interactive Git commit history viewer |
| `Ctrl+Shift+C` | `CMD_GIT_COMMIT` | Open Git commit dialog with multi-line message |
| `Ctrl+Shift+R` | `CMD_GIT_BRANCH` | Open Git branch switcher & creation modal |
| `Ctrl+Shift+Z` | `CMD_GIT_STASH` | Open Git stash management modal |
| `Ctrl+G` | `CMD_GIT_ADD` | Stage active file in git index |
| `Ctrl+U` | `CMD_GIT_UNSTAGE` | Unstage active file from git index |
| `Ctrl+T` | `CMD_GIT_RESTORE` | Discard working tree changes on active file |
| `Ctrl+B` | `CMD_GIT_BLAME` | Open Git blame view for current file |

### Intelligent IDE Services (LSP)
| Shortcut | Action Enum | Description |
|---|---|---|
| `Ctrl+Space` | `CMD_LSP_COMPLETION` | Trigger code completion popup at cursor |
| `Ctrl+]` | `CMD_LSP_DEFINITION` | Jump to symbol definition |
| `Alt+K` | `CMD_LSP_HOVER` | Show hover type information & docstring popup |
| `Alt+Shift+F` | `CMD_LSP_REFERENCES` | Find all references across project |
| `Alt+Enter` | `CMD_LSP_CODE_ACTION`| Open code actions & quick-fix suggestions |
| `Alt+0` | `CMD_LSP_DIAGNOSTICS`| Display count of active document diagnostics |

### Debugger (DAP), Remote & Ecosystem
| Shortcut | Action Enum | Description |
|---|---|---|
| `F5` | `CMD_DAP_START_CONTINUE`| Start debugging session or continue execution |
| `Shift+F5` | `CMD_DAP_STOP` | Terminate active debugging session |
| `F6` | `CMD_DAP_PAUSE` | Pause running debug target |
| `F9` | `CMD_DAP_BREAKPOINT_TOGGLE`| Toggle breakpoint on current line (`B` indicator) |
| `F10` | `CMD_DAP_STEP_OVER` | Step over next instruction / line |
| `F11` | `CMD_DAP_STEP_INTO` | Step into function call |
| `Shift+F11` | `CMD_DAP_STEP_OUT` | Step out of current function |
| `Alt+V` | `CMD_DAP_DASHBOARD` | Open full DAP debugger dashboard (call stack & vars) |
| `Ctrl+Shift+U` | `CMD_REMOTE_OPEN` | Open remote file modal (`ssh://`, `https://`) |
| `Alt+N` | `CMD_PROJECT_NEW` | Launch interactive Project Template Scaffolding Wizard |
| `Ctrl+Shift+M` | `CMD_MARKETPLACE` | Open Extension & Package Marketplace |
| `Alt+Shift+M` | `CMD_PACKAGE_MANAGER`| Open Package Manager view |

### Theme & UI Customization
| Shortcut | Action Enum | Description |
|---|---|---|
| `Alt+T` | `CMD_THEME_NEXT` | Cycle to next theme with status bar notification |
| `Alt+1` | `CMD_STATUSBAR_SETTINGS`| Open status bar display settings modal |
| `Alt+2` | `CMD_LAYOUT_SETTINGS` | Open layout settings modal (line numbers, gutter) |
| `Alt+3` | `CMD_THEME_MAKER` | Open Theme Maker palette editor |
| `Alt+4` | `CMD_THEME_MANAGER` | Open Theme Manager (export, import, activate) |

---

## 6. Build Targets & Test Suite Specification

### Primary Makefile Targets
- `make` or `make all`: Compiles the optimized `claw` binary.
- `make asan`: Cleans and compiles `claw` with AddressSanitizer and UndefinedBehaviorSanitizer (`-fsanitize=address,undefined -fno-omit-frame-pointer`).
- `make test`: Compiles and executes all 14 standalone test suites sequentially with automated assertion validation.
- `make clean`: Removes all `.o` objects, test binaries, and compiled target executables.
- `make rebuild`: Full clean and rebuild.

### Automated Test Matrix (`make test`)
1. `buffer_test`: Document row insertion, deletion, newline splitting, and cursor bounds.
2. `file_test`: File loading, atomic saving, CRLF line ending preservation.
3. `input_test`: Terminal rawmode escape sequence decoding and modifier bitmasks.
4. `workspace_test`: Workspace root path detection and configuration loading.
5. `session_test`: Workspace session serialization and recovery.
6. `bookmark_test`: Line bookmark toggling, cycling, and clearing.
7. `phase5_test`: Workspace Explorer and Quick File Switcher integration.
8. `project_test`: Project task execution and compiler diagnostic line parsing.
9. `phase8_test`: Multi-buffer manager, horizontal/vertical split panes, terminal panel, symbol indexing.
10. `git_test`: Git repository detection, branch management, stashing, diff generation, and gutter markers.
11. `phase10_test`: Highlighting engine for C, Python, JavaScript, HTML, Markdown, and custom theme engine.
12. `phase11_test`: LSP protocol framing, code completion, definitions, hover, references, and code actions.
13. `phase12_test`: DAP debugging protocol, breakpoints, stepping, remote VFS, and project scaffolding templates.
14. `phase13_test`: SHA-256 hash engine, HMAC digital signatures, .cpkg package bundling, and marketplace catalogue.
15. `bugfixes_test`: Regression suite verifying all 30+ fixed bugs (search infinite loops, buffer overreads, smart backspace undo, horizontal divider alignment, comment toggle search refresh, etc.).

---

## 7. Extension Package (.cpkg) Format & Plugin Specification

### Package Archive Structure
A `.cpkg` extension archive is structured as:
```
[4 bytes: MAGIC 'CPKG']
[4 bytes uint32: Manifest JSON Length]
[N bytes: Manifest JSON (UTF-8)]
[Binary payload: Plugin source, assets, or configuration]
```

### Manifest Schema (`package.json`)
```json
{
  "name": "claw-theme-nord",
  "version": "1.2.0",
  "description": "Arctic Nord color palette for Claw",
  "author": "Developer <dev@example.com>",
  "category": "themes",
  "main": "nord.theme",
  "checksum": "a4f8...64-char-sha256...",
  "signature": "3c9b...64-char-hmac-sha256..."
}
```

### Digital Signatures & Security
Packages are verified before installation using SHA-256 checksum calculation over the payload, followed by HMAC-SHA256 signature verification against authorized publisher keys. Corrupted or tampered packages fail verification and are rejected automatically.
