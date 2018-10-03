#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>


#define local "127.0.0.1"
#define Port 6669
#define len 100



int main (int argc, char ** argv){

  int sock = socket( AF_INET, SOCK_STREAM, 0 );

  struct sockaddr_in mon_adresse;
  memset(&mon_adresse, '\0', sizeof(struct sockaddr_in));

  const char *IP= local;




  mon_adresse.sin_family = AF_INET;
  mon_adresse.sin_port = htons(Port);
  mon_adresse.sin_addr.s_addr = inet_addr(IP);

  int e = bind (sock, (struct sockaddr*)&mon_adresse, sizeof(struct sockaddr_in));
  struct sockaddr addr;
  socklen_t addrlen;

  while (1){

    int e2= listen (sock, 1);

    int newsock= accept (sock, &addr, &addrlen);


    while(1){
      char buf[len];






      ssize_t size =  recv(newsock, &buf, len ,0);


      printf ("client :\n");
      for (int j=0; j<size; j++){
        printf ("%c", *(buf+j));
      }
      printf ("\n");

      int e4 = send (newsock, &buf, size, 0);

  }
}




  return 0;
}
