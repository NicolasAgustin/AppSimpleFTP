#ifndef _SRV_HEADER_
#define _SRV_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <signal.h>
#include <netinet/in.h>

#define BUFSIZE 512
#define CMDSIZE 5
#define PARSIZE 100

#define MSG_220 "220 srvFtp version 1.0\r\n"
#define MSG_331 "331 Password required for %s\r\n"
#define MSG_230 "230 User %s logged in\r\n"
#define MSG_530 "530 Login incorrect\r\n"
#define MSG_221 "221 Goodbye\r\n"
#define MSG_550 "550 %s: no such file or directory\r\n"
#define MSG_299 "299 File %s size %ld bytes\r\n"
#define MSG_226 "226 Transfer complete\r\n"

bool recv_cmd(int sd, char *operation, char *param);
bool send_ans(int sd, char *message, ...);
void retr(int sd, char *file_path);
bool check_credentials(char *user, char *pass);
bool authenticate(int sd);
void operate(int sd);
void handler(int signum);

#endif // _SRV_HEADER_