#include "irc.h"
#include "irc_commands.h"
#include "irc_replies.h"
#include "tui.h"
#include "util.h"
#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define macroReallocShrink(ptr, len, _mal, type)                               \
  if (len >= _mal) {                                                           \
    while (len >= _mal)                                                        \
      _mal *= 2;                                                               \
    ptr = (type *)realloc(ptr, sizeof(type) * _mal);                           \
    if (NULL == ptr)                                                           \
      exit(-1);                                                                \
  }

#define macroReallocGrow(ptr, len, _mal, type)                                 \
  if (len * 4 < _mal) {                                                        \
    _mal = len;                                                                \
    ptr = (type *)realloc(ptr, sizeof(type) * _mal);                           \
    if (NULL == ptr)                                                           \
      exit(-1);                                                                \
  }

IrcServer *servers;
uint lenServers, _mallocServers, curServer;
pthread_mutex_t mutex;

IrcUser userTemplate;

FILE *outfile;

int createUserFromStr(IrcUser *user, char *str, size_t len) {
  char buf[64];
  uint c = 0, cc = 0;
  bool host = false;
  while (c < len && str[c] != ' ' && str[c] != 0 && cc < 64) {
    if (str[c] == '!') {
      c++;
      buf[cc] = 0;
      strncpy(user->nick, buf, cc);
      cc = 0;
      continue;
    }
    if (str[c] == '@') {
      c++;
      buf[cc] = 0;
      strncpy(user->user, buf, cc);
      cc = 0;
      host = true;
      continue;
    }
    buf[cc] = str[c];
    c++;
    cc++;
  }
  if (host) {
    buf[cc] = 0;
    inet_aton(buf, (struct in_addr *)&user->host);
  }
  return 0;
}

IrcChannel *getChannelFromStr(IrcServer *server, char *query) {
  if (NULL == server || NULL == server->channels)
    return NULL;

  for (uint i = 0; i < server->lenChannels; i++) {
    if (strcmp(server->channels[i].name, query) == 0) {
      return &server->channels[i];
    }
  }

  return NULL;
}

IrcUser *getUserFromStr(IrcChannel *channel, char *query) {
  if (NULL == channel || NULL == channel->users)
    return NULL;

  for (uint i = 0; i < channel->lenUsers; i++) {
    if (strcmp(channel->users[i].nick, query) == 0) {
      return &channel->users[i];
    }
  }

  return NULL;
}

void ircDisconnect(IrcServer *server, char *reason) {
  if (server->sockfd == -1) {
    return;
  }
  ircCmdQuit(server, reason);
  close(server->sockfd);
  server->sockfd = -1;
  pthread_join(server->thrRecv, NULL);
  pthread_join(server->thrTimer, NULL);
  pthread_mutex_destroy(&mutex);
  // more?
}

void ircServerRemove(IrcServer *server) {
  if (lenServers == 0)
    return;
  uint loc = 0;
  while (loc < lenServers) {
    IrcServer *s = &servers[loc];
    if (strcmp(server->host, s->host)) {
    }
    loc++;
  }
  if (loc == lenServers)
    return;
  while (loc < lenServers - 1) {
    memcpy(&servers[loc], &servers[loc + 1], sizeof(IrcServer));
  }
  lenServers--;
  macroReallocShrink(servers, lenServers, _mallocServers, IrcServer);
}

ulong getServerPing(IrcServer *server) {
  return round(1E-4 * (server->pong - server->ping));
}

void ircConnect(IrcServer *server) {
  server->sockfd = initSocket(&server->addr);
  if (server->sockfd < 0)
    return;

  pthread_mutex_init(&mutex, NULL);

  pthread_create(&server->thrRecv, NULL, ircThreadRecv, server);
  pthread_create(&server->thrTimer, NULL, ircThreadTimer, server);

  ircCmdNick(server, server->me.nick);
  ircCmdUser(server, server->me.user, server->me.mode, server->me.name);
  // more?
}

int initSocket(struct sockaddr_in *addr) {
  int s;
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(outfile, "Failed to create socket: %s\n", strerror(errno));
    return -1;
  }

  int c;
  if ((c = connect(s, (struct sockaddr *)addr, sizeof(*addr))) < 0) {
    fprintf(outfile, "Failed to connect: %s\n", strerror(errno));
    return -1;
  }

  return s;
}

