#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "cursor.h"

static void resetBuffer(void)
{
	bufferFree();
	bufferInit();
	cursor.x = 0;
	cursor.y = 0;
}

int main(void)
{
	resetBuffer();
	insertChar('a');
	insertChar('b');
	assert(buffer.numRows == 1);
	assert(buffer.rows[0].size == 2);
	assert(strcmp(buffer.rows[0].chars, "ab") == 0);

	insertNewLine();
	assert(buffer.numRows == 2);
	assert(strcmp(buffer.rows[0].chars, "ab") == 0);
	assert(buffer.rows[1].size == 0);

	insertChar('c');
	deleteChar();
	assert(buffer.numRows == 2);
	assert(buffer.rows[1].size == 0);

	cursor.y = 0;
	cursor.x = 1;
	deleteForward();
	assert(strcmp(buffer.rows[0].chars, "a") == 0);

	cursor.x = 1;
	deleteForward();
	assert(buffer.numRows == 1);
	assert(strcmp(buffer.rows[0].chars, "a") == 0);

	bufferFree();
	return 0;
}
