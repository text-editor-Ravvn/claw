#ifndef BRACKETS_H
#define BRACKETS_H

extern int bracketMatchX;
extern int bracketMatchY;

int isBracket(char c);
char getMatchingBracket(char c);

void updateBracketMatch(void);

#endif