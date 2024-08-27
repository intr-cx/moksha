#include "irc.h"
#include "irc_commands.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

char ircClientCmds[LEN_ARRAY][8] = {
    0 // TODO
};

void ircCmd(int sockfd, char *cmd, char *data) {
  if (sockfd < 0) {
    fprintf(outfile, "Socket unavailable\n");
    return;
  }
  if (NULL == data)
    dprintf(sockfd, "%s\r\n", cmd);
  else
    dprintf(sockfd, "%s %s\r\n", cmd, data);
}

void ircCmdPassword(IrcServer *server, char *pass) {
  ircCmd(server->sockfd, "PASS", pass);
}

void ircCmdNick(IrcServer *server, char *nick) {
  ircCmd(server->sockfd, "NICK", nick);
}

void ircCmdOper(IrcServer *server, char *name, char *pass) {
  char data[128];
  sprintf(data, "%s %s", name, pass);
  ircCmd(server->sockfd, "OPER", data);
}

void ircCmdUser(IrcServer *server, char *user, char *mode, char *realName) {
  char data[128] = {0};
  sprintf(data, "%s %s * :%s", user, mode, realName);
  ircCmd(server->sockfd, "USER", data);
}

void ircCmdMode(IrcServer *server, char *nick, char *modes) {
  char data[128];
  sprintf(data, "%s %s", nick, modes);
  ircCmd(server->sockfd, "MODE", data);
}

void ircCmdService(IrcServer *server, char *nick, char *_reserved1,
                   char *distribution, char *type, char *_reserved2,
                   char *info) {
  char data[128];
  sprintf(data, "%s %s %s %s %s :%s", nick, _reserved1, distribution, type,
          _reserved2, info);
  ircCmd(server->sockfd, "SERVICE", data);
}

void ircCmdQuit(IrcServer *server, char *reason) {
  if (NULL == reason)
    ircCmd(server->sockfd, "QUIT", NULL);
  else {
    char data[128];
    sprintf(data, ":%s", reason);
    ircCmd(server->sockfd, "QUIT", data);
  }
}

void ircCmdSquit(IrcServer *server, char *other, char *reason) {
  char data[128];
  sprintf(data, "%s :%s", other, reason);
  ircCmd(server->sockfd, "SQUIT", data);
}

void ircCmdChJoin(IrcServer *server, char *chans, char *passes) {
  if (NULL == passes)
    ircCmd(server->sockfd, "JOIN", chans);
  else {
    char data[128];
    sprintf(data, "%s %s", chans, passes);
    ircCmd(server->sockfd, "JOIN", data);
  }
}

void ircCmdChPart(IrcServer *server, char *chans, char *reason) {
  if (NULL == reason)
    ircCmd(server->sockfd, "PART", chans);
  else {
    char data[128];
    sprintf(data, "%s %s", chans, reason);
    ircCmd(server->sockfd, "PART", data);
  }
}

void ircCmdChMode(IrcServer *server, char *chan, char *modes, char *nick) {
  char data[128];
  sprintf(data, "%s %s %s", chan, modes, nick);
  ircCmd(server->sockfd, "MODE", data);
}

void ircCmdChTopic(IrcServer *server, char *topic) {
  if (NULL == topic)
    ircCmd(server->sockfd, "TOPIC", NULL);
  else {
    char data[128];
    sprintf(data, ":%s", topic);
    ircCmd(server->sockfd, "TOPIC", data);
  }
}

void ircCmdChNames(IrcServer *server, char *chans) {
  ircCmd(server->sockfd, "NAMES", chans);
}

void ircCmdChList(IrcServer *server, char *chans) {
  ircCmd(server->sockfd, "LIST", chans);
}

void ircCmdChInvite(IrcServer *server, char *nick, char *chan) {
  char data[128];
  sprintf(data, "%s %s", nick, chan);
  ircCmd(server->sockfd, "INVITE", data);
}

void ircCmdChKick(IrcServer *server, char *chan, char *nick, char *reason) {
  char data[128];
  if (NULL == reason)
    sprintf(data, "%s %s", chan, nick);
  else
    sprintf(data, "%s %s :%s", chan, nick, reason);
  ircCmd(server->sockfd, "KICK", data);
}

void ircCmdClPrivmsg(IrcServer *server, char *target, char *msg) {
  char data[LEN_MSG + LEN_NICK + 2];
  sprintf(data, "%s :%s", target, msg);
  ircCmd(server->sockfd, "PRIVMSG", data);
}

void ircCmdClNotice(IrcServer *server, char *target, char *msg) {
  char data[512];
  sprintf(data, "%s :%s", target, msg);
  ircCmd(server->sockfd, "NOTICE", data);
}

void ircCmdSvMotd(IrcServer *server, char *target) {
  ircCmd(server->sockfd, "MOTD", target);
}

void ircCmdSvLusers(IrcServer *server, char *mask, char *target) {
  if (NULL == target)
    ircCmd(server->sockfd, "LUSERS", mask);
  else {
    char data[128];
    sprintf(data, "%s %s", mask, target);
    ircCmd(server->sockfd, "LUSERS", data);
  }
}

void ircCmdSvVersion(IrcServer *server, char *target) {
  ircCmd(server->sockfd, "VERSION", target);
}

void ircCmdSvStats(IrcServer *server, char *query, char *target) {
  if (NULL == target)
    ircCmd(server->sockfd, "STATS", query);
  else {
    char data[128];
    sprintf(data, "%s %s", query, target);
    ircCmd(server->sockfd, "STATS", data);
  }
}

void ircCmdSvTime(IrcServer *server, char *target) {
  ircCmd(server->sockfd, "TIME", target);
}

void ircCmdSvInfo(IrcServer *server, char *target) {
  ircCmd(server->sockfd, "INFO", target);
}

void ircCmdSvServlist(IrcServer *server, char *mask, char *type) {
  if (NULL == mask)
    ircCmd(server->sockfd, "SERVLIST", NULL);
  else if (NULL == type)
    ircCmd(server->sockfd, "SERVLIST", mask);
  else {
    char data[128];
    sprintf(data, "%s %s", mask, type);
    ircCmd(server->sockfd, "SERVLIST", data);
  }
}

void ircCmdSvSquery(IrcServer *server, char *servicename, char *text) {
  char data[128];
  sprintf(data, "%s :%s", servicename, text);
  ircCmd(server->sockfd, "SQUERY", data);
}

void ircCmdSvWho(IrcServer *server, char *mask, bool operator) {
  ircCmd(server->sockfd, "WHO", strcat(mask, operator? " o" : ""));
}

void ircCmdSvWhois(IrcServer *server, char *target, char *masks) {
  if (NULL == target)
    ircCmd(server->sockfd, "WHOIS", masks);
  else {
    char data[128];
    sprintf(data, "%s %s", target, masks);
    ircCmd(server->sockfd, "WHOIS", data);
  }
}

void ircCmdSvPing(IrcServer *server, char *target, char *target2) {
  if (NULL == target2)
    ircCmd(server->sockfd, "PING", target);
  else {
    char data[128];
    sprintf(data, "%s %s", target, target2);
    ircCmd(server->sockfd, "PING", data);
  }
  struct timespec t = {0};
  clock_gettime(CLOCK_MONOTONIC, &t);
  server->ping = t.tv_nsec;
}

void ircCmdSvPong(IrcServer *server, char *target, char *target2) {
  if (NULL == target2)
    ircCmd(server->sockfd, "PONG", target);
  else {
    char data[128];
    sprintf(data, "%s %s", target, target2);
    ircCmd(server->sockfd, "PONG", data);
  }
}
