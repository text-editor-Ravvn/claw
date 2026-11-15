# Claw: Complete Interactive Testing Manual

This manual provides direct, standalone instructions for testing every feature in the **Claw** terminal editor. Each module is self-contained: start from the specified shell command, follow the exact keystrokes, and observe the expected behavior.

---

## Table of Contents
- [Module 0: Compilation & Setup](#module-0-compilation--setup)
- [Module 1: Basic Editing, Smart Delimiters & Indentation](#module-1-basic-editing-smart-delimiters--indentation)
- [Module 2: Transactional Undo, Redo & Line Operations](#module-2-transactional-undo-redo--line-operations)
- [Module 3: Search, Replace, Replace-All & Whole-Word Matching](#module-3-search-replace-replace-all--whole-word-matching)
- [Module 4: Multi-Buffer Management & Buffer Switching](#module-4-multi-buffer-management--buffer-switching)
- [Module 5: Split-Screen Layouts (Vertical & Horizontal Panes)](#module-5-split-screen-layouts-vertical--horizontal-panes)
- [Module 6: Integrated Pseudo-Terminal Shell Panel](#module-6-integrated-pseudo-terminal-shell-panel)
- [Module 7: Workspace Symbol Indexer & Navigation](#module-7-workspace-symbol-indexer--navigation)
- [Module 8: Workspace, Quick File Switcher & File Explorer](#module-8-workspace-quick-file-switcher--file-explorer)
- [Module 9: Line Bookmarks Navigation](#module-9-line-bookmarks-navigation)
- [Module 10: Git Source Control Platform (Dashboard, Diff, Log, Branch, Stash)](#module-10-git-source-control-platform)
- [Module 11: Intelligent IDE Services (LSP Completion, Hover, Definition)](#module-11-intelligent-ide-services-lsp)
- [Module 12: DAP Interactive Debugger (Breakpoints, Stepping, Dashboard)](#module-12-dap-interactive-debugger)
- [Module 13: Project Scaffolding Wizard (C, Python, Rust, Go, Web)](#module-13-project-scaffolding-wizard)
- [Module 14: Remote VFS File Access (`ssh://`, `https://`)](#module-14-remote-vfs-file-access)
- [Module 15: Extension Ecosystem & Marketplace (.cpkg)](#module-15-extension-ecosystem--marketplace)
- [Module 16: Themes, Custom Palette Maker & Theme Manager](#module-16-themes-custom-palette-maker--theme-manager)
- [Module 17: Layout & Status Bar Settings](#module-17-layout--status-bar-settings)
- [Module 18: Project Build, Test, Output & Diagnostics Navigator](#module-18-project-build-test-output--diagnostics-navigator)
- [Module 19: Full Automated Test Suites (`make test`)](#module-19-full-automated-test-suites)

---

## Module 0: Compilation & Setup

### Direct Shell Commands
```bash
# Clean previous builds
make clean

# Compile release binary
make

# Or compile with AddressSanitizer & UndefinedBehaviorSanitizer
make asan
```

### Expected Output
- Compiles `claw` without any warnings or errors.
- Output binary `./claw` is created in current directory.

---

## Module 1: Basic Editing, Smart Delimiters & Indentation

### Direct Launch Command
```bash
./claw test_edit.c
```

### Keystroke Steps
1. **Type code with automatic delimiter pairing**:
   - Type: `int main() {`
   - *What it does*: Typing `(` automatically inserts `)` and positions the cursor in between. Typing `{` automatically inserts `}`.
2. **Press `Enter`**:
   - *What it does*: Automatically indents the next line by 4 spaces.
3. **Type statement**:
   - Type: `printf("Hello World\n");`
   - *What it does*: Double quotes `"` are paired automatically.
4. **Test Matching Delimiter Jump**:
   - Move cursor over `{` or `(` and press `Alt+]`.
   - *What it does*: Instantly jumps the cursor to the matching closing delimiter `}` or `)`.
5. **Save the File**:
   - Press `Ctrl+S`.
   - *What it does*: Writes `test_edit.c` to disk. Status bar shows `Saved test_edit.c`.
6. **Exit Claw**:
   - Press `Ctrl+X`.
   - *Cleanup*: `rm -f test_edit.c`.

---

## Module 2: Transactional Undo, Redo & Line Operations

### Direct Launch Command
```bash
./claw test_undo.txt
```

### Keystroke Steps
1. **Insert lines of text**:
   - Type: `Line One` -> Press `Enter`.
   - Type: `Line Two` -> Press `Enter`.
   - Type: `Line Three`.
2. **Duplicate Line**:
   - Press `Alt+D`.
   - *What it does*: Duplicates `Line Three` directly below.
3. **Move Line Up**:
   - Press `Shift+Up`.
   - *What it does*: Swaps current line upward with `Line Two`.
4. **Move Line Down**:
   - Press `Shift+Down`.
   - *What it does*: Swaps current line downward back to bottom.
5. **Toggle Comment**:
   - Press `Alt+/`.
   - *What it does*: Prepend `// ` to the line. Pressing `Alt+/` again uncomments the line.
6. **Join Line with Next**:
   - Move cursor to Line 1 and press `Alt+J`.
   - *What it does*: Joins Line 1 and Line 2 into a single line.
7. **Test Undo & Redo**:
   - Press `Ctrl+Z` multiple times.
   - *What it does*: Rolls back edits step-by-step through transaction history.
   - Press `Ctrl+Y` multiple times.
   - *What it does*: Re-applies undone changes.
8. **Exit**:
   - Press `Ctrl+X` (if prompted with unsaved changes, press `Ctrl+X` again to force quit).
   - *Cleanup*: `rm -f test_undo.txt`.

---

## Module 3: Search, Replace, Replace-All & Whole-Word Matching

### Direct Launch Command
```bash
# Create sample file with operator and word tests
cat << 'EOF' > test_search.c
int calculate(int x, int y) {
    int total = x + y;
    total = total + 10;
    return total;
}
EOF
./claw test_search.c
```

### Keystroke Steps
1. **Interactive Search**:
   - Press `Ctrl+F`. Status prompt shows `Search: `.
   - Type: `total`.
   - Press `Enter`.
   - *What it does*: Highlights all 4 occurrences in inverse color and places cursor on match 1. Status shows `4 matches found`.
2. **Cycle Search Matches**:
   - Press `Ctrl+N`. Moves to `Match 2/4`.
   - Press `Ctrl+N`. Moves to `Match 3/4`.
   - Press `Ctrl+P`. Moves back to `Match 2/4`.
   - Press `Esc`. Clears search highlights.
3. **Search for Operators/Symbols**:
   - Press `Ctrl+F`.
   - Type: `+`.
   - Press `Enter`.
   - *What it does*: Directly locates `+` operator in `x + y` and `total + 10`.
   - Press `Esc` to clear.
4. **Single-Match Replacement**:
   - First, ensure target search matches exist: Press `Ctrl+F` -> type `total` -> press `Enter` (highlights matches).
   - Press `Ctrl+R` to enter Replace mode. Status prompt shows `Replace: `.
   - Type replacement string: `sum`.
   - Press `Enter`.
   - *What it does*: Replaces the active match `total` with `sum`. Status bar confirms `Replaced "total" with "sum"`.
5. **Global Replace-All**:
   - Search for current instances: Press `Ctrl+F` -> type `sum` -> press `Enter`.
   - Press `Ctrl+R` to enter Replace mode (prompt shows `Replace: `).
   - Type new replacement: `accumulator`.
   - Press `Ctrl+A`.
   - *What it does*: Replaces all occurrences across the entire file in a single pass. Status bar displays `3 replacements made`.
6. **Exit**:
   - Press `Ctrl+X` twice.
   - *Cleanup*: `rm -f test_search.c`.

---

## Module 4: Multi-Buffer Management & Buffer Switching

### Direct Launch Command
```bash
./claw src/main.c include/editor.h Makefile
```

### Keystroke Steps
1. **Verify Initial Buffer**:
   - Observe status bar: Shows `[1/3] main.c`.
2. **Cycle to Next Buffer**:
   - Press `Ctrl+Tab`.
   - *What it does*: Switches active buffer to `include/editor.h`. Status shows `Buffer [2/3]: editor.h`.
3. **Cycle to Previous Buffer**:
   - Press `Ctrl+Shift+Tab`.
   - *What it does*: Switches back to `src/main.c`.
4. **Direct Buffer Jump by Number**:
   - Press `Ctrl+3`.
   - *What it does*: Jumps directly to buffer 3 (`Makefile`).
   - Press `Ctrl+1`.
   - *What it does*: Jumps directly back to buffer 1 (`src/main.c`).
5. **Close Current Buffer**:
   - Press `Ctrl+W`.
   - *What it does*: Closes `src/main.c`. Active buffer shifts to `editor.h` and status confirms `Buffer closed. Active [1/2]: editor.h`.
6. **Exit**:
   - Press `Ctrl+X`.

---

## Module 5: Split-Screen Layouts (Vertical & Horizontal Panes)

### Direct Launch Command
```bash
./claw src/editor/editor.c include/editor.h
```

### Keystroke Steps
1. **Open Vertical Split (Side-by-Side)**:
   - Press `Ctrl+\`.
   - *What it does*: Divides the terminal vertically with a center `│` divider. Left pane displays `editor.c`; right pane displays `editor.h`. Both panes have isolated gutter line numbers.
2. **Switch Focus Between Panes**:
   - Press `Alt+O`.
   - *What it does*: Moves keyboard focus to right pane (Pane 2). Status shows `Active Pane: Pane 2 (editor.h)`. Cursor moves inside Pane 2.
   - Press `Alt+O`.
   - *What it does*: Moves keyboard focus back to left pane (Pane 1).
3. **Scroll Left Pane Independently**:
   - Press `Arrow Down` multiple times.
   - *What it does*: Pane 1 scrolls vertically while Pane 2 stays stationary.
4. **Toggle to Horizontal Split (Top/Bottom)**:
   - Press `Ctrl+-`.
   - *What it does*: Transforms layout into horizontal split with an inverse-video title bar `--- Pane 1: editor.c (Active) --- Pane 2: editor.h ---`.
5. **Close Active Split Pane**:
   - Press `Ctrl+W`.
   - *What it does*: Closes the split layout. The editor smoothly collapses back to full-screen single-pane mode.
6. **Exit**:
   - Press `Ctrl+X`.

---

## Module 6: Integrated Pseudo-Terminal Shell Panel

### Direct Launch Command
```bash
./claw src/main.c
```

### Keystroke Steps
1. **Open Terminal Panel**:
   - Press ``Ctrl+` ``.
   - *What it does*: Allocates a bottom terminal panel displaying `claw:~$ ` prompt.
2. **Execute Builtin Commands**:
   - Type: `pwd` -> Press `Enter`.
   - *What it does*: Prints current working directory in scrollback.
   - Type: `help` -> Press `Enter`.
   - *What it does*: Displays Claw builtin commands overview.
3. **Execute Standard Shell Commands**:
   - Type: `ls -la include/` -> Press `Enter`.
   - *What it does*: Executes `ls` and prints output into the terminal panel.
4. **Test Command History Navigation**:
   - Press `Arrow Up`. Recalls `ls -la include/`.
   - Press `Arrow Up`. Recalls `help`.
   - Press `Arrow Down`. Returns toward latest input.
5. **Clear Scrollback**:
   - Press `Ctrl+L`.
   - *What it does*: Clears terminal panel output lines.
6. **Close Terminal Panel**:
   - Press `Esc` or ``Ctrl+` ``.
   - *What it does*: Hides terminal panel and restores full editor height.
7. **Exit**:
   - Press `Ctrl+X`.

---

## Module 7: Workspace Symbol Indexer & Navigation

### Direct Launch Command
```bash
./claw src/main.c
```

### Keystroke Steps
1. **Open Symbol Navigator**:
   - Press `Ctrl+Shift+S`.
   - *What it does*: Scans workspace source files and opens the Symbol Navigator overlay displaying `Query: _`.
2. **Filter by Symbol Name**:
   - Type: `editor`.
   - *What it does*: Live-filters indexed symbols to matches such as `editorInit`, `editorRun`, `editorShutdown`. Cursor stays directly inside the `Query: ` input field.
3. **Navigate Filtered Results**:
   - Press `Arrow Down` to select `editorShutdown`.
4. **Jump to Definition**:
   - Press `Enter`.
   - *What it does*: Closes Symbol Navigator, automatically opens `src/editor/editor.c`, and positions cursor at the line where `editorShutdown` is declared.
5. **Exit**:
   - Press `Ctrl+X`.

---

## Module 8: Workspace, Quick File Switcher & File Explorer

### Direct Launch Command
```bash
./claw
```

### Keystroke Steps
1. **Open Workspace**:
   - Press `Alt+W`.
   - *What it does*: Detects workspace root (e.g. `claw`) and loads configuration. Status shows `Workspace: claw`.
2. **Quick File Switcher (Fuzzy Prefix Filter)**:
   - Press `Alt+Q`.
   - *What it does*: Opens Quick File Switcher modal.
   - Type: `split`.
   - *What it does*: Live filters to `src/layout/split.c` and `include/split.h`.
   - Press `Enter`.
   - *What it does*: Immediately opens `split.c` into the editor.
3. **Workspace File Explorer**:
   - Press `Alt+E`.
   - *What it does*: Opens full-screen File Explorer showing directories `[DIR]` and files `[FILE]` with byte sizes. Cursor sits directly on `>` indicator.
   - Press `Arrow Down` to select `include` `[DIR]`.
   - Press `Enter`.
   - *What it does*: Enters `include/` directory.
   - Press `Backspace` or `Arrow Left`.
   - *What it does*: Returns to parent directory.
   - Press `Esc`.
   - *What it does*: Closes File Explorer and returns to editing.
4. **Exit**:
   - Press `Ctrl+X`.

---

## Module 9: Line Bookmarks Navigation

### Direct Launch Command
```bash
./claw src/main.c
```

### Keystroke Steps
1. **Set Bookmarks**:
   - Move cursor to Line 14 (`editorInit();`).
   - Press `Alt+B`.
   - *What it does*: Gutter displays a yellow `*` bookmark marker on Line 14.
   - Move cursor to Line 34 (`editorRun();`).
   - Press `Alt+B`.
   - *What it does*: Sets a second bookmark `*` on Line 34.
2. **Navigate Between Bookmarks**:
   - Press `Alt+Up`.
   - *What it does*: Jumps cursor directly to bookmark at Line 14.
   - Press `Alt+Down`.
   - *What it does*: Jumps cursor directly to bookmark at Line 34.
3. **Clear Bookmarks**:
   - Press `Alt+C`.
   - *What it does*: Clears all bookmarks across the document. Gutter markers disappear.
4. **Exit**:
   - Press `Ctrl+X`.

---

## Module 10: Git Source Control Platform

### Direct Launch Command
```bash
./claw src/main.c
```

### Keystroke Steps
1. **Open Git Dashboard**:
   - Press `Ctrl+Shift+G` or `Alt+5`.
   - *What it does*: Opens full-screen Git Dashboard showing current branch (`main`), upstream tracking, ahead/behind count, staged changes, unstaged changes, and untracked files.
2. **Inspect & Stage Files**:
   - Press `Arrow Down` to highlight an unstaged file.
   - Press `Space`.
   - *What it does*: Stages the highlighted file (moves to Staged section). Pressing `Space` again unstages it.
3. **Interactive Diff Viewer**:
   - Press `d` (or press `Ctrl+Shift+K` from editor).
   - *What it does*: Opens the Diff Viewer modal displaying color-coded additions (`+` green) and deletions (`-` red).
   - Press `Esc` to return to dashboard.
4. **Git Commit History Log**:
   - Press `l` (or press `Ctrl+Shift+L` from editor).
   - *What it does*: Opens Git Log viewer showing commit hashes, authors, relative dates, and commit titles. Press `Enter` on any commit to view full commit stats.
   - Press `Esc` to return.
5. **Branch Manager**:
   - Press `b` (or press `Ctrl+Shift+R` from editor).
   - *What it does*: Lists local git branches. Allows creating a new branch (`n`), switching (`Enter`), or deleting (`d`).
   - Press `Esc` to return.
6. **Stash Manager**:
   - Press `z` (or press `Ctrl+Shift+Z` from editor).
   - *What it does*: Displays git stashes. Allows pushing changes to stash (`p`), popping (`Enter`), or applying (`a`).
   - Press `Esc` to return.
7. **Close Dashboard**:
   - Press `Esc`.
8. **Test Git Blame View**:
   - Press `Ctrl+B`.
   - *What it does*: Loads git blame annotations line-by-line for `src/main.c`.
   - Press `Esc` or `q` to return safely to normal file editing.
9. **Exit**:
   - Press `Ctrl+X`.

---

## Module 11: Intelligent IDE Services (LSP)

### Direct Launch Command
```bash
./claw src/main.c
```

### Keystroke Steps
1. **Trigger Code Completion**:
   - Place cursor after `editor` and press `Ctrl+Space`.
   - *What it does*: Displays LSP completion popup list with symbols (`editorInit`, `editorRun`, `editorShutdown`).
   - Press `Arrow Down` / `Arrow Up` to choose an item.
   - Press `Enter` to insert completion.
2. **Go to Definition**:
   - Place cursor over `editorInit` and press `Ctrl+]`.
   - *What it does*: Resolves definition target and opens `src/editor/editor.c` at Line 118 where `void editorInit(void)` begins.
3. **Hover Documentation**:
   - Press `Alt+K`.
   - *What it does*: Opens hover popup window displaying type signatures and documentation.
   - Press `Esc` to dismiss.
4. **Find References**:
   - Press `Alt+Shift+F`.
   - *What it does*: Scans workspace and opens references overlay showing all files and line numbers referencing the symbol.
   - Press `Esc` to return.
5. **Code Actions & Refactoring**:
   - Press `Alt+Enter`.
   - *What it does*: Opens Code Actions menu suggesting quick-fixes and refactorings.
   - Press `Esc` to close.
6. **Exit**:
   - Press `Ctrl+X`.

---

## Module 12: DAP Interactive Debugger

### Direct Launch Command
```bash
./claw src/main.c
```

### Keystroke Steps
1. **Toggle Breakpoints**:
   - Move cursor to Line 14 (`editorInit();`).
   - Press `F9`.
   - *What it does*: Gutter displays red `B` breakpoint indicator on Line 14.
   - Move cursor to Line 34 and press `F9` to toggle a second breakpoint.
2. **Open Debugger Dashboard**:
   - Press `Alt+V`.
   - *What it does*: Opens DAP Debugger Dashboard showing:
     - Debug session state (`IDLE` / `STOPPED` / `RUNNING`)
     - Active Breakpoints list with filenames and lines
     - Call Stack frames
     - Variables inspection tree
3. **Debugger Session Controls**:
   - `F5`: Start debugging / Continue execution.
   - `F10`: Step Over next statement.
   - `F11`: Step Into function call.
   - `Shift+F11`: Step Out of current function.
   - `F6`: Pause running debug session.
   - `Shift+F5`: Stop / Terminate debug session.
4. **Exit**:
   - Press `Esc` to close dashboard, then `Ctrl+X`.

---

## Module 13: Project Scaffolding Wizard

### Direct Launch Command
```bash
./claw
```

### Keystroke Steps
1. **Open Project Scaffolding Wizard**:
   - Press `Alt+N`.
   - *What it does*: Opens full-screen interactive project template wizard.
2. **Select Project Language/Framework**:
   - Options displayed:
     - `1. C Application` (Makefile, include/, src/)
     - `2. Python Package` (pyproject.toml, src/, tests/)
     - `3. Rust Project` (Cargo.toml, src/main.rs)
     - `4. Go Module` (go.mod, main.go)
     - `5. Web Application` (index.html, app.js, style.css)
   - Press `Arrow Down` to highlight `1. C Application`.
3. **Generate Scaffolding**:
   - Press `Enter`.
   - *What it does*: Generates project files in workspace root and opens template entry point.
4. **Exit**:
   - Press `Esc` or `Ctrl+X`.

---

## Module 14: Remote VFS File Access

### Direct Launch Command
```bash
./claw
```

### Keystroke Steps
1. **Open Remote Dialog**:
   - Press `Ctrl+Shift+U`.
   - *What it does*: Opens Remote File URI modal prompt: `Remote URI: `.
2. **Enter Remote URI**:
   - Type: `ssh://user@server.local/home/user/project/file.c` or `https://raw.githubusercontent.com/.../README.md`.
   - Press `Enter`.
   - *What it does*: Resolves URI protocol, fetches file into local VFS cache, and opens it into a buffer.
3. **Exit**:
   - Press `Esc` to cancel or `Ctrl+X` to quit.

---

## Module 15: Extension Ecosystem & Marketplace

### Direct Launch Command
```bash
./claw
```

### Keystroke Steps
1. **Open Marketplace**:
   - Press `Ctrl+Shift+M`.
   - *What it does*: Opens Extension Marketplace catalogue.
2. **Browse Categories**:
   - Categories listed: `All`, `Themes`, `Languages`, `Tools`, `Linters`.
   - Press `Tab` or `Arrow Right` to switch categories.
3. **Search Extensions**:
   - Press `/` to enter search mode.
   - Type: `theme`.
   - *What it does*: Filters packages to matching theme bundles.
4. **Install / Manage Package**:
   - Highlight any package and press `Enter`.
   - *What it does*: Opens package details view. Press `i` to install/upgrade (verifies SHA-256 and digital signature) or `u` to uninstall.
5. **Close Marketplace**:
   - Press `Esc` to return to editor.
6. **Exit**:
   - Press `Ctrl+X`.

---

## Module 16: Themes, Custom Palette Maker & Theme Manager

### Direct Launch Command
```bash
./claw src/main.c
```

### Keystroke Steps
1. **Cycle Built-in Themes**:
   - Press `Alt+T`.
   - *What it does*: Switches to next theme (e.g. `Theme: Solarized Dark`). Status bar clearly confirms the theme name.
   - Press `Alt+T` again. Switches to `Dracula`, `Monokai`, `Nord`, etc.
2. **Open Theme Maker Palette Editor**:
   - Press `Alt+3`.
   - *What it does*: Opens interactive color palette creator. Use `Up`/`Down` to select syntax token color (Keyword, String, Comment, Type), `Left`/`Right` to adjust ANSI color codes in real-time, and `Ctrl+S` to save.
   - Press `Esc` to close.
3. **Open Theme Manager**:
   - Press `Alt+4`.
   - *What it does*: Displays custom theme manager. Allows importing custom themes (`i`), exporting (`e`), activating (`Enter`), or deleting (`d`).
   - Press `Esc` to close.
4. **Exit**:
   - Press `Ctrl+X`.

---

## Module 17: Layout & Status Bar Settings

### Direct Launch Command
```bash
./claw src/main.c
```

### Keystroke Steps
1. **Status Bar Settings**:
   - Press `Alt+1`.
   - *What it does*: Opens Status Bar configuration menu.
   - Use `Up`/`Down` to select items (`Show Encoding`, `Show Line Count`, `Show Git Branch`).
   - Press `Space` to toggle on/off.
   - Press `Enter` to save changes.
2. **Layout Settings**:
   - Press `Alt+2`.
   - *What it does*: Opens Layout configuration menu.
   - Options include: `Show Line Numbers`, `Show Status Bar`, `Show Welcome Screen`.
   - Press `Space` to toggle line numbers gutter.
   - Press `Enter` to apply.
3. **Exit**:
   - Press `Ctrl+X`.

---

## Module 18: Project Build, Test, Output & Diagnostics Navigator

### Direct Launch Command
```bash
./claw
```

### Keystroke Steps
1. **Run Project Build Task**:
   - Press `Ctrl+Shift+B`.
   - *What it does*: Executes workspace build command (`make` / `cargo build`) in the background.
2. **Run Project Test Task**:
   - Press `Ctrl+Shift+T`.
   - *What it does*: Executes workspace test suite command.
3. **View Build Output Console**:
   - Press `Ctrl+Shift+O`.
   - *What it does*: Opens full output log modal showing raw compiler/test runner stdout and stderr.
   - Press `Esc` to dismiss.
4. **Navigate Diagnostics (Compiler Errors)**:
   - Press `Ctrl+Shift+D`.
   - *What it does*: Opens compiler error diagnostics list parsed from build output.
   - Press `Ctrl+Down` / `Ctrl+Up` to cycle directly through compiler error locations in code.
   - Press `Enter` on any error to jump directly to file and line.
5. **Project Dashboard**:
   - Press `Ctrl+Shift+P`.
   - *What it does*: Opens project overview dashboard with workspace metadata and task triggers.
   - Press `Esc` to close.
6. **Exit**:
   - Press `Ctrl+X`.

---

## Module 19: Full Automated Test Suites

### Direct Shell Command
```bash
make test
```

### Expected Output
Executes all 14 test suites and reports 100% completion:
```
[1/5] Testing Buffer Manager (Multi-Buffer)... PASSED.
[2/5] Testing Split-Screen Layout (Panes)... PASSED.
[3/5] Testing Integrated Terminal... PASSED.
[4/5] Testing Symbol Indexer & Navigation... PASSED.
[5/5] Testing Keymap & Input Decoding... PASSED.
ALL PHASE 8 TESTS COMPLETED & PASSED!

ALL PHASE 9 TESTS COMPLETED & PASSED! (Git Platform)
ALL PHASE 10 TESTS COMPLETED & PASSED! (Syntax Highlighting & Themes)
ALL PHASE 11 TESTS COMPLETED & PASSED! (LSP Services)
ALL PHASE 12 TESTS COMPLETED & PASSED! (DAP, Remote, Templates)
ALL PHASE 13 TESTS COMPLETED & PASSED! (Marketplace & Security)
ALL BUG FIX TESTS PASSED! (Search Loops, Gutter, Smart Backspace, Comments)
```
