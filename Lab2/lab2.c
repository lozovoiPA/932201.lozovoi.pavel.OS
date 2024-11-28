#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 1243
#define BUF_SIZE 64

int cur_client = -1;
volatile sig_atomic_t sigHup = 0;

void sigHupHandler(int r){
 sigHup = 1;
}

int main (){
 
 int s, s_new;
 struct sockaddr_in sin, from_sin;
 int from_len = sizeof(from_sin);
 char buf[BUF_SIZE];
 fd_set fds;

	//инициализация сокета
 s = socket(AF_INET, SOCK_STREAM, 0);
 if(s < 0) {
  printf("Error in socket init");
  exit(-1);
 }
 memset ((char*)&sin, '\0', sizeof(sin));
 sin.sin_family = AF_INET;
 sin.sin_addr.s_addr = INADDR_ANY;
 sin.sin_port = htons(PORT);
 if(bind(s, (struct sockaddr *)&sin, sizeof(sin))){
  printf("Error in socket binding");
  exit(-1);
 }

 	//регистрация обработчика сигнала
 struct sigaction sa;
 sigaction(SIGHUP, NULL, &sa);
 sa.sa_handler = sigHupHandler;
 sa.sa_flags |= SA_RESTART;
 if(sigaction(SIGHUP, &sa, NULL)){
  printf("Error in sigaction init");
  exit(-1);
 }
 

 	//блокировка сигнала
 sigset_t blockedMask, origMask;
 sigemptyset(&blockedMask);
 sigaddset(&blockedMask, SIGHUP);
 sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

 	//начинаем прослушивать соединения
 if(listen(s,1)){
  printf("Error when trying to listen on the socket");
  exit(-1);
 }
 printf("I am listening on port %d!\n", sin.sin_port);
 
 int max_fd = -1, res;
 while(1) {
	//получили сигнал sigHup - завершаем работу сервера
  if(sigHup){
   if(cur_client != -1){
    shutdown(cur_client, 0);
    close(cur_client);
   }
   break;
  }
	//Обнуляем множество файловых дескрипторов. В этих дескрипторах - ожидаем некоторую активность. 
  	//Здесь дескрипторы - дескрипторы сокетов.
  FD_ZERO(&fds);
  FD_SET(s, &fds);
  if(cur_client != -1){
    FD_SET(cur_client, &fds);
  }
  max_fd = (cur_client > s) ? cur_client : s;
  res = pselect(max_fd+1, &fds, NULL, NULL, NULL, &origMask);
  if(res == -1){
   	//произошло прерывание сигналом
   if(errno == EINTR){
    printf("Принят сигнал sigHup!\n");
    continue;
   }
   else{
    printf("Error in pselect()\n");
    exit(-1);
   }
  }
  else{
   	//pselect() завершился успешно, необходимо проверить, остались ли дескрипторы в сете.
	//Если да, то гарантированно не заблокированы операции accept() и read().
   if(FD_ISSET(s, &fds)){
    s_new = accept(s, &from_sin, &from_len);
    printf("Server got new connection %d\n", s_new);
    if(cur_client == -1){
     cur_client = s_new;
    }
    else{
     printf("Rejecting connection..\n");
     shutdown(s_new,0);
     close(s_new);
    }
   }

   if(cur_client != -1 && FD_ISSET(cur_client, &fds)){
    read(cur_client, buf, BUF_SIZE);
    printf("Client said: %s\n", buf);
   }
  }
 }
 close(s);
 return 0;
}
