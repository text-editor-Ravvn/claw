# Claw Editor: Analysis of Errors, Bugs, and Malfunctions

## Overview
A comprehensive static analysis and operational review of the Claw C11 codebase was performed across all 13 architecture phases. The following sections categorize and document all discovered fatal bugs, crash risks, UI glitches, memory leaks, and compilation issues.

---

## 1. High-Severity Functional Bugs & Crash Risks

### 1.1 Search Engine Failing on Operators, Symbols & Non-Alphanumeric Tokens
- **File**: `src/search/search.c:21-36`
- **Component**: Word Boundary Engine (`isWholeWordMatch`)
- **Bug**:
  ```c
  int leftBoundary = match == line || !isWordCharacter((unsigned char)match[-1]);
  int rightBoundary = match[queryLength] == '\0' || !isWordCharacter((unsigned char)match[queryLength]);
  ```
  `isWholeWordMatch` unconditionally tested whether the preceding and following characters were non-word characters, without checking whether the query itself began or ended with a word character. When searching for any operator or symbol adjacent to identifiers (e.g. `->` in `a->b`, `+` in `1+2`, `.` in `foo.bar`, `==` in `x==y`, `[` in `arr[0]`), `match[-1]` is a word character, making `!isWordCharacter(match[-1])` false. The search engine reported "No matches found" and refused to find operators or code symbols anywhere in the file.
- **Fix**: Require left word boundary constraints only when `isWordCharacter(match[0])` is true, and right word boundary constraints only when `isWordCharacter(match[queryLength - 1])` is true.

### 1.2 Unchecked Heap Buffer Overread in Terminal Renderer
- **File**: `src/render/render.c:513`
- **Component**: Syntax Highlighting Rendering Loop (`renderPaneTextRow`)
- **Bug**:
  ```c
  else if (row->hl && fileCol < row->size)
  {
      int hlType = row->hl[fileCol];
  ```
  `row->hl` is allocated with `row->hlSize` elements. Whenever `row->size > row->hlSize` (such as after text replacement or before highlight recalculation), `fileCol < row->size` is true while `fileCol >= row->hlSize`. Accessing `row->hl[fileCol]` reads out-of-bounds heap memory, causing memory corruption or segmentation faults.
- **Fix**: Check `fileCol < row->size && fileCol < row->hlSize`.

### 1.3 Out-of-Bounds Heap Underflow and Crash in ReplaceCurrentMatch
- **File**: `src/search/search.c:257-295`
- **Component**: Single Match Replacement (`replaceCurrentMatch`)
- **Bug**:
  `replaceCurrentMatch` accessed `buffer.rows[row]` without checking if `row < buffer.numRows` or `r->chars != NULL`. Furthermore, if `col + queryLen > r->size` (e.g. if the document was mutated or matches were stale), the expression `(size_t)(r->size - col - queryLen)` underflows to ~18 quintillion (`SIZE_MAX - k`), causing `memcpy` to immediately crash with a segmentation fault / heap corruption.
- **Fix**: Added comprehensive guards verifying `searchState.currentMatch < searchState.matchCount`, `row >= 0 && row < buffer.numRows`, `r->chars != NULL`, `queryLen > 0`, and `col >= 0 && col + queryLen <= r->size`.

### 1.4 Infinite Loop & Process Freeze in Search & Replace All
- **File**: `src/search/search.c:416-460`
- **Component**: Global Search and Replace (`replaceAllMatches`)
- **Bug**:
  The function executed `while (1) { performSearch(); if (searchState.matchCount == 0) break; ... }` where each iteration re-ran a search across the entire document from the beginning. If the replacement string contained or formed another instance of the query (e.g., replacing `foo` with `foo bar`, or replacing a character `a` with `aa`), `performSearch()` repeatedly matched the newly inserted text, locking the editor into an infinite loop and permanently freezing the process.
- **Fix**: Performed a single-pass, row-by-row replacement advancing past replaced tokens.

### 1.5 Missing Syntax Highlight Invalidation Causing Heap Corruption
- **File**: `src/search/search.c:286-294`, `src/search/search.c:451-458`
- **Component**: Single and Global Match Replacement (`replaceCurrentMatch`, `replaceAllMatches`)
- **Bug**:
  When `r->chars` was replaced and `r->size` updated, `free(r->hl); r->hl = NULL; r->hlSize = 0;` was never called. The stale `hl` array with the old dimensions remained attached to the enlarged line, directly triggering the heap buffer overread in `render.c:513`.
