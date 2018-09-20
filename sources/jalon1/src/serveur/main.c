#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>



#define Addr "127.0.0.1"
#define Port 6666



int main (int argc, char ** argv){
  int sock = socket( AF_INET, SOCK_STREAM, 0 );
  struct sockaddr_in* mon_adresse = malloc (sizeof (struct sockaddr_in));
  mon_adresse->sin_family = AF_INET;
  mon_adresse->sin_port = Port;
  mon_adresse->sin_addr.s_addr = atoi(Addr);
  for (int i=0; i<8; i++){
    *(i+(mon_adresse->sin_zero)) = 0;
  }
  int e = bind (sock, (struct sockaddr*)mon_adresse, sizeof(struct sockaddr_in));
  struct sockaddr addr;
  socklen_t addrlen;
  while (1){
    int e2= listen (sock, 1);
    int e3= accept (sock, &addr, &addrlen);
    char buf[25];
    ssize_t len = 100;

    ssize_t size =  recv(sock, &buf, len ,0);

    int e4 = send (sock, &buf, len, 0);

  }





  return 0;
}
