#ifndef INCLUDE_INC_IRC_REPLIES_H_
#define INCLUDE_INC_IRC_REPLIES_H_

#include "irc.h"

// array of function pointers corresponding to received commands
extern void (*ircReplyFuncs[LEN_ARRAY])(IrcServer *,
                                        char[LEN_PROTARRAY][LEN_PROTMSG], size_t);

extern char ircCmdNames[LEN_ARRAY][8];

typedef struct IrcTag {
  char key[16];
  char val[16];
} IrcTag;

void ircRpl(IrcServer *, char *, size_t);

void ircHandleReplyNum(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircHandleReplyCmd(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);

void ircRplTopic(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircRplPrivmsg(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircRplNotice(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircRplJoin(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircRplPart(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircRplPing(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircRplPong(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircRplSquery(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);
void ircRplQuit(IrcServer *, char[LEN_PROTARRAY][LEN_PROTMSG], size_t argc);

#endif // INCLUDE_INC_IRC_REPLIES_H_
// vim: ft=c
