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

#include "TXCProxy.h"
#pragma comment(lib, "ws2_32.lib")

char* fname;

int main(int argc, char** argv) {
   WSADATA wd; 
   WSAStartup(MAKEWORD(2, 2), &wd);

#ifdef SPLIT
   fname = "server.exe";
#else
   fname = argv[0];
#endif

   if (argc == 2 && !strcmp(argv[1], "fork()")) {
#ifndef SPLIT
      *(strrchr(fname, '\\') + 1) = '\0';
      server();
#endif
   } else {
      OPTIONS op;
      getopt(argc, argv, &op);
      master(&op);
   }
}

void getopt(int argc, char** argv, OPTIONS* op) {
   char* px_type = "HTTP-CONNECT",
       * px_addr = NULL,
       * px_port = NULL,
       * px_user = NULL,
       * px_pass = NULL,
       * tq_addr = NULL,
       * tq_port = NULL,
       * dll     = "txmlconnector.dll", 
       * ll2     = "2",
       * tmp     = op->str; 
   int i = 0;

   op->addr = INADDR_ANY;  
   op->port = htons(PORT);

   while (++i < argc - 1) {
      if (!strcmp(argv[i], "-addr")) {
         op->addr = inet_addr(argv[++i]);
      } else if (!strcmp(argv[i], "-port")) {
         op->port = htons(atoi(argv[++i]));
      } else if (!strcmp(argv[i], "-px_type")) {
         i++;
         if (!strcmp(argv[i], "S4")) 
            px_type = "SOCKS4";
         else if (!strcmp(argv[i], "S5")) 
            px_type = "SOCKS5";
      } else if ((!strcmp(argv[i], "-px_addr") && strlen(px_addr = argv[++i]) > 15) ||
                 (!strcmp(argv[i], "-tq_addr") && strlen(tq_addr = argv[++i]) > 15) ||
                 (!strcmp(argv[i], "-px_port") && strlen(px_port = argv[++i]) > 5)  ||
                 (!strcmp(argv[i], "-tq_port") && strlen(tq_port = argv[++i]) > 5)  ||
                 (!strcmp(argv[i], "-ll2")     && strlen(ll2     = argv[++i]) > 1)  ||
                 (!strcmp(argv[i], "-px_user") && strlen(px_user = argv[++i]) > MAX_ALEN - 1) ||
                 (!strcmp(argv[i], "-px_pass") && strlen(px_pass = argv[++i]) > MAX_ALEN - 1) ||
                 (!strcmp(argv[i], "-dll")     && strlen(dll     = argv[++i]) > MAX_PATH - 1)) { 
        goto _USAGE;
      } 
   }

   if (!tq_addr || !tq_port || (px_addr && !px_port)) {
_USAGE: 
      printf("%s", USAGE);
      exit(EXIT_FAILURE);
   }

   sprintf(op->dll, "%s", dll);

   tmp += sprintf(tmp, "<host>%s</host>"
                       "<port>%s</port>" 
                       "<logsdir>.\\logs\\</logsdir>"
                       "<loglevel>%s</loglevel>",
                       tq_addr, tq_port, ll2);
   if (px_addr) {
      tmp += sprintf(tmp, "<proxy type=%s addr=%s port=%s",
                          px_type, px_addr, px_port);
      if (px_pass) 
         tmp += sprintf(tmp, " login=%s password=%s",
                        px_user, px_pass);
      sprintf(tmp, "/>");
   }   
}
