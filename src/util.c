#include "irc.h"
#include "util.h"
#include <netdb.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint isNumeric(char *c, size_t len) {
  size_t counter = 0;
  while (counter < len && c[counter] != 0) {
    if ((uint)c[counter] - 48 >= 10)
      return 0;
    counter++;
  }
  return 1;
}

struct sockaddr_in getFromHost(char *host) {
  struct hostent *ent = gethostbyname(host);

  if (NULL == ent) {
    fprintf(outfile, "Host null\n");
    exit(1);
  }

  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  memcpy(&addr.sin_addr, ent->h_addr_list[0], ent->h_length);

  addr.sin_family = AF_INET;

  return addr;
}

void strTrim(uint *left, uint *right, char *str) {
  uint l = 0, r = 0;
  while (str[l] == ' ')
    l++;
  while (str[++r] != 0)
    ;
  while (str[--r] == ' ')
    ;
  str[r + 1] = 0;

	*left = l;
	*right = r;
}
