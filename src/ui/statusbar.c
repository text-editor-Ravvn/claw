#include <stdio.h>

#include "statusbar.h"

void refreshStatusBar(const char *filename, int modified, int cursorX, int cursorY, const char *message)
{
	/* Reverse video separates editor metadata from document text. */
	const char *displayName = filename ? filename : "[No Name]";

	printf("\033[7m %-30s %s  Ln %d, Col %d ",
		displayName,
		modified ? "[Modified]" : "[Saved]",
		cursorY + 1,
		cursorX + 1);
	printf("\033[K\033[0m\r\n");

	if (message && message[0] != '\0')
		printf("%s\033[K\r\n", message);
}
