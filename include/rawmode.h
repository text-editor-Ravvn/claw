#ifndef RAWMODE_H
#define RAWMODE_H

/* Switch stdin to character-at-a-time mode and restore it on shutdown. */
void enableRawMode(void);
void disableRawMode(void);

#endif