- **Fix**: Invalidate `r->hl` and reset `r->hlSize` whenever rows are modified in search/replace.

### 1.6 Unmanaged / Scratch Buffer Wiped Out on Git View Exit
- **File**: `src/editor/editor.c:1971-2022`, `include/editor.h:20-30`
- **Component**: Git View Mode (`openGitView`, `closeGitView`)
- **Bug**:
  When `openGitView()` was called on an untitled/scratch buffer or before files were opened in `bufferManager` (`bufferManager.count == 0`), `openFile(tempFile)` loaded the blame view. When exiting (`closeGitView()`), `bufferFree()` freed the blame buffer, but `bufferManagerSyncToGlobal()` did nothing because `bufferManager.count == 0`. This left `buffer.rows = NULL` and `buffer.numRows = 0`, causing subsequent typing, cursor movement, or redraws to crash immediately.
- **Fix**: Store `savedBuffer`, `savedCursor`, `savedRowOffset`, and `savedColOffset` directly in `GitViewState`, and safely restore the original buffer upon closing the Git view.

### 1.7 Use-After-Free & Double-Free on Editor Shutdown After Adding Lines
- **File**: `src/editor/editor.c:1932-1945`
- **Component**: Editor Shutdown (`editorShutdown`)
- **Bug**:
  When newlines were added (`insertNewLine()`), `buffer.rows` was reallocated via `realloc()`. However, `bufferManager.buffers[activeIndex].doc.rows` retained the old (now freed) pointer. In `editorShutdown()`, `bufferManagerFreeAll()` was called without first synchronizing from `buffer`. `freeDocRows()` then traversed and freed the stale pointer, triggering a use-after-free and double-free under ASan.
- **Fix**: Call `bufferManagerSyncFromGlobal()` before `bufferManagerFreeAll()` in `editorShutdown()`.

### 1.8 Smart Backspace Corrupting Undo Stack & Deleting Wrong Characters
- **File**: `src/buffer/row.c:255-300`, `src/history/history.c:101-125`
- **Component**: Buffer Editing & Undo Engine
- **Bug**:
  When `historyUndo()` undid an insertion (or a newline), it called `deleteChar()` while history recording was disabled (`recordingEnabled == 0`). `deleteChar()` unconditionally ran `handleSmartBackspace()`. If the cursor was positioned at an indentation column, `handleSmartBackspace()` ignored the fact that an undo was in progress and deleted up to 4 spaces instead of the 1 inserted character being undone.
- **Fix**: Check `historyIsRecording()` before running smart backspace, and properly record deleted indentation spaces to undo history.

### 1.9 Negative Cursor Coordinates and Missing Row Bounds
- **File**: `src/buffer/row.c:209-253, 310-349, 408-474`, `src/buffer/cursor.c:18-232`
- **Component**: Buffer Editing & Cursor Navigation
- **Bug**:
  `insertChar()`, `deleteChar()`, and `deleteForward()` checked `cursor.y >= buffer.numRows` but omitted checks for negative `cursor.x < 0` or `cursor.x > row->size`. In `deleteForward()`, negative `cursor.x` read `row->chars[-1]`. In cursor movement (`moveCursorRight`, `moveCursorUp`, `moveCursorDown`, `moveCursorWordLeft`, `moveCursorWordRight`), missing `cursor.y < buffer.numRows` and negative row checks permitted indexing before the allocation.
- **Fix**: Added explicit clamping `[0, row->size]` and `[0, buffer.numRows - 1]` across row editing and cursor movement functions.

### 1.10 Null Pointer Dereference in File I/O
- **File**: `src/fileio/fileio.c:55-68, 122-135`
- **Component**: File Loading & Saving (`openFile`, `saveFile`)
- **Bug**:
  Calling `openFile(NULL)` or `saveFile(NULL)` passed `NULL` directly to `fopen(filename, "rb")` or `snprintf()`, causing immediate segmentation faults.
- **Fix**: Added `if (!filename) return 0;` guards at the entry of both functions.

### 1.11 Null Pointer Dereference in Search Row Scanning
- **File**: `src/search/search.c:101-115, 352-362`
- **Component**: Search Engine Scanning (`performSearch`, `refreshSearchMatches`)
- **Bug**:
  If any row in the document had `chars == NULL`, `strstr(start, query)` was called with a `NULL` haystack, causing undefined behavior / segmentation faults.
