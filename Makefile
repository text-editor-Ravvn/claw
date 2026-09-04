# ==========================================
# Claw Build System (Current Stage)
# ==========================================

CC = gcc

TARGET = claw

CFLAGS = -Wall -Wextra -std=c11 -g -Iinclude

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

# ==========================================
# Clean
# ==========================================

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

# ==========================================
# Rebuild
# ==========================================

rebuild: clean all

.PHONY: all run clean rebuild debug