CC = gcc
TARGET = claw
CFLAGS = -Wall -Wextra -std=c11 -g -Iinclude
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
      src/fileio/fileio.c

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
	$(CC) $(CFLAGS) tests/buffer_test.c src/buffer/buffer.c src/buffer/row.c src/buffer/cursor.c -o buffer_test
	./buffer_test
	$(CC) $(CFLAGS) tests/file_test.c src/fileio/fileio.c src/buffer/buffer.c src/buffer/row.c src/buffer/cursor.c -o file_test
	./file_test
	rm -f buffer_test file_test

clean:
	rm -f $(OBJ) $(TARGET) buffer_test file_test

rebuild: clean all
