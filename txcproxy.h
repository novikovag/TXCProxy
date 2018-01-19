/*===========================================================================
    TXCProxy, TRANSAQ XML Connector Proxy server 
    Copyright (C) 2010, 2018 Novikov Artem Gennadievich

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
===========================================================================*/

#ifndef _TXCProxy_
#define _TXCProxy_

#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#include <winsock2.h>
#include <winbase.h>
#include <tlhelp32.h>

#define VERSION "v1.1" 

#define MSG100 "100 TXCProxy " VERSION "\r\n"
#define MSG101 "101 BYE\r\n"
#define MSG110 "110 DP #%d ready, waiting...\r\n"

#define MSG200 "200 OK\r\n"

#define MSG400 "400 Command unrecognized\r\n"
#define MSG401 "401 Bad arguments\r\n"
#define MSG410 "\r\n410 Message too large\r\n"
#define MSG411 "411 Encoding error\r\n"

#define MSG500 "500 Can't load connector\r\n"
#define MSG510 "510 I/O error\r\n"
#define MSG511 "511 DP init error\r\n"
#define MSG520 "520 CLOS at first\r\n"
#define MSG521 "521 OPEN at first\r\n"
#define MSG530 "530 Undefined username or password\r\n"
#define MSG531 "531 Username already in use\r\n"
#define MSG540 "540 ID not allowed\r\n" 
#define MSG541 "541 %s\r\n"

#define sendmsg(s, msg) send(s, msg, strlen(msg), 0) 

#define USAGE \
"Usage: TXCProxy -tq_addr -tq_port [-px_addr -px_port [options]] \n"\
"-addr     local IP adress           \n"\
"-port     local port                \n"\
"-tq_addr  TRANSAQ IP adress         \n"\
"-tq_port  TRANSAQ port              \n"\
"-px_addr  proxy adress              \n"\
"-px_port  proxy port                \n"\
"-px_type  proxy type (S4, S5, HTTP) \n"\
"-px_user  proxy user                \n"\
"-px_pass  proxy password            \n"\
"-dll      connector dll             \n"\
"-ll2      connector log level       \n"

#define MAX_ALEN  20 // символов имя/пароль 
#define MAX_SLEN  165 + MAX_ALEN * 2
#define MAX_BLEN  1024

#define PORT 4444

enum {OPEN, CLOS, DATA, STAT, QUIT, USER, PASS, SEND} id;

struct options {
   u_long  addr;
   u_short port;
   char dll[MAX_PATH];
   char str[MAX_SLEN];
};

typedef struct options OPTIONS;

void getopt(int, char**, OPTIONS*);

void master(OPTIONS*);
void printt(void);
bool WINAPI mhandler(u_long);

void server(void);
void getcmd(void);
char* skip(char*);
bool WINAPI shandler(u_long);

void fdump(char*, char*, int); 

// typedef bool (*tcallback)(BYTE* pData);
typedef bool (WINAPI* callback)(char*);
// bool SetCallback(tcallback pCallback)
typedef bool (WINAPI* _txcset)(callback);
// BYTE* SendCommand(BYTE* pData);
typedef char* (WINAPI* _txcsend)(char*); 
// bool FreeMemory(BYTE* pData);
typedef bool (WINAPI* _txcfree)(char*);
// BYTE* Initialize(const BYTE* logPath, int logLevel);
typedef char* (WINAPI* _txcinit)(char*, int); 
// BYTE* UnInitialize();
typedef char* (WINAPI* _txcuninit)(); 

bool CALLBACK acceptor(BYTE*); 

#endif
