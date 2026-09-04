#ifndef FILEIO_H
#define FILEIO_H

/* Return non-zero only when the complete operation succeeds. */
int openFile(const char *filename);
int saveFile(const char *filename);

#endif