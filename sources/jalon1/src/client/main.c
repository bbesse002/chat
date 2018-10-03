#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>



#define Addr "127.0.0.1"
#define Port "6666"
#define len 100

int main (int argc, char ** argv){

  char message[len];
  char recu[len];


  int sock = socket( AF_INET, SOCK_STREAM, 0 );
  struct sockaddr_in* sock_host = malloc (sizeof (struct sockaddr_in));
  memset(sock_host, '\0', sizeof (struct sockaddr_in));
  if (sock == -1){
    printf ("erreur socket");
    perror ("socket");
  }

  sock_host->sin_family = AF_INET;
  sock_host->sin_port = htons(atoi(Port));
  sock_host->sin_addr.s_addr = htonl(atoi(Addr));


  int connexion = connect(sock,(struct sockaddr*)sock_host, sizeof(*sock_host));
  if (connexion == -1){
    printf ("erreur de connexion\n");
    perror("co");
  }

  while(1){

    for (int i=0; i<100; i++){
      *(message+i)='0';
      *(recu+i)='0';
    }

    printf("saisissez votre message (max 100 caractères)\n");
    fgets (message, len, stdin);




    int len_sent = send ( sock, message, len, 0);

    int len_rcv= recv ( sock, recu, len, 0);

    printf ("serveur :\n");
    for (int j=0; j<len_rcv; j++){
      printf ("%c", *(recu+j));
    }
    printf ("\n");



  }
  return 1;
}
