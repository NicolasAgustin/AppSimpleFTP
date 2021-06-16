#ifndef _CLIENT_HEADER_

#define _CLIENT_HEADER_

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <err.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 512

bool recv_msg(int sd, int code, char *text);
void send_msg(int sd, char *operation, char *param);
char * read_input();
void authenticate(int sd);
void get(int sd, char *file_name);
void quit(int sd);
void operate(int sd);

#endif // _CLIENT_HEADER_