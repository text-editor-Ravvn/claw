CC=gcc

CFLAGS=-Wall -Wextra -std=c11 -Iinclude

SRC=src/main.c \
    src/editor.c \
    src/input.c \
    src/render.c

TARGET=claw

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)