- **Fix**: Guarded with `if (!buffer.rows || !buffer.rows[row].chars) continue;`.

### 1.12 Stale Search Matches After Line Manipulation
- **File**: `src/buffer/row.c:609-717, 851-909`
- **Component**: Buffer Productivity Actions (`duplicateLine`, `moveLineUp`, `moveLineDown`, `joinLineWithNext`)
- **Bug**:
  When lines were swapped, duplicated, or joined, row indices changed and `numRows` decreased, but `refreshSearchMatches()` was not called. The cached match coordinates pointed to incorrect lines or out-of-bounds rows.
- **Fix**: Added `refreshSearchMatches()` calls to `duplicateLine()`, `moveLineUp()`, `moveLineDown()`, and `joinLineWithNext()`.

### 1.13 Desynchronized Active Buffer on Session Restore
- **File**: `src/workspace/session.c:161-185`
- **Component**: Workspace Session Restoration (`sessionRestore`)
- **Bug**:
  When `sessionRestore()` restored an active file into `buffer`, `bufferManager` was not notified, leaving its active slot as `[Untitled]`.
- **Fix**: Synchronize `bufferManager` via `bufferManagerSyncFromGlobal()` after restoring the session document.

### 1.14 Silent Truncation of Long Lines in Search & Replace
- **File**: `src/search/search.c:269`, `src/search/search.c:434`
- **Component**: String Replacement Buffers
- **Bug**:
  `char newLine[2048]` was allocated on the stack. If any document row was longer than 2048 bytes or became longer than 2048 bytes after replacement, `snprintf` truncated the line, silently discarding user source code.
- **Fix**: Use dynamically allocated memory (`malloc`) proportional to row size and replacement length.

### 1.15 Missing Directory Creation Causing Build / Task Failure
- **File**: `src/workspace/project.c:257-275`, `src/plugin/plugin_commands.c:15-325`
- **Component**: Project Command Execution & Plugin Logging
- **Bug**:
  `runProjectCommand` and `pluginLog` redirected output to `.claw/project-output.log` and `logs/plugins.log`. If `.claw` or `logs` did not exist, file opening failed silently or caused build tasks to fail.
- **Fix**: Ensure directories exist via `mkdir()` before file operations.

---

## 2. Terminal UI Glitches & Layout Issues

### 2.1 Status Bar Terminal Auto-Wrap and Screen-Scrolling Glitch
- **File**: `src/ui/statusbar.c:60-85, 185-210`
- **Component**: Status Bar Rendering (`drawStatusBar`)
- **Bug**:
  When total status characters exceeded `viewport.screenCols`, or when terminal width was very small (e.g. `screenCols < 15`), printing `right` (`"Ln 1, Col 1"`) exceeded the screen width. The terminal emulator auto-wrapped the line, pushing the entire editor screen up by one row on every keystroke.
- **Fix**: Clamped `leftLen` and clamped `right` to `viewport.screenCols - leftLen - padCount`, ensuring total output width strictly never exceeds `viewport.screenCols`.

### 2.2 Horizontal Scrolling Jitter / Oscillation in Narrow Panes
- **File**: `src/viewport/viewport.c:74-86`
- **Component**: Editor Viewport Scrolling (`scrollEditor`)
- **Bug**:
  `scrollEditor` used a fixed `HSCROLL_MARGIN = 10`. When `availCols <= 20` (e.g. in vertical split panes or small terminal windows), the left-scroll condition (`cursor.x < colOffset + 10`) and right-scroll condition (`cursor.x >= colOffset + availCols - 10`) overlapped and simultaneously triggered on the same cursor position, causing rapid scroll oscillation.
- **Fix**: Scaled `hMargin` so it never exceeds `availCols / 3`.

### 2.3 Prompt Cursor Detachment in Search, Save, Goto, File Switcher & Symbols
- **File**: `src/render/render.c:169, 1065-1099, 1605`
- **Component**: Cursor Positioning on Screen Refresh (`refreshScreen`)
- **Bug**:
  During `savePrompt`, `gotoPrompt`, `searchState`, `fileSwitcherState`, and `symbolState`, the terminal cursor remained positioned at code coordinates or at the bottom row rather than inside the active user input prompt.
