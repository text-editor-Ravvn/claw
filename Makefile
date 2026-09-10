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
	$(CC) $(CFLAGS) tests/buffer_test.c src/buffer/buffer.c src/buffer/row.c src/buffer/cursor.c src/history/history.c -o buffer_test
	./buffer_test
	$(CC) $(CFLAGS) tests/file_test.c src/fileio/fileio.c src/buffer/buffer.c src/buffer/row.c src/buffer/cursor.c src/history/history.c -o file_test
	./file_test
	rm -f buffer_test file_test

clean:
	rm -f $(OBJ) $(TARGET) buffer_test file_test

rebuild: clean all
