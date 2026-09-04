#ifndef STATUSBAR_H
#define STATUSBAR_H

/* Draw persistence state, cursor coordinates, and the latest message. */
void refreshStatusBar(const char *filename, int modified, int cursorX, int cursorY, const char *message);

#endif