void ircUserCreate(IrcUser *u, char *nick, char *user, char *name) {
  strlcpy(u->nick, nick, 64);
  strlcpy(u->user, user, 64);
  strlcpy(u->name, name, 64);
  strcpy(u->mode, "0");
  memset(&u->host, 0, sizeof(struct sockaddr_in));
}

void ircServerCreate(IrcServer *server, char *host, uint16_t port,
                     IrcUser user) {
  strcpy(server->host, host);
  server->port = port;
  server->lenChannels = 0;
  server->_mallocChannels = 1;
  server->channels = (IrcChannel *)malloc(sizeof(IrcChannel));
  if (NULL == server->channels)
    exit(-1);
  server->me = user;
  server->addr = getFromHost(host);
  server->addr.sin_port = htons(port);
  server->ping = 0;
  server->pong = 0;
  IrcChannel *ch = ircServerAddChannel(server, "MESSAGES");
  ch->writeable = 0;
}

IrcServer *ircAddServer(IrcServer *server) {
  servers[lenServers++] = *server;
  curServer = lenServers - 1;
  macroReallocGrow(servers, lenServers, _mallocServers, IrcServer);
  return &servers[curServer];
}

void ircServerFree(IrcServer *server) {
  if (NULL != server->channels) {
    for (uint i = 0; i < server->lenChannels; i++) {
      ircChannelFree(&server->channels[i]);
    }
  }
  free(server->channels);
}

void ircChannelFree(IrcChannel *channel) {
  if (NULL != channel->users)
    free(channel->users);
  free(channel->msgs);
}

IrcChannel *ircServerAddChannel(IrcServer *server, char *name) {
  IrcChannel channel = {0};
  strlcpy(channel.name, name, 64);
  channel.writeable = 1;

  channel.users = (IrcUser *)malloc(sizeof(IrcUser));
  channel.msgs = (IrcMsg *)malloc(sizeof(IrcMsg) * 8);

  if (NULL == channel.users || NULL == channel.msgs)
    exit(-1);
  channel._mallocUsers = 1;
  channel._mallocMsgs = 8;

  server->channels[server->lenChannels++] = channel;
  server->curChannel = server->lenChannels - 1;
  macroReallocGrow(server->channels, server->lenChannels,
                   server->_mallocChannels, IrcChannel);
  return &server->channels[server->curChannel];
}

void ircChannelAddNick(IrcChannel *channel, char *name) {
  IrcUser nick = {0};
  strlcpy(nick.nick, name, 64);

  ircChannelAddUser(channel, &nick);
}

void ircServerAddMsg(IrcChannel *channel, char *ident, char sep, char *msg) {
  IrcMsg m = {0};
  strlcpy(m.ident, ident, LEN_NICK);
  strlcpy(m.msg, msg, LEN_MSG);
  m.sep = sep;

  channel->msgs[channel->lenMsgs++] = m;
  macroReallocGrow(channel->msgs, channel->lenMsgs, channel->_mallocMsgs,
                   IrcMsg);
}

void ircChannelAddUser(IrcChannel *channel, IrcUser *user) {
  channel->users[channel->lenUsers++] = *user;
  macroReallocGrow(channel->users, channel->lenUsers, channel->_mallocUsers,
                   IrcUser);
}

void ircServerRemoveChannel(IrcServer *server, char *name) {
  uint loc = 0;
  while (loc < server->lenChannels) {
    if (strcmp(server->channels[loc].name, name) == 0)
      break;
    loc++;
  }
  if (server->lenChannels == loc)
    return;
  while (loc < server->lenChannels - 1) {
    memcpy(&server->channels[loc], &server->channels[loc + 1],
           sizeof(IrcChannel));
    loc++;
  }
  server->lenChannels--;
  macroReallocShrink(server->channels, server->lenChannels,
                     server->_mallocChannels, IrcChannel);
}

void ircChannelRemoveUser(IrcChannel *channel, IrcUser *user) {
  uint loc = 0;
  while (loc < channel->lenUsers) {
    if (strcmp(channel->users[loc].nick, user->nick) == 0)
      break;
    loc++;
  }
  if (channel->lenUsers == loc)
    return;
  while (loc < channel->lenUsers - 1) {
    memcpy(&channel->users[loc], &channel->users[loc + 1], sizeof(IrcUser));
    loc++;
  }
  channel->lenUsers--;
  macroReallocShrink(channel->users, channel->lenUsers, channel->_mallocUsers,
                     IrcUser);
}

