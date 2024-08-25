#include "irc.h"
#include "irc_commands.h"
#include "tui.h"
#include "util.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	outfile = fopen("moksha.log", "ab");

  memset(&userTemplate, 0, sizeof(IrcUser));
  strlcpy(userTemplate.nick, getenv("USER"), LEN_NICK);
  strlcpy(userTemplate.name, getenv("USER"), LEN_NICK);
  strlcpy(userTemplate.user, getenv("USER"), LEN_NICK);
	strcpy(userTemplate.mode, "*");
  servers = (IrcServer *)malloc(sizeof(IrcServer));
  curServer = 0;
  lenServers = 0;
  _mallocServers = 1;

  tuiInit();

  tuiLoop();

  tuiDeinit();
  fclose(outfile);
  return 0;
}
