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


#define local "127.0.0.1"
#define Port 6666
#define len 100



int main (int argc, char ** argv){

  int sock = socket( AF_INET, SOCK_STREAM, 0 );

  struct sockaddr_in mon_adresse;
  memset(&mon_adresse, '\0', sizeof(struct sockaddr_in));

  struct in_addr a;
  a.s_addr =inet_addr(local);



  mon_adresse.sin_family = AF_INET;
  mon_adresse.sin_port = htons(Port);
  mon_adresse.sin_addr = a;

  int e = bind (sock, (struct sockaddr*)&mon_adresse, sizeof(struct sockaddr_in));
  struct sockaddr addr;
  socklen_t addrlen;
  while (1){
    int e2= listen (sock, 1);
    int e3= accept (sock, &addr, &addrlen);
    char buf[len];


    ssize_t size =  recv(sock, &buf, len ,0);

    printf ("client :\n");
    for (int j=0; j<size; j++){
      printf ("%c", *(buf+j));
    }
    printf ("\n");

    int e4 = send (sock, &buf, len, 0);

  }





  return 0;
}
