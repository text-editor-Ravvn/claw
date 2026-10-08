CC = gcc
TARGET = claw
CFLAGS = -Wall -Wextra -std=c11 -g -Iinclude -D_POSIX_C_SOURCE=200809L
SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer

SRC = src/main.c \
      src/editor/editor.c \
      src/input/input.c \
      src/input/rawmode.c \
      src/buffer/buffer.c \
      src/buffer/row.c \
      src/buffer/cursor.c \
      src/render/render.c \
      src/ui/statusbar.c \
      src/fileio/fileio.c \
	  src/viewport/viewport.c \
	  src/search/search.c \
	  src/history/history.c \
	  src/config/config.c \
	  src/utils/string_utils.c \
	  src/keymap/keymap.c \
	  src/keymap/parser.c \
	  src/command/command.c \
	  src/highlight/highlight.c \
	  src/git/git.c \
	  src/plugin/plugin.c \
	  src/plugin/plugin_commands.c \
	  src/plugin/plugin_api.c \
	  src/brackets.c \
	  src/indent.c \
	  src/workspace/workspace.c \
	  src/workspace/recent.c \
	  src/workspace/session.c \
	  src/workspace/bookmark.c \
	  src/workspace/explorer.c \
	  src/workspace/file_switcher.c \
	  src/theme/theme.c \

OBJ = $(SRC:.c=.o)

.PHONY: all run debug asan test clean rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

debug: clean
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

asan: clean
	$(CC) $(CFLAGS) $(SANITIZER_FLAGS) $(SRC) -o $(TARGET)

test:
	$(CC) $(CFLAGS) tests/buffer_test.c src/buffer/buffer.c src/buffer/row.c src/buffer/cursor.c src/history/history.c src/config/config.c src/utils/string_utils.c -o buffer_test
	./buffer_test
	$(CC) $(CFLAGS) tests/file_test.c src/fileio/fileio.c src/buffer/buffer.c src/buffer/row.c src/buffer/cursor.c src/history/history.c src/plugin/plugin_api.c src/config/config.c src/utils/string_utils.c -o file_test
	./file_test
	$(CC) $(CFLAGS) tests/input_test.c src/input/input.c src/keymap/keymap.c src/keymap/parser.c src/utils/string_utils.c -o input_test
	./input_test
	$(CC) $(CFLAGS) tests/workspace_test.c src/workspace/workspace.c src/config/config.c src/utils/string_utils.c -o workspace_test
	./workspace_test
	$(CC) $(CFLAGS) tests/session_test.c src/workspace/session.c src/workspace/workspace.c src/workspace/recent.c src/utils/string_utils.c src/config/config.c -o session_test
	./session_test
	$(CC) $(CFLAGS) tests/bookmark_test.c src/workspace/bookmark.c -o bookmark_test
	./bookmark_test
	$(CC) $(CFLAGS) tests/phase5_integration_test.c src/workspace/workspace.c src/workspace/recent.c src/workspace/session.c src/workspace/bookmark.c src/workspace/file_switcher.c src/workspace/explorer.c src/config/config.c src/utils/string_utils.c -o phase5_test
	./phase5_test
	rm -f buffer_test file_test input_test workspace_test session_test bookmark_test phase5_test

clean:
	rm -f $(OBJ) $(TARGET) buffer_test file_test input_test workspace_test session_test bookmark_test phase5_test

rebuild: clean all
