CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Wpedantic
PROGRAM = claw-splash
SOURCES = src/main.c src/terminal.c src/renderer.c
HEADERS = include/terminal.h include/renderer.h include/claw_art_data.h

.PHONY: all clean run
all: $(PROGRAM)

$(PROGRAM): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -Iinclude -o $(PROGRAM) $(SOURCES)

run: $(PROGRAM)
	./$(PROGRAM)

clean:
	rm -f $(PROGRAM) $(PROGRAM).exe
