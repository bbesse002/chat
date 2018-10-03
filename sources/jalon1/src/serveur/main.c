#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>



#define Addr "127.0.0.1"
#define Port 6666
#define len 100



int main (int argc, char ** argv){

  int sock = socket( AF_INET, SOCK_STREAM, 0 );

  struct sockaddr_in* mon_adresse = malloc (sizeof (struct sockaddr_in));
  memset(mon_adresse, '\0', sizeof(struct sockaddr_in));
  mon_adresse->sin_family = AF_INET;
  mon_adresse->sin_port = htons(Port);
  mon_adresse->sin_addr.s_addr = htonl(atoi(Addr));

  int e = bind (sock, (struct sockaddr*)mon_adresse, sizeof(struct sockaddr_in));
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
