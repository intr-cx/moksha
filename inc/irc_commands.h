#ifndef INCLUDE_INC_IRC_COMMANDS_H_
#define INCLUDE_INC_IRC_COMMANDS_H_

#include "irc.h"

extern char ircClientCmds[LEN_ARRAY][8];

void ircCmd(int sockfd, char *cmd, char *data);

void ircCmdPassword(IrcServer *, char *pass);
void ircCmdNick(IrcServer *, char *nick);
void ircCmdUser(IrcServer *, char *user, char *mode, char *realName);
void ircCmdOper(IrcServer *, char *name, char *pass);
void ircCmdMode(IrcServer *, char *nick, char *modes);
void ircCmdService(IrcServer *, char *nick, char *_reserved1,
                   char *distribution, char *type, char *_reserved2,
                   char *info);
void ircCmdQuit(IrcServer *, char *reason);
void ircCmdSquit(IrcServer *, char *other, char *reason);

// pass NULL to `passes` for no password
void ircCmdChJoin(IrcServer *, char *chans, char *passes);
void ircCmdChPart(IrcServer *, char *chans, char *reason);
void ircCmdChMode(IrcServer *, char *chan, char *modes, char *nick);
void ircCmdChTopic(IrcServer *, char *topic);
void ircCmdChNames(IrcServer *, char *chans);
void ircCmdChList(IrcServer *, char *chans);
void ircCmdChInvite(IrcServer *, char *nick, char *chan);
void ircCmdChKick(IrcServer *, char *chan, char *nick, char *reason);

void ircCmdClPrivmsg(IrcServer *, char *target, char *msg);
void ircCmdClNotice(IrcServer *, char *target, char *msg);
void ircCmdClSquery(IrcServer *);

void ircCmdSvMotd(IrcServer *, char *target);
void ircCmdSvLusers(IrcServer *, char *mask, char *target);
void ircCmdSvVersion(IrcServer *, char *target);
void ircCmdSvStats(IrcServer *, char *query, char *target);
void ircCmdSvTime(IrcServer *, char *target);
/* void ircCmdSvConnect(); */
/* void ircCmdSvTrace(); */
/* void ircCmdSvAdmin(); */
void ircCmdSvInfo(IrcServer *, char *target);
void ircCmdSvServlist(IrcServer *, char *mask, char *type);
void ircCmdSvSquery(IrcServer *, char *servicename, char *text);
void ircCmdSvWho(IrcServer *, char *mask, bool operator);
void ircCmdSvWhois(IrcServer *, char *target, char *masks);
/* void ircCmdSvWhowas(); */
/* void ircCmdSvKill(); */
void ircCmdSvPing(IrcServer *, char *server, char *server2);
void ircCmdSvPong(IrcServer *, char *server, char *server2);

#endif // INCLUDE_INC_IRC_COMMANDS_H_
