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

#include "txcproxy.h"

extern char* fname;

void master(OPTIONS* op) {
   char cmd[MAX_PATH];
   int  len;

   SOCKET s1, s2;
   SOCKADDR_IN sin;

   SECURITY_ATTRIBUTES sa;
   STARTUPINFO         si;
   PROCESS_INFORMATION pi;
   WSAPROTOCOL_INFO    wi;
  
   HANDLE pin, pout; 

   SetConsoleCtrlHandler(mhandler, TRUE);

   sin.sin_family      = AF_INET;
   sin.sin_addr.s_addr = op->addr;
   sin.sin_port        = op->port;

   if ((s1 = socket(AF_INET, SOCK_STREAM, 0))  == INVALID_SOCKET || 
       bind(s1, (PSOCKADDR) &sin, sizeof(sin)) == SOCKET_ERROR   || 
       listen(s1, SOMAXCONN)                   == SOCKET_ERROR) {
      printf("Socket error %d", WSAGetLastError());
      exit(EXIT_FAILURE);
   } 

   sa.nLength              = sizeof(sa); 
   sa.lpSecurityDescriptor = NULL;
   sa.bInheritHandle       = TRUE;

   CreatePipe(&pin, &pout, &sa, 0);

   len = sizeof(sin);

   memset(&si, 0, sizeof(si)); 
   si.cb        = sizeof(si); 
   si.hStdInput = pin;
   si.dwFlags   = STARTF_USESTDHANDLES;

   sprintf(cmd, "%s fork()", fname);

   while (TRUE) {
      printt(); printf(" READY\n");

      if ((s2 = accept(s1, (PSOCKADDR) &sin, &len)) == INVALID_SOCKET) continue; 

      printt(); printf(" <- %s:%d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

      CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

      if (WSADuplicateSocket(s2, pi.dwProcessId, &wi) == SOCKET_ERROR) {
         printf("Socket error %d", WSAGetLastError());
         exit(EXIT_FAILURE);
      } 

      WriteFile(pout, &wi, sizeof(wi), &len, NULL);
      WriteFile(pout, op, sizeof(*op), &len, NULL); 

      closesocket(s2);
   }
}

void printt() {
   time_t     tm  = time(NULL);
   struct tm* ltm = localtime(&tm);

   printf("%02d.%02d.%d %02d:%02d:%02d",    
          ltm->tm_mday, ltm->tm_mon + 1, (ltm->tm_year + 1900), 
          ltm->tm_hour, ltm->tm_min,  ltm->tm_sec);
}

bool WINAPI mhandler(u_long event) {
   char*  name; 
   HANDLE snapshot, child;
   PROCESSENTRY32 pe = {sizeof(pe)};
#ifdef SPLIT 
   name = fname;
#else
   name = strrchr(fname, '\\') + 1;
#endif
   snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
   Process32First(snapshot, &pe); // [System Process]

   while ((pe.dwSize = sizeof(pe)) && Process32Next(snapshot, &pe)) 
      if (!strcmp(pe.szExeFile, name) && GetCurrentProcessId() != pe.th32ProcessID) 
         GenerateConsoleCtrlEvent(event, pe.th32ProcessID);  // фурычит тока при dwCreationFlags == 0

   printt(); printf(" EXIT");
   exit(EXIT_SUCCESS); 
}


