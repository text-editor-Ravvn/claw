CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
TARGET = my_c_project
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
