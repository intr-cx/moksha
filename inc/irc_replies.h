#ifndef INCLUDE_INC_IRC_REPLIES_H_
#define INCLUDE_INC_IRC_REPLIES_H_

#include "irc.h"

// array of function pointers corresponding to received commands
extern void (*ircReplyFuncs[ARRAY_LEN])(IrcServer *,
                                        char[ARG_LINES][ARG_MSGLEN], size_t);

extern char ircCmdNames[ARRAY_LEN][8];

typedef struct IrcTag {
  char key[16];
  char val[16];
} IrcTag;

void ircRpl(IrcServer *, char *, size_t);

void ircHandleReplyNum(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircHandleReplyCmd(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);

void ircRplTopic(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircRplPrivmsg(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircRplNotice(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircRplJoin(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircRplPart(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircRplPing(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircRplPong(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircRplSquery(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);
void ircRplQuit(IrcServer *, char[ARG_LINES][ARG_MSGLEN], size_t argc);

#endif // INCLUDE_INC_IRC_REPLIES_H_
// vim: ft=c
