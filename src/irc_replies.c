#include "irc_replies.h"
#include "irc.h"
#include "irc_commands.h"
#include "irc_numerics.h"
#include "tui.h"
#include "util.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char ircCmdNames[LEN_ARRAY][8] = {
    "JOIN", "PART",   "TOPIC", "PRIVMSG", "PING",
    "PONG", "NOTICE", "QUIT",  "\0",
};

void (*ircReplyFuncs[LEN_ARRAY])(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG],
                                 size_t) = {
    ircRplJoin, ircRplPart,   ircRplTopic, ircRplPrivmsg, ircRplPing,
    ircRplPong, ircRplNotice, ircRplQuit};

void ircHandleReplyCmd(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                       size_t argc) {
  for (size_t i = 0; i < LEN_ARRAY; i++) {
    if (strcmp(args[1], ircCmdNames[i]) == 0) {
      ircReplyFuncs[i](server, args, argc);
      break;
    }
  }
}

void ircHandleReplyNum(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                       size_t argc) {
  switch (atoi(args[1])) {
  case RPL_WELCOME:
  case RPL_CREATED:
  case RPL_MOTD:
  case RPL_MYINFO: {
    IrcChannel *ch = &server->channels[0];
    ircServerAddMsg(ch, "server", '|', args[3]);
    break;
  }
		case RPL_WHOISUSER: {
			IrcUser *u = getUserFromStr(&server->channels[server->curChannel], args[3]);
			strlcpy(u->nick, args[3], LEN_NICK);
			strlcpy(u->user, args[4], LEN_NICK);
			strlcpy(u->host, args[5], LEN_NICK);
			// args[6] = *
			strlcpy(u->name, args[7], LEN_NICK);
			char msg[128];
			snprintf("%s is %s, %s at %s", 128, u->nick, u->user, u->name, u->host);
			ircServerAddMsg(&server->channels[0], "server", '|', msg);
			break;
		}
		case RPL_WHOISOPERATOR: {
			break;
		}
		case RPL_WHOISHOST: {
			break;
		}
  case RPL_TOPIC: {
    ircRplTopic(server, args, argc);
    break;
  }
  case RPL_NAMREPLY: {
    if (argc > 5) {
      IrcChannel *channel = getChannelFromStr(server, args[4]);
      if (NULL == channel)
        break;
      char buf[64] = {0};
      uint c = 0, cc = 0;
      while (c < LEN_PROTMSG && args[5][c] != 0) {
        buf[cc] = args[5][c];
        cc++;
        c++;
        if (args[5][c] == ' ' || args[5][c] == 0 || cc >= 63) {
          buf[cc] = 0;
          cc = 0;
          ircChannelAddNick(channel, buf);
        }
      }
    }
    break;
  }
  default:
    break;
  }
}

int fill(char *target, char *buf, size_t len) {
  bool finalarg = (buf[0] == ':'); // ignore spaces when started by colon
  uint c = 0;
  while (c < len && (finalarg || buf[c] != ' ')) {
    if (buf[c] == 0) {
      return -1;
    }
    uint bc = c + finalarg; // skip over the colon
    if (bc < len)           // since it can overshoot now
      target[c] = buf[bc];
    c++;
  }
  target[c] = 0;

  return c;
}

int tag(IrcTag *tags, char *buf, size_t len) {
  uint c = 0, cc = 0, t = 0;

  bool isKey = true;

  while (c < len && buf[c] != ' ') {
    if (buf[c] == 0) {
      return -1;
    }
    if (buf[c] == '=') {
      isKey = false;
      c++;
      cc = 0;
      continue;
    }
    if (buf[c] == ';') {
      isKey = false;
      c++;
      cc = 0;
      t++;
      if (t >= 16)
        break;
      continue;
    }
    if (cc < 16) {
      if (isKey)
        tags[t].key[cc] = buf[c];
      else
        tags[t].val[cc] = buf[c];
    }
    cc++;
    c++;
  }

  return c;
}