void *ircThreadTimer(void *argp) {
  IrcServer *server = (IrcServer *)argp;
  while (server->sockfd >= 0) {
    struct timespec t = {0};
    clock_gettime(CLOCK_MONOTONIC, &t);
    if (t.tv_nsec - server->ping > 2E7) {
      ircCmdSvPing(server, server->host, NULL);
      server->ping = t.tv_nsec;
    }
    sleep(1);
  }
  return NULL;
}

void *ircThreadRecv(void *argp) {
  IrcServer *server = (IrcServer *)argp;
  while (server->sockfd >= 0) {
    const size_t bufsize = 2048;
    char buf[bufsize];
    size_t res = recv(server->sockfd, buf, bufsize - 1, 0);
    if (res <= 0) {
      continue;
    }
    buf[res] = 0;
    char linebuf[bufsize];
    uint c, cc;
    c = cc = 0;
    while (c < bufsize - 1 && buf[c] != 0) {
      if (buf[c] == '\r' && buf[c + 1] == '\n') {
        linebuf[cc] = 0;
        ircRpl(server, linebuf, cc);
        c += 2;
        cc = 0;
        continue;
      }
      linebuf[cc] = buf[c];
      cc++;
      c++;
    }
  }
  return NULL;
}
void ircChannelSendMsg(IrcServer *server, IrcChannel *channel, char *message) {
  if (NULL == server || NULL == channel || message[0] == '/') {
    ircChannelSendCmd(server, channel, &message[message[0] == '/' ? 1 : 0]);
  } else {
    if (channel->writeable == 0)
      return;
    uint l, r;
    strTrim(&l, &r, message);
    ircCmdClPrivmsg(server, channel->name, &message[l]);
    ircServerAddMsg(channel, server->me.nick, ':', &message[l]);
  }
  if (lenServers > 0 && servers[curServer].lenChannels > 0) {
    IrcServer *s = &servers[curServer];
    IrcChannel *c = &s->channels[s->curChannel];
    if (s->host == server->host && c->name == channel->name)
      tuiDrawChannel(s, c);
  }
}

void ircChannelSendCmd(IrcServer *server, IrcChannel *channel, char *buf) {
  if (NULL == server)
    return;
  char args[64][1024] = {0};
  uint c, cc, argc;
  c = cc = argc = 0;
  bool q, qq;
  q = qq = false;
  while (argc < 64 && buf[c] != 0) {
    if (buf[c] == '\'' && (c > 0 && buf[c - 1] != '\\')) {
      q = !q;
      c++;
      continue;
    }
    if (buf[c] == '"' && (c > 0 && buf[c - 1] != '\\')) {
      qq = !(!q || qq);
      c++;
      continue;
    }
    if (!q && !qq && buf[c] == ' ') {
      if (cc < 1024 - 1)
        args[argc][cc + 1] = 0;
      c++;
      argc++;
      cc = 0;
    }

    args[argc][cc++] = buf[c++];
  }
  for (size_t i = 0; i < ARRAY_LEN && ircCmdNames[i][0] != 0; i++) {
    if (strcasecmp(args[0], ircCmdNames[i]) == 0) {
      uint j = 0;
      switch (i) {
      case 0: { // JOIN
        if (argc > 0)
          ircCmdChJoin(server, args[1], argc > 1 ? args[2] : NULL);
        return;
      }
      case 1: { // PART
        if (argc > 2) {
          while (buf[j] != 0 && buf[j++] != ' ')
            ;
        }
        if (argc > 0)
          ircCmdChPart(server, args[1], argc > 2 ? &buf[j] : NULL);
        return;
      }
      case 2: { // TOPIC
        while (buf[j++] != ' ')
          ;
        ircCmdChTopic(server, &buf[j - 1]);
        return;
      }
      case 7: { // QUIT
        if (argc > 1) {
          while (buf[j] != 0 && buf[j++] != ' ')
            ;
        }
        ircDisconnect(server, argc > 1 ? &buf[j] : NULL);
        return;
      }
      }
      break;
    }
  }
  if (strcasecmp("connect", args[0]) == 0) {
    size_t port = argc > 1 ? atoi(args[2]) : 6667;
    IrcServer s = {0};
    IrcUser u = userTemplate;
    ircServerCreate(&s, args[1], port, u);
    ircConnect(ircAddServer(&s));
    return;
  }
  if (strcasecmp("whois", args[0]) == 0) {
    ircCmdSvWhois(server, NULL, args[1]);
    return;
  }
}
