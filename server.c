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

SOCKET s1, s3;
HANDLE lib = 0;
_txcfree txcfree;

#ifdef SPLIT
   char* fname;

int main(int argc, char** argv) {
   WSADATA wd; 
   WSAStartup(MAKEWORD(2, 2), &wd);

   fname = argv[0];
   *(strrchr(fname, '\\') + 1) = '\0';
   server();
}
#else
   extern char* fname;
#endif

void server(void) {
   char    * cmd, * arg, * tmp, * msg, * cst = NULL;
   char    buf[MAX_BLEN], 
           user[MAX_ALEN * 4] = {'\0'}, // UTF8 MAX 4 bytes
           pass[MAX_ALEN * 4] = {'\0'};
   wchar_t wdir[MAX_ALEN];
   int     len, port;

   SOCKET      s2;
   SOCKADDR_IN sin;

   HANDLE pin, lock = 0;
   
   _txcsend   txcsend;
   
   OPTIONS          op;
   WSAPROTOCOL_INFO wi;

   SetConsoleCtrlHandler(shandler, TRUE);
 
   pin = GetStdHandle(STD_INPUT_HANDLE);
 
   ReadFile(pin, &wi, sizeof(wi), &len, NULL); 
   ReadFile(pin, &op, sizeof(op), &len, NULL); 

   if ((s1 = WSASocket(AF_INET, SOCK_STREAM, 0, &wi, 0, 0)) == INVALID_SOCKET) exit(EXIT_FAILURE);
   
   if (!(lib = LoadLibrary(op.dll))) shandler(MSG500); // Невозможно загрузить конектор

   if (((_txcinit)GetProcAddress(lib, "Initialize"))(".", 1)) shandler(MSG500); 
   
   ((_txcset)GetProcAddress(lib, "SetCallback"))(acceptor);
	txcsend = (_txcsend)GetProcAddress(lib, "SendCommand");
	txcfree = (_txcfree)GetProcAddress(lib, "FreeMemory");
 
   sendmsg(s1, MSG100); // Приветствие

   while (TRUE) {
      len = 0;
 
      while (TRUE) {  
         if (len == MAX_BLEN) shandler(MSG410);  // Слишком большое сообщение

         if ((len += recv(s1, buf + len, MAX_BLEN - len, 0)) == SOCKET_ERROR) exit(EXIT_FAILURE); // Ошибка сокета  

         if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n') {
            buf[len - 2] = '\0';
            break;
         }
      }

      if ((cmd = skip(buf)) == NULL) continue;
 
      tmp = cmd;
      while ((*tmp = toupper(*tmp)) && !isblank(*tmp)) tmp++; 
      *tmp = '\0';

      if (!strcmp("OPEN", cmd)) {
         id = OPEN;
      } else if (!strcmp("CLOS", cmd)) {
         id = CLOS; 
      } else if (!strcmp("DATA", cmd)) {
         id = DATA; 
      } else if (!strcmp("STAT", cmd)) {
         id = STAT;
      } else if (!strcmp("QUIT", cmd)) {
         id = QUIT;
      } else if (!strcmp("USER", cmd)) {
         id = USER;
      } else if (!strcmp("PASS", cmd)) {
         id = PASS;
      } else if (!strcmp("SEND", cmd)) {
         id = SEND;
      } else { 
         sendmsg(s1, MSG400); // Неизвестная команда
         continue;
      }

      msg = MSG200;

      if (id == OPEN && (lock || !strlen(user) || !strlen(pass))) {   // CLOS!
         msg = lock ? MSG520 : MSG530;
      } else if ((id == CLOS || id == SEND) && !lock) {               // OPEN!
         msg = MSG521;
      } else if (id >= USER && (tmp == (buf + len - 2) || (arg = skip(tmp + 1)) == NULL)) { // Отсутствуют аргументы 
         msg = MSG401;       
      } else if (id == OPEN) {
         SetCurrentDirectory(fname);   // Текущий каталог программы, (ANSI)
         CreateDirectoryW(wdir, NULL); // ERROR_ALREADY_EXISTS
         SetCurrentDirectoryW(wdir);   // Рабочий каталог пользователя

         if ((lock = CreateFile("lock", GENERIC_READ, 0, NULL, CREATE_ALWAYS, 0, NULL)) == INVALID_HANDLE_VALUE) { 
            if (GetLastError() != ERROR_SHARING_VIOLATION) shandler(MSG510);

            lock = 0; 
            msg  = MSG531;
         } else { 
            arg = buf; 
            sprintf(arg, "<command id=\"connect\">"
                         "<login>%s</login>"
                         "<password>%s</password>"
                         "%s</command>", user, pass, op.str);  
            goto _SEND;
         }
      } else if (id == QUIT || id == CLOS) {
         CloseHandle(lock);
         lock = 0;
         arg  = buf;
         sprintf(arg, "<command id=\"disconnect\"/>");
         goto _SEND;
      } else if (id == DATA) {
         closesocket(s3);
         port = PORT;

         sin.sin_family      = AF_INET;
         sin.sin_addr.s_addr = op.addr;

         s2 = socket(AF_INET, SOCK_STREAM, 0);  

         while (TRUE) { // Возможен выход за пределы портов 
            sin.sin_port = htons(++port); 

            if (!bind(s2, (PSOCKADDR) &sin, sizeof(sin))) {
               break;
            } else if (WSAGetLastError() != WSAEADDRINUSE) { 
               shandler(MSG511);               
            }
         };

         sprintf(buf, MSG110, ntohs(sin.sin_port));
         sendmsg(s1, buf);
  
         if (listen(s2, SOMAXCONN) == SOCKET_ERROR || (s3 = accept(s2, NULL, NULL)) == INVALID_SOCKET)  
            shandler(MSG511);

         closesocket(s2);
      } else if (id == STAT) {
         arg = buf;
         sprintf(arg, "<command id=\"server_status\"/>");
         goto _SEND;
      } else if (id == USER || id == PASS) {
         strlen(arg) > (MAX_ALEN * 4 - 1) ? msg = MSG410 : strcpy(id == USER ? user : pass, arg);

         if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, user, -1, wdir, MAX_ALEN)) shandler(MSG411);
      } else { // SEND
         tmp = arg;
         while ((*tmp = tolower(*tmp)) && *tmp != '>') tmp++; 

         if (strstr(arg, "connect") || strstr(arg, "disconnect")) {
            msg = MSG540; // Команда перехвачена
         } else { 
         _SEND:
         #ifdef FDUMP
            fdump("send.txt", arg, strlen(arg));
         #endif
            txcfree(cst);
            cst = txcsend(arg);

            if (id == QUIT) shandler(0);

            if (id != CLOS && strcmp(cst, "<result success=\"true\"/>")) {
               tmp = cst + strlen("<result success=\"false\"><message>"); 
               *(strchr(tmp, '<') - 1) = '\0'; // До точки
               sprintf(buf, MSG541, tmp);
               msg = buf;
             }
         }
      } 
      sendmsg(s1, msg);
    } // while
}

BOOL WINAPI shandler(DWORD event) {
   char* msg;
   msg = event ? (char*) event : MSG101;
   sendmsg(s1, msg); 
   
   if (lib) ((_txcuninit)GetProcAddress(lib, "UnInitialize"))();

   exit(EXIT_SUCCESS); 
}

bool CALLBACK acceptor(BYTE* data) {
#ifdef FDUMP
	fdump("data.txt", data, strlen(data));
#endif
	sendmsg(s3, data);
	sendmsg(s3, "\r\n");
	return txcfree(data);
}

char* skip(char* ptr) {
   while (isblank(*ptr)) ptr++; // \0 !!!
   return *ptr ? ptr : NULL;
}

#ifdef FDUMP
void fdump(char* fname, char* buf, int len) {
   FILE* fh;
   fh = fopen(fname, "ab");
   fwrite(buf, 1, len, fh);
   fclose(fh);
}
#endif