void ircRpl(IrcServer *s, char *buf, size_t len) {
  if (len <= 0) // empty
    return;
  fprintf(outfile, "%s\n", buf);
  char args[LEN_PROTARRAY][LEN_PROTMSG] = {0};
  size_t argc = 0;
  uint c = 1; // skip over the first : or @

  if (buf[0] != ':' && buf[0] != '@') {
    argc = 1;
    c = 0;
  }

  IrcTag tags[16] = {0};

  if (buf[0] == '@') {
    int t = tag(tags, buf + 1, len - 1);
    if (-1 == t) // malformed
      return;
    c += t + 1;
  }

  while (c < len && argc < LEN_PROTARRAY) {
    int f = fill(args[argc], buf + c, len - c);
    if (-1 == f)
      break;
    c += f + 1;
    argc++;
  }

  /* pthread_mutex_lock(&mutex); */
  // the command
  if (isNumeric(args[1], 3))
    ircHandleReplyNum(s, args, argc); // irc_numerics.h
  else
    ircHandleReplyCmd(s, args, argc);

  tuiDrawChannel(s, &s->channels[s->curChannel]);

  /* pthread_mutex_unlock(&mutex); */
}

void fillUser(IrcUser *u, char *nick, char *user, char *name, char *mode,
              char *host) {
  strlcpy(u->nick, nick, LEN_NICK);
  strlcpy(u->user, user, LEN_NICK);
  strlcpy(u->name, name, LEN_NICK);
  strlcpy(u->mode, mode, LEN_MODE);
  strlcpy(u->host, host, LEN_NICK);
}

void ircRplNotice(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                  size_t argc) {
  if (argc <= 2)
    return;
  IrcChannel *c;
  if (args[2][0] == '*')
    c = NULL;
  else
    c = getChannelFromStr(server, args[2]);
  ircServerAddMsg(NULL == c ? &server->channels[0] : c, "server", '!', args[3]);
}

void ircRplPrivmsg(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                   size_t argc) {
  if (argc <= 3)
    return;
  IrcUser u = {0};
  createUserFromStr(&u, args[0], LEN_PROTMSG);
  IrcChannel *c;
  if (strcmp(args[2], server->me.nick) == 0) {
    c = getChannelFromStr(server, u.nick);
    if (NULL == c)
      c = ircServerAddChannel(server, u.nick);
  } else {
    c = getChannelFromStr(server, args[2]);
  }
  ircServerAddMsg(c, u.nick, ':', args[3]);
}

void ircRplJoin(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                size_t argc) {
  (void) argc;
  IrcUser u = {0};
  createUserFromStr(&u, args[0], LEN_PROTMSG);
  if (strcmp(u.user, server->me.user) == 0) {
    ircServerAddChannel(server, args[2]);
  } else {
    IrcChannel *ch = getChannelFromStr(server, args[2]);
    if (NULL != ch)
      ircChannelAddUser(ch, &u);
  }
}

void ircRplPart(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                size_t argc) {
  (void) argc;
  IrcUser u = {0};
  createUserFromStr(&u, args[0], LEN_PROTMSG);
  if (strcmp(u.user, server->me.user) == 0) {
    ircServerRemoveChannel(server, args[2]);
  } else {
    IrcChannel *ch = getChannelFromStr(server, args[2]);
    if (NULL != ch)
      ircChannelRemoveUser(ch, &u);
  }
}

void ircRplPing(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                size_t argc) {
  (void) argc;
  ircCmdSvPong(server, args[2], NULL);
}

void ircRplPong(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                size_t argc) {
  (void) argc;
  (void) args;
  struct timespec t = {0};
  clock_gettime(CLOCK_MONOTONIC, &t);
  server->pong = t.tv_nsec;
}

void ircRplNick(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                size_t argc) {
  (void) argc;
  IrcUser u = {0};
  createUserFromStr(&u, args[0], LEN_PROTMSG);
  if (strcmp(u.user, server->me.user) != 0) {
    strlcpy(server->me.user, args[2], 64);
    return;
  }

  for (uint i = 0; i < server->lenChannels; i++) {
    IrcChannel *ch = &server->channels[i];
    IrcUser *us = getUserFromStr(ch, u.nick);
    if (NULL != us)
      strlcpy(us->nick, args[2], 64);
  }
}

void ircRplTopic(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                 size_t argc) {
  (void) argc;
  IrcChannel *channel = getChannelFromStr(server, args[3]);
  if (NULL == channel)
    return;
  strlcpy(channel->topic, args[4], 256);
}

void ircRplQuit(IrcServer *server, char args[LEN_PROTARRAY][LEN_PROTMSG],
                size_t argc) {
  (void) argc;
  IrcUser u = {0};
  createUserFromStr(&u, args[0], LEN_PROTMSG);
  if (strcmp(u.user, server->me.user) != 0) {
    ircDisconnect(server, NULL);
    return;
  }

  for (uint i = 0; i < server->lenChannels; i++) {
    IrcChannel *ch = &server->channels[i];
    IrcUser *us = getUserFromStr(ch, u.nick);
    ircChannelRemoveUser(ch, us);
  }
}
