# ==========================================
# Claw Build System (Current Stage)
# ==========================================

CC = gcc

TARGET = claw

CFLAGS = -Wall -Wextra -std=c11 -g -Iinclude
SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer

# Current implemented modules only

SRC = src/main.c \
      src/editor/editor.c \
      src/input/input.c \
      src/input/rawmode.c \
      src/buffer/buffer.c \
      src/buffer/row.c \
      src/buffer/cursor.c \
      src/render/render.c \
      src/fileio/fileio.c

OBJ = $(SRC:.c=.o)

# ==========================================
# Build
# ==========================================

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# Run
# ==========================================

run: all
	./$(TARGET)

# ==========================================
# Debug
# ==========================================

debug: clean
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

asan: clean
      $(CC) $(CFLAGS) $(SANITIZER_FLAGS) $(SRC) -o $(TARGET)

test:
      $(CC) $(CFLAGS) tests/buffer_test.c src/buffer/buffer.c src/buffer/row.c src/buffer/cursor.c -o buffer_test
      ./buffer_test
      $(CC) $(CFLAGS) tests/file_test.c src/fileio/fileio.c src/buffer/buffer.c src/buffer/row.c src/buffer/cursor.c -o file_test
      ./file_test
      rm -f buffer_test
      rm -f file_test

# ==========================================
# Clean
# ==========================================

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)
      rm -f buffer_test file_test

# ==========================================
# Rebuild
# ==========================================

rebuild: clean all

.PHONY: all run clean rebuild debug asan test