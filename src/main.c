#include "irc.h"
#include "irc_commands.h"
#include "tui.h"
#include "util.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  outfile = fopen("moksha.log", "ab");

  char uname[LEN_NICK];
  getlogin_r(uname, LEN_NICK);
  memset(&userTemplate, 0, sizeof(IrcUser));
  strlcpy(userTemplate.nick, getenv("USER"), LEN_NICK);
  strlcpy(userTemplate.name, uname, LEN_NICK);
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
