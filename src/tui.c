#include "irc.h"
#include "tui.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char input[512];
uint cursor = 0;
WINDOW *w;
bool running = true;

void tuiDrawList(uint *sep, uint height);
void tuiDrawChannel(IrcServer *, IrcChannel *, uint sep, uint width,
                    uint height);
void tuiDrawCmd(uint width, uint height);

void tuiInit(void) {
  w = initscr();
  noecho();
  cbreak();
  nonl();
  keypad(w, true);
  start_color();
  uint x, y;
  x = y = 0;
  getmaxyx(w, y, x);
  move(y - 2, cursor + 1);
}

void tuiLoop(void) {
  int c;
  while (running && (c = getch()) != 0) {
    pthread_mutex_lock(&mutex);
    move(0, 0);
    fprintf(outfile, "%d\n", c);
    switch (c) {
    case KEY_PPAGE: {
      if (NULL != servers) {
        if (servers[curServer].curChannel > 0)
          servers[curServer].curChannel--;
        else if (curServer > 0) {
          curServer--;
          servers[curServer].curChannel = servers[curServer].lenChannels - 1;
        }
      }
      break;
    }
    case KEY_NPAGE: {
      if (NULL != servers) {
        if (servers[curServer].curChannel < servers[curServer].lenChannels - 1)
          servers[curServer].curChannel++;
        else if (curServer < lenServers - 1) {
          curServer++;
          servers[curServer].curChannel = 0;
        }
      }
      break;
    }
    case 10: {
      if (cursor != 0) {
        IrcServer *s = NULL == servers ? NULL : &servers[curServer];
        IrcChannel *ch = NULL == servers || s->lenChannels == 0
                             ? NULL
                             : &s->channels[s->curChannel];
        ircChannelSendMsg(s, ch, input);
        memset(input, 0, 512);
        cursor = 0;
      }
      break;
    }
    case 8:
    case 23: {
      while (input[cursor] != ' ' && cursor > 0) {
        input[--cursor] = 0;
      }
      break;
    }
    case KEY_BACKSPACE: {
      if (cursor == 0)
        break;
      input[--cursor] = 0;
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
    draw();
    pthread_mutex_unlock(&mutex);
  }
}

void draw(void) {
  uint sep = 8;

  uint width, height;
  width = height = 0;
  getmaxyx(w, height, width);
  clear();
  tuiDrawList(&sep, height);

  if (NULL != servers && NULL != servers[curServer].channels) {
    tuiDrawChannel(&servers[curServer],
                   &servers[curServer].channels[servers[curServer].curChannel],
                   sep, width, height);
  }
  tuiDrawCmd(width, height);

  refresh();
}

void tuiDrawList(uint *sep, uint height) {
  uint c = 0;
  uint sl;
  for (uint i = 0; i < lenServers && i < height; i++) {
    IrcServer *s = &servers[i];
    sl = strlen(s->host);
    if (*sep < sl + 2)
      *sep = sl + 2;
    for (uint k = 0; k < *sep && s->host[k] != 0; k++) {
      move(i + c, k + 1);
      addch(s->host[k]);
    }
    for (uint j = 0; j < s->lenChannels && j < height; j++) {
      IrcChannel *ch = &s->channels[j];
      sl = strlen(ch->name);
      if (*sep < sl + 3)
        *sep = sl + 3;
      if (curServer == i && s->curChannel == j) {
        move(i + j + c + 2, 0);
        addch('*');
      }
      for (uint k = 0; k < *sep && ch->name[k] != 0; k++) {
        move(i + j + c + 2, k + 1);
        addch(ch->name[k]);
      }
    }
    c += s->lenChannels;
  }
}

void tuiDrawCmd(uint width, uint height) {
  for (uint u = 0; u < width; u++) {
    move(height - 2, u + 1);
    addch(input[u] == 0 ? ' ' : input[u]);
  }
  move(height - 2, cursor + 1);
}

void tuiDrawChannel(IrcServer *server, IrcChannel *channel, uint sep,
                    uint width, uint height) {
  const uint nicksep = 16;
  width -= sep;
  uint i = 0, s = 0;
  move(height - 3, 1);
  char line[254] = {0};
  snprintf(line, 254, "%s / %s - %s", server->host, channel->name,
           server->me.nick);
  addstr(line);
  while (i < channel->lenMsgs && height - i > 4) {
    IrcMsg m = channel->msgs[channel->lenMsgs - 1 - i];
    uint t = ceil((double)strlen(m.msg) / (double)(width - (nicksep + 3)));
    s = 0;
    while (s < nicksep - 1 && m.ident[s] != 0)
      s++;
    move(height - i - t - 4, nicksep - s - 1 + sep);
    uint u;
    for (u = 0; u < s && m.ident[u] != 0; u++)
      addch(m.ident[u]);
    move(height - i - t - 4, nicksep + sep);
    addch(m.sep);
    i += t;
  }
  i = 0;
  s = nicksep + 3;
  while (i < channel->lenMsgs) {
    IrcMsg m = channel->msgs[channel->lenMsgs - 1 - i];
    uint msgwid = ceil((double)strlen(m.msg) / (double)(width - s));
    i += msgwid;
    for (int t = msgwid; t >= 0; t--) {
      for (uint u = 0; u < (width - s); u++) {
        uint mindex = u + (t * (width - s));
        if (mindex >= LEN_MSG || m.msg[mindex] == 0)
          break;
        move(height - (i - t) - 4, s + u - 1 + sep);
        if (m.msg[mindex] > 30)
          addch(m.msg[mindex]);
        else {
          // TODO colors & attributes
        }
      }
    }
  }
  move(height - 2, cursor + 1);
}

void tuiDeinit(void) { endwin(); }
