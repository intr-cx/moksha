#ifndef INCLUDE_INC_UTIL_H_
#define INCLUDE_INC_UTIL_H_

#include <stdlib.h>
#include <netinet/ip.h>

uint isNumeric(char *c, size_t len);

struct sockaddr_in getFromHost(char *);

void strTrim(uint *left, uint *right, char *str);

#endif  // INCLUDE_INC_UTIL_H_
