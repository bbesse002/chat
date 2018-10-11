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




#define len 100


int main (int argc, char ** argv){

  char q[]="/quit";

  if (argc!=3){
    printf ("arguments");
    return 0;
  }

  const char *IP =argv[1];
  int Port = atoi(argv[2]);


  char message[len];
  char recu[len];
  char test_b[] = "0\n";

  int sock = socket( AF_INET, SOCK_STREAM, 0 );
  struct sockaddr_in sock_host;
  memset(&sock_host, '\0', sizeof (struct sockaddr_in));
  if (sock == -1){
    printf ("erreur socket");
    perror ("socket");
    return 0;
  }






  sock_host.sin_family = AF_INET;
  sock_host.sin_port = htons(Port);
  sock_host.sin_addr.s_addr = inet_addr(IP);


  int connexion = connect(sock,(struct sockaddr*)&sock_host,sizeof(sock_host));

  if (connexion == -1){
    perror("erreur de connexion\n");
    return 0;
  }




    recv(sock, test_b, 3, 0);


  char t[]="0\n";
  if (strcmp(test_b,t) == 0){
    printf("connexion refusée par le serveur\n");
    fflush(stdout);
    return 0;
  }



  while (1){

    for (int i=0; i<100; i++){
      *(message+i)='0';
      *(recu+i)='0';
    }

    printf("saisissez votre message (max 100 caractères)\n");
    fgets (message, len, stdin);

    int l = strlen(message);



    int len_sent = send ( sock, message, l, 0);


    int len_rcv= recv ( sock, recu, len, 0);

    if (strncmp(recu, q,5)==0){
      printf ("connexion terminée\n");
      return 0;
    }
    else{
      printf ("serveur :\n");
      for (int j=0; j<len_rcv; j++){
        printf ("%c", *(recu+j));
      }
      printf ("\n");
    }



  }
  return 1;
}
