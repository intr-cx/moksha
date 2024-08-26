#ifndef INCLUDE_INC_IRC_H_
#define INCLUDE_INC_IRC_H_

#include <netinet/ip.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define LEN_MODE 8
#define LEN_NICK 64
#define LEN_MSG 512

/* most servers limit messages to 512, but you never know */
#define LEN_PROTMSG 1024
#define LEN_PROTARRAY 16

#define LEN_ARRAY 128
#define LEN_IRCCMD 8


typedef struct IrcMsg {
	char msg[LEN_MSG];
	char sep;
	char ident[LEN_NICK];
} IrcMsg;

typedef struct IrcUser {
  char nick[LEN_NICK];
  char mode[LEN_MODE];
  char user[LEN_NICK];
  char name[LEN_NICK];
  char host[LEN_NICK];
  /* struct sockaddr_in host; */
} IrcUser;

typedef struct IrcChannel {
	char name[LEN_NICK];
	char mode[LEN_MODE];
	char topic[LEN_MSG];
	IrcMsg *msgs;
	uint lenMsgs, _mallocMsgs;
	IrcUser *users;
	uint lenUsers, _mallocUsers;
	uint8_t writeable;
} IrcChannel;

typedef struct IrcServer {
  struct sockaddr_in addr;
  char host[LEN_NICK];
  char name[LEN_NICK];
  uint16_t port;
  int sockfd;
  ulong ping;
  ulong pong;
  pthread_t thrRecv, thrTimer;
  IrcUser me;
  IrcChannel *channels;
  uint lenChannels, _mallocChannels, curChannel;
} IrcServer;

extern IrcServer *servers;
extern uint lenServers, _mallocServers, curServer;
extern pthread_mutex_t mutex;
extern IrcUser userTemplate;

extern FILE *outfile;

int createUserFromStr(IrcUser *, char *, size_t );
IrcChannel *getChannelFromStr(IrcServer *, char *);
IrcUser *getUserFromStr(IrcChannel *, char *);

ulong getServerPing(IrcServer *);

void ircConnect(IrcServer *);
void ircDisconnect(IrcServer *, char *reason);

void *ircThreadRecv(void *);
void *ircThreadTimer(void *);

int initSocket(struct sockaddr_in *addr);

void ircUserCreate(IrcUser *, char *nick, char *user, char *name);
void ircServerCreate(IrcServer *, char *host, uint16_t port, IrcUser);
IrcServer *ircAddServer(IrcServer *);

void ircServerFree(IrcServer *);
void ircChannelFree(IrcChannel *);

IrcChannel *ircServerAddChannel(IrcServer *, char *name);
void ircChannelAddNick(IrcChannel *,  char *name);
void ircChannelAddUser(IrcChannel *, IrcUser *);
void ircServerRemoveChannel(IrcServer *, char *name);
void ircChannelRemoveNick(IrcChannel *,  char *name);
void ircChannelRemoveUser(IrcChannel *, IrcUser *);

void ircServerAddMsg(IrcChannel *, char *ident, char sep, char *msg);

void ircChannelSendMsg(IrcServer *, IrcChannel *, char *buf);
void ircChannelSendCmd(IrcServer *, IrcChannel *, char *buf);

void ircServerStart(IrcServer *);

#endif // INCLUDE_INC_IRC_H_
