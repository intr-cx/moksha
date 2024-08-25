#ifndef INCLUDE_INC_TUI_H_
#define INCLUDE_INC_TUI_H_

#include "irc.h"
#include <ncurses.h>

void tuiInit(void);
void tuiLoop(void);
void tuiDeinit(void);

void tuiDrawList(void);
void tuiDrawChannel(IrcServer *, IrcChannel *);

#endif // INCLUDE_INC_TUI_H_
