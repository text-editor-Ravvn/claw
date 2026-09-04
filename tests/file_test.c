#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "fileio.h"

int main(void)
{
	const char *filename = "file_test_input.tmp";
	const char source[] = "first\nsecond\0byte";
	FILE *file = fopen(filename, "wb");
	assert(file != NULL);
	assert(fwrite(source, 1, sizeof(source) - 1, file) == sizeof(source) - 1);
	assert(fclose(file) == 0);

	assert(openFile(filename) == 1);
	assert(buffer.numRows == 2);
	assert(buffer.rows[0].size == 5);
	assert(strcmp(buffer.rows[0].chars, "first") == 0);
	assert(buffer.rows[1].size == 11);
	assert(buffer.rows[1].chars[6] == '\0');
	assert(buffer.endsWithNewline == 0);
	assert(saveFile(filename) == 1);

	file = fopen(filename, "rb");
	assert(file != NULL);
	char saved[sizeof(source)] = {0};
	assert(fread(saved, 1, sizeof(saved) - 1, file) == sizeof(saved) - 1);
	assert(fclose(file) == 0);
	assert(memcmp(saved, source, sizeof(source) - 1) == 0);

	remove(filename);
	bufferFree();
	return 0;
}