- **Fix**: Positioned terminal cursor at `(viewport.screenRows, promptLen + 1)` for bottom prompts, and at `(3, filterLen + 1)` for modal switcher and symbol navigator input fields.

### 2.4 Backspace (ASCII 8) Ignored in Theme Maker & File Explorer
- **File**: `src/editor/editor.c:1070, 1484`
- **Component**: Modal Input Handlers
- **Bug**:
  Theme Maker name input and File Explorer checked only `key == 127`, ignoring terminals that send ASCII 8 (BS) for the backspace key.
- **Fix**: Added `key == 8` checks alongside `127`.

---

## 3. Robustness & Compiler Warnings

### 3.1 Shadowed Variable Declaration Warning
- **File**: `src/theme/theme.c:1093`, `src/theme/theme.c:1180`
- **Component**: Custom Theme Import/Export (`themeExportCustom`, `themeImportCustom`)
- **Bug**: Local variable `char buffer[4096]` shadowed global declaration `extern Buffer buffer;`.
- **Fix**: Renamed local variable to `ioBuf`.

### 3.2 Missing Prototypes and Header Includes
- **Files**:
  - `src/fileio/fileio.c`: Added `#include "fileio.h"`
  - `src/input/rawmode.c`: Added `#include "rawmode.h"`
  - `include/keymap.h`: Added prototype for `keymapParseFile`
  - `src/render/render.c`: Marked `drawSymbolNavigator` and `drawTerminalPanel` as `static`.
- **Fix**: Added missing headers, prototypes, and static specifiers.

### 3.3 Unbounded String Copy in Plugin Loader
- **File**: `src/plugin/plugin.c:52`
- **Component**: Plugin File Loading (`pluginLoad`)
- **Bug**: `strcpy(p.filename, path)` without checking buffer size.
- **Fix**: Replaced with `snprintf(p.filename, sizeof(p.filename), "%s", path)`.

---

## 4. Split-Screen Layout & Multi-Pane Buffer Isolation

### 4.1 Pane 0 Cursor Coordinates & Viewport Zeroed Out on Split Creation
- **File**: `src/layout/split.c:92-181, 183-271`
- **Component**: Split Initialization (`splitHorizontal`, `splitVertical`)
- **Bug**: When splitting the screen with only 1 active buffer, `splitVertical()` or `splitHorizontal()` enabled split mode *before* calling `bufferManagerNew()`. Inside `bufferManagerNew()`, `if (splitIsActive())` executed and wiped `splitLayout.panes[0]`'s cursor and scroll offsets to 0. When switching panes via `Alt+O`, the user's cursor position and line offset in Pane 0 were lost.
- **Fix**: Preserved pre-split cursor and viewport coordinates, temporarily suspended split mode during new buffer creation, and restored Pane 0 coordinates and global buffer.

### 4.2 Split Pane Cursor Jump on Split Closure
- **File**: `src/layout/split.c:273-314`
- **Component**: Split Closure (`splitClose`)
- **Bug**: Closing a split pane with `Ctrl+W` read `splitLayout.panes[remainingPane]` coordinates without first syncing the current active editor `cursor` and `viewport` into that pane, overwriting the user's recent edits and scrolling with stale coordinates.
- **Fix**: Synced active cursor and viewport coordinates into the remaining pane prior to collapsing the split layout.

### 4.3 Tab Character Expansion Corrupting Vertical Divider Alignment
- **File**: `src/render/render.c:466-560`
- **Component**: Pane Text Row Rendering (`renderPaneTextRow`)
- **Bug**: When files in Pane 0 contained tab characters (`\t`), `putchar('\t')` expanded up to 8 visual terminal columns, but character counts were computed as byte lengths. The subsequent space padding overshot `textCols0`, pushing the `│` vertical divider rightward and causing Pane 1 text to wrap onto subsequent terminal lines.
- **Fix**: Implemented strict 4-column visual tab stop calculation, visual column tracking (`printedCols`), and hard column clamping up to `textCols`.

### 4.4 Ghost Dividers & Remnants Due to Missing Screen Invalidation
- **File**: `src/layout/split.c:100, 179, 192, 269, 312`
- **Component**: Split Mode Transitions
- **Bug**: Toggling between horizontal and vertical splits or closing a split did not trigger `renderForceClear()`. If the terminal window dimensions remained unchanged, `render.c` drew incrementally without clearing stale `│` vertical dividers or horizontal separator lines.
- **Fix**: Added explicit `renderForceClear()` calls on all split transitions, openings, and closures.

