#include "tui.h"
#include "irc.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WID 16

char input[512];
uint cursor = 0;
WINDOW *w;

void tuiInit(void) {
  w = initscr();
  noecho();
  cbreak();
  keypad(w, true);
}

void tuiLoop(void) {
  int c;
  uint x, y;
  x = y = 0;
  getmaxyx(w, y, x);
  while ((c = wgetch(w)) != 0) {
    pthread_mutex_lock(&mutex);
    move(0, 0);
    char wht[8];
    snprintf(wht, 8, "%d", c);
    addstr(wht);
    switch (c) {
    case KEY_PPAGE: {
      if (NULL != servers) {
        if (servers[curServer].curChannel > 0)
          servers[curServer].curChannel--;
        else if (curServer > 0)
          curServer--;
        tuiDrawChannel(
            &servers[curServer],
            &servers[curServer].channels[servers[curServer].curChannel]);
      }
      break;
    }
    case KEY_NPAGE: {
      if (NULL != servers) {
        if (servers[curServer].curChannel < servers[curServer].lenChannels - 1)
          servers[curServer].curChannel++;
        else if (curServer < lenServers - 1)
          curServer++;
        tuiDrawChannel(
            &servers[curServer],
            &servers[curServer].channels[servers[curServer].curChannel]);
      }
      break;
    }
    case 10: {
      if (cursor != 0) {
        IrcServer *s = NULL == servers ? NULL : &servers[curServer];
        IrcChannel *ch = NULL == servers || s->lenChannels == 0 ? NULL : &s->channels[s->curChannel];
        ircChannelSendMsg(s, ch, input);
        memset(input, 0, 512);
        cursor = 0;
        move(y - 2, 1);
        for (uint i = 0; i < x && i < 512; i++)
          delch();
      }
      break;
    }
    case 8:
    case 23: {
      while (input[cursor] != ' ' && cursor > 0) {
        input[--cursor] = 0;
        move(y - 2, cursor + 1);
        delch();
      }
      break;
    }
    case KEY_BACKSPACE: {
      if (cursor == 0)
        break;
      input[--cursor] = 0;
      move(y - 2, cursor + 1);
      delch();
      break;
    }
    default: {
      if (cursor >= 511)
        break;
      if (c < 127 && c > 31)
        input[cursor++] = c;
      break;
    }
    }
    move(y - 2, 1);
    for (uint i = 0; i < x && i < 512; i++) {
      if (input[i] == 0)
        break;
      addch(input[i]);
    }
    refresh();
    pthread_mutex_unlock(&mutex);
  }
}

void tuiDrawList(void) {}

void tuiDrawChannel(IrcServer *server, IrcChannel *channel) {
  if (NULL == server || NULL == channel)
    return;

  clear();
  uint x, y;
  x = y = 0;
  getmaxyx(w, y, x);
  uint i = 0, s = 0;
  move(y - 3, 1);
  char line[254] = {0};
  snprintf(line, 254, "%s / %s - %s", server->host, channel->name,
           server->me.nick);
  addstr(line);
  while (i < channel->lenMsgs && y - i > 4) {
    IrcMsg m = channel->msgs[channel->lenMsgs - 1 - i];
    s = 0;
    while (s < WID - 1 && m.ident[s] != 0)
      s++;
    move(y - i - 4, WID - s - 1);
    uint u;
    for (u = 0; u < s && m.ident[u] != 0; u++)
      addch(m.ident[u]);
    move(y - i - 4, WID);
    addch(m.sep);
    /* i += ceil((double)strlen(&m.msg[u]) / (double)(x - WID + 3)); */
    i++;
  }
  i = 0;
  s = WID + 3;
  while (i < channel->lenMsgs && y - i > 4) {
    IrcMsg m = channel->msgs[channel->lenMsgs - 1 - i];
    move(y - i - 4, WID + 3);
    for (uint u = 0; m.msg[u] != 0 && u < 512; u++)
      addch(m.msg[u]);
    i++;
  }
  refresh();
}

void tuiDeinit(void) { endwin(); }
