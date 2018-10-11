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


struct client{
  int i;
  char pseudo[21];
  struct client* com;
  struct client* next;

}

#define len 100

struct client* trouver_client(struct client* client0, char* pseudo){
  struct client* client_actuel = client0;
  while (1){
    if (client_actuel == NULL){
      return NULL;
    }
    else if (strcmp(pseudo,client_actuel->pseudo) == 0){
      return client_actuel;
    }
    else {
      client_actuel = client_actuel->next;
    }
  }
}


int creer_client (struct client* client0, int i, char* pseudo){
  if (trouver_client(client0, pseudo) != NULL){
    return -1;
  }
  else{
    struct client* client_actuel = client0;
    while (client_actuel->next != NULL){
      client_actuel = client_actuel->next;
    }
    client_actuel->next = malloc(sizeof(struct client));
    client_actuel=client_actuel->next;
    client_actuel->next =NULL;
    client_actuel->i=i;
    client_actuel->com = NULL;
    client_actuel->pseudo = pseudo;
    return 1;
  }
}




int main (int argc, char ** argv){
  struct client* client0=malloc(sizeof(struct client));

  client0->next=NULL;
  client0->pseudo="server";
  client0->i=-1;
  client0->com = NULL;


  int co_max =20;

  char  q[7]="/quit\n";

  if (argc!=3){
    printf ("arguments");
    return 0;
  }

  const char *IP =argv[1];
  int Port = atoi(argv[2]);

  int sock = socket( AF_INET, SOCK_STREAM, 0 );

  struct sockaddr_in mon_adresse;
  memset(&mon_adresse, '\0', sizeof(struct sockaddr_in));





  mon_adresse.sin_family = AF_INET;
  mon_adresse.sin_port = htons(Port);
  mon_adresse.sin_addr.s_addr = inet_addr(IP);

  int e = bind (sock, (struct sockaddr*)&mon_adresse, sizeof(struct sockaddr_in));
  struct sockaddr addr;
  socklen_t addrlen;

  int e2= listen (sock, 1);
  struct pollfd fds[co_max+1];
  for (int j=0; j<co_max+1;j++){
    fds[j].fd = sock;
    fds[j].events = POLLIN;
  }

  int k=0;

  while (1){

    poll(fds,co_max+1,-1);

    for (int i =0 ; i<co_max+1;i++){

      if (fds[i].revents == POLLIN){
        if (fds[i].fd==sock){

          if (k<co_max){
            fds[i].fd=accept(sock, &addr, &addrlen);
            int ls = send(fds[i].fd, "1\n",3,0);
            fds[i].events=POLLIN;
            k++;
            break;
          }
          else{
            int temp = accept(sock, &addr, &addrlen);
            int ls = send(temp, "0\n", 3, 0);
            close(temp);
            break;
          }
        }



        else{

          char buf[len];

          ssize_t size =  recv(fds[i].fd, &buf, len ,0);

          printf ("client :\n");
          for (int j=0; j<size; j++){
            printf ("%c", *(buf+j));
          }
          printf ("\n");
          int e4 = send (fds[i].fd, &buf, size, 0);

          if (strncmp(buf,q,7)==0){
            printf ("connexion avec un client fermée\n");
            fflush (stdout);
            close (fds[i].fd);
            k--;
            fds[i].fd = sock;
            fds[i].events = POLLIN;
            break;
          }


        }
      }
    }



}




  return 0;
}