### 4.5 Multi-Buffer Closure Collapsing Both Panes to Identical Buffer
- **File**: `src/buffer/buffer_manager.c:380-415`
- **Component**: Buffer Manager (`bufferManagerClose`)
- **Bug**: When multiple files were open and a buffer displayed in Pane 1 was closed, `bufferManagerClose()` indiscriminately assigned `bufferManager.activeIndex` (Pane 0's buffer) to Pane 1, forcing both panes onto the exact same buffer. Additionally, closing Buffer 0 when 3+ buffers were open caused sequential index shifts to collide, collapsing both panes to Buffer 0.
- **Fix**: Implemented a two-pass reassignment algorithm that shifts buffer indices down and assigns closing panes to available unviewed buffers (`b != otherBuf`).

### 4.6 Buffer Cycling (`Ctrl+Tab` / `Ctrl+Shift+Tab`) Colliding With Other Pane
- **File**: `src/buffer/buffer_manager.c:220-238`
- **Component**: Buffer Switching (`bufferManagerNext`, `bufferManagerPrev`)
- **Bug**: When cycling buffers via `Ctrl+Tab` or `Ctrl+Shift+Tab` in a split view with 3+ buffers open, advancing by $\pm 1$ picked the buffer index currently displayed in the inactive pane, causing both panes to display identical content.
- **Fix**: Added split mode guards in `bufferManagerNext()` and `bufferManagerPrev()` to skip the buffer index currently active in the other pane.

### 4.7 Carriage Return Character Bleed in Split Rendering
- **File**: `src/render/render.c:474`
- **Component**: Split Row Text Rendering (`renderPaneTextRow`)
- **Bug**: Files with CRLF line endings contained `\r` characters that reset terminal column positioning mid-line, breaking pane boundaries.
- **Fix**: Added explicit `if (ch == '\r') continue;` guard.

---

## 5. IDE Platform, UI Navigation & Lifecycle Synchronization Fixes

### 5.1 Symbol Navigator Cursor Misplacement on Separator Row with Incorrect Filter Label Offset
- **File**: `src/render/render.c:172`
- **Component**: Symbol Search Modal (`drawSymbolNavigator`)
- **Bug**:
  ```c
  printf("\033[3;%dH", (int)strlen(" Filter: ") + (int)strlen(symbolState.query) + 1);
  ```
  The Symbol Navigator renders its title on line 1, its query input prompt (`Query: %s_`) on line 2, and a dashed separator on line 3. Line 172 positioned the terminal cursor on line 3 (`\033[3;...`) and used the prefix `" Filter: "` (length 9) from the File Switcher instead of `"Query: "` (length 7). The cursor was visually detached and hovering in the middle of dashes on row 3 rather than following the text on row 2.
- **Fix**: Changed escape code to `\033[2;%dH` using `(int)strlen("Query: ") + (int)strlen(symbolState.query) + 1`.

### 5.2 Horizontal Split Title Multi-Byte UTF-8 Truncation & Asymmetric Pad Underflow
- **File**: `src/render/render.c:936-944`
- **Component**: Horizontal Split Divider Bar (`refreshScreen`)
- **Bug**:
  The divider title used multi-byte box-drawing characters (`───`), where each dash is 3 bytes in UTF-8 (`\xe2\x94\x80`) but occupies only 1 visual column. The string was printed with `%.*s` bounded by `viewport.screenCols`, which on narrower screens truncated the multibyte sequence mid-byte, causing invalid UTF-8 replacement glyphs. Furthermore, the trailing padding loop checked `for (int p = titleLen; p < viewport.screenCols; p++)` where `titleLen` was byte length (e.g. 60) rather than visual columns (36), leaving up to 24 columns unrendered and breaking inverse video continuity.
- **Fix**: Switched title decoration to clean ASCII borders (`---`) so byte length equals column count, clamped `titleLen` to `viewport.screenCols`, and padded remaining columns cleanly.

### 5.3 File Explorer Terminal Cursor Detachment on Help Legend
- **File**: `src/render/render.c:1583`
- **Component**: Workspace File Explorer Modal (`drawFileExplorer`)
- **Bug**: When `drawFileExplorer()` was active, `refreshScreen()` enabled the cursor with `\033[?25h`, but `drawFileExplorer()` printed through to the footer help line without repositioning the cursor, leaving the hardware cursor stranded at the bottom edge of the screen instead of tracking the highlighted directory entry.
- **Fix**: Added explicit cursor positioning to row `5 + (explorerState.selectedIndex - start)` at column 2, directly overlying the `>` indicator.

### 5.4 Git View / Blame Mode Document Memory Leak and Slot Overwrite on Editor Shutdown
- **File**: `src/editor/editor.c:1936`
- **Component**: Editor Shutdown Lifecycle (`editorShutdown`)
- **Bug**: If Claw was shut down or exited while `gitView.active` was 1 (such as while viewing git blame or the plugin log), `closeGitView()` was never executed. `gitView.savedBuffer` holding the user's original buffer rows was abandoned on the heap, and `bufferManagerSyncFromGlobal()` synced the temporary blame rows into the active buffer slot before shutdown, corrupting buffer state and leaking the original document.
- **Fix**: Added `if (gitView.active) closeGitView();` at the start of `editorShutdown()` to ensure original buffers and temp files are cleaned up prior to global sync and free.

### 5.5 Stale Search Match Coordinates Following Line Comment Toggle
- **File**: `src/buffer/row.c:860`
- **Component**: Buffer Line Commenting (`toggleCommentLine`)
- **Bug**: When commenting or uncommenting a line with `Alt+/`, prefix characters (e.g. `// `) were inserted or removed, shifting character columns across the row. Unlike `insertChar()`, `deleteChar()`, and `moveLineDown()`, `toggleCommentLine()` did not invoke `refreshSearchMatches()`. Any active search highlights on that row pointed to out-of-date column coordinates until manual re-search.
- **Fix**: Added `refreshSearchMatches();` call to `toggleCommentLine()`.

### 5.6 Silent Status Bar on Theme Next Cycling
- **File**: `src/command/command.c:590`
- **Component**: Theme Command Dispatch (`CMD_THEME_NEXT`)
- **Bug**: Pressing `Alt+T` advanced the active theme in memory, but called `editorSetStatusMessage("");`, wiping the status bar and leaving the user with zero visual indication of which theme was selected.
- **Fix**: Updated status message to format `"Theme: %s"` via `themeCurrentName()`.

### 5.7 Missing Status Confirmation on Single-Buffer Closure
- **File**: `src/command/command.c:618`
- **Component**: Buffer Command Dispatch (`CMD_BUFFER_CLOSE`)
- **Bug**: When closing a buffer with `Ctrl+W` in single-pane mode, `bufferManagerCloseActive()` closed the buffer without issuing a status notification.
- **Fix**: Added confirmation status message displaying remaining buffer count and current active buffer name.

---

## 6. Rendering Boundaries, Viewport Scrolling & Platform Robustness Fixes

### 6.1 Unallocated Buffer Null Pointer Dereference in Welcome Splash
- **File**: `src/render/render.c:46`
- **Component**: Welcome Splash Screen Detection (`welcomeSplashIsActive`)
- **Bug**:
  ```c
  int welcomeSplashIsActive(void) {
      return ... && buffer.numRows == 1 && buffer.rows[0].size == 0;
  }
  ```
  If `buffer.numRows == 1` while `buffer.rows == NULL` (e.g., prior to buffer row allocation or during temporary buffer swaps), evaluating `buffer.rows[0].size` dereferenced `NULL`, causing an immediate crash with `SIGSEGV`.
- **Fix**: Added `buffer.rows &&` validation prior to accessing index 0.

### 6.2 Heap Memory Underflow in Pane Text Row Renderer on Negative Indices
- **File**: `src/render/render.c:452`
- **Component**: Text Row Rendering Engine (`renderPaneTextRow`)
- **Bug**:
  `renderPaneTextRow()` checked `if (!doc || fileRow >= doc->numRows)` but omitted a check for `fileRow < 0`. If a negative row index was supplied (due to scrolling underflow or uninitialized pane offsets), the check passed and `Row *row = &doc->rows[fileRow]` indexed heap memory before the allocation, triggering heap memory corruption and segmentation faults under AddressSanitizer.
- **Fix**: Changed condition to `if (!doc || fileRow < 0 || fileRow >= doc->numRows)`.

### 6.3 Negative File Row Evaluation in Line Number Gutter & Subsystem Queries
- **File**: `src/render/render.c:870, 901, 980`
- **Component**: ANSI Line Number & Indicator Gutter Rendering (`refreshScreen`)
- **Bug**:
  In horizontal/vertical split and single-pane gutter loops, the row validity check was only `if (fileRow < curDoc->numRows)`. For negative `fileRow`, this condition evaluated to true, resulting in negative line arguments being passed to `bookmarkIsBookmarked(fileRow)`, `gitGutterGetLineStatus(fileRow + 1)`, and `dapGutterStatus(currentFile, fileRow + 1)`.
- **Fix**: Guarded all gutter rendering loops with `fileRow >= 0 && fileRow < curDoc->numRows`.

### 6.4 Negative Vertical Scrolling Offset Underflow in Viewport Engine
- **File**: `src/viewport/viewport.c:66-73`
- **Component**: Scrolling Offset Engine (`scrollEditor`)
- **Bug**:
  When `cursor.y < viewport.rowOffset`, the calculation `viewport.rowOffset = cursor.y` permitted `viewport.rowOffset` to become negative if `cursor.y < 0`. While horizontal scrolling clamped `viewport.colOffset` to 0, vertical scrolling lacked a lower bound check, causing negative row offsets to propagate into rendering routines.
- **Fix**: Added explicit clamping `if (viewport.rowOffset < 0) viewport.rowOffset = 0;`.

### 6.5 Terminal Hardware Cursor Gutter Misalignment in Vertical Split Pane 1
- **File**: `src/render/render.c:1170-1175`
- **Component**: ANSI Cursor Positioning (`refreshScreen`)
- **Bug**:
  In vertical split mode with line numbers enabled in Pane 1 (`g1 > 0`), the minimum column boundary was clamped to `splitLayout.panes[0].width + 2`. Because Pane 1's gutter digits occupy columns up to `splitLayout.panes[0].width + 1 + g1`, negative horizontal scrolling placed the hardware cursor on top of the line number digits rather than in the text area.
- **Fix**: Clamped Pane 1 cursor column to `minCol1 = splitLayout.panes[0].width + 1 + g1 + 1`.

### 6.6 Uninitialized Syntax Highlighting and Stale Status on New File Creation
- **File**: `src/buffer/buffer_manager.c:387-400`
- **Component**: Buffer Manager Opening Lifecycle (`bufferManagerOpen`)
- **Bug**:
  When `bufferManagerOpen()` opened a new file that did not yet exist on disk, `openFile(filename)` returned 0 and reached the fallback return. The fallback returned `targetIdx` without synchronizing the global buffer, without resetting viewport offsets or cursor coordinates, and without invoking `highlightInit()` or `gitRefreshStatus()`. The newly created buffer lacked syntax highlighting for its file extension and displayed stale Git status.
- **Fix**: Updated the fallback path to reset cursor/viewport coordinates, synchronize global buffer state, update split pane assignments, initialize language-specific syntax highlighting, and refresh Git status.

### 6.7 Pane Cursor Coordinate Overwrite in Split Switching
- **File**: `src/layout/split.c:338-344`
- **Component**: Split Pane Focus Switching (`splitSwitchPane`)
- **Bug**:
  When switching active split panes (`splitSwitchPane`), `bufferManagerSwitchTo(in->bufferIndex)` was invoked, which synchronized `fb->cursor` into `splitLayout.panes[splitLayout.activePane].cursorX`. This overwrote the incoming pane's isolated cursor coordinates before `cursor.x = in->cursorX` was applied.
- **Fix**: Preserved `in->cursorX`, `in->cursorY`, `in->rowOffset`, and `in->colOffset` in local variables across the buffer switch call and restored them explicitly to both the global cursor and pane descriptors.

### 6.8 Unbounded Row Access in Symbol Navigation on Empty Documents
- **File**: `src/workspace/symbols.c:504-512`
- **Component**: Symbol Navigator Jump (`symbolsJumpToSelected`)
- **Bug**:
  If a symbol jump was triggered into a buffer with `buffer.numRows == 0`, `cursor.y = targetLine - 1` set `cursor.y` to a positive value in an empty document, leading to out-of-bounds row dereferences in subsequent edits.
- **Fix**: Assigned `cursor.y = 0` whenever `buffer.numRows == 0`.



