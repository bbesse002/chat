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
#include <fcntl.h>

#define len 100

struct client{
  int i;
  char pseudo[22];
  struct client* com;
  struct client* next;

};



struct client* trouver_client(struct client* client0, char* pseudo){
  if (strlen(pseudo)<1){

  }
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

struct client* trouver_client_indice(struct client* client0, int i){
  struct client* client_actuel = client0;
  while (1){
    if (client_actuel == NULL){
      return NULL;
    }
    else if (client_actuel->i == i){
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
  struct client* a =trouver_client_indice(client0,i);
  if (a!=NULL){
    strcpy(a->pseudo,pseudo);
    a->com=NULL;
    return 1;
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
    strcpy(client_actuel->pseudo,pseudo);
    return 1;
  }
}

int supprimer_client (struct client* client0, int i){
  struct client *client_actuel =client0;
  if (client_actuel->next==NULL){
    return -1;
  }
  while (client_actuel->next->i!=i && client_actuel->next->next!=NULL){
    client_actuel=client_actuel->next;
  }
  if (client_actuel->next->i == i){
    struct client* ptr = client_actuel->next;
    client_actuel->next=client_actuel->next->next;
    client_actuel=client0;
    while (client_actuel != NULL){
      if (client_actuel->com == ptr) client_actuel->com =NULL;
      client_actuel=client_actuel->next;
    }
    free(ptr);
    return 1;
  }
  else return 0;
}

void liste_utilisateurs(struct client* client0,int i, char tmp[500]){
  struct client * client_actuel= client0->next;
  strcpy(tmp, "choisissez un contact :\n");
  if (client_actuel->next==NULL){
    strcpy(tmp,"personne de connecte pour le moment (envoyez un message pour rafaichir)\n");
    exit;
  }
  while (client_actuel != NULL){
    if (client_actuel->i !=i){
      strcat(tmp,client_actuel->pseudo);
    }
    client_actuel=client_actuel->next;
  }
}


int main (int argc, char ** argv){
  struct client* client0=malloc(sizeof(struct client));

  client0->next=NULL;
  strcpy(client0->pseudo,"server");
  client0->i=-1;
  client0->com = NULL;


  int co_max =20;

  char  q[]="/quit\n";
  char w[]="/who\n";
  char wi[]="/wois\n";
  char n[]="/nick";


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
            int ls = send(fds[i].fd, "1",2,0);
            fds[i].events=POLLIN;
            k++;
            break;
          }
          else{
            int temp = accept(sock, &addr, &addrlen);
            int ls = send(temp, "0", 2, 0);
            close(temp);
            break;
          }
        }



        else{

          char buf[len];

          ssize_t size =  recv(fds[i].fd, buf, len ,0);

          printf ("%d :\n",fds[i].fd);
          for (int j=0; j<size; j++){
            printf ("%c", *(buf+j));
          }
          printf ("\n");
          fflush(stdout);
          if (strcmp(buf,q)==0){
            int e4 = send (fds[i].fd, buf, size, 0);
            printf ("connexion avec un client fermée\n");
            fflush (stdout);
            supprimer_client(client0,i);
            close (fds[i].fd);
            k--;
            fds[i].fd = sock;
            fds[i].events = POLLIN;
            break;
          }
          else if (strncmp(buf,n,5)==0){
            if (creer_client(client0,i,buf+6)==1){
              char tmp[500];
              liste_utilisateurs(client0,i,tmp);
              send (fds[i].fd, tmp, 500, 0);
            }
            else{
              send (fds[i].fd, "entrez un pseudo valable", 17, 0);
            }
          }
          else if (strcmp(buf,w)==0){
            struct client* az = trouver_client_indice(client0,i);
            az->com=NULL;
            send (fds[i].fd, buf, 500, 0);
          }
          else if (strcmp(buf,wi)==0){

          }
          else{
            if (trouver_client_indice(client0,i)==NULL){
              if (creer_client(client0,i,buf)==1){
                char tmp [500];
                liste_utilisateurs(client0,i,tmp);
                send (fds[i].fd, tmp, 500, 0);
              }
              else{
                send (fds[i].fd, "/client_non_cree", 17, 0);
              }
              break;
            }
            else {
              struct client* ptr = trouver_client_indice(client0,i);
              if (ptr->com ==NULL){
                struct client* correspondant = trouver_client(client0,buf);
                if (correspondant !=NULL){
                  ptr->com=correspondant;
                  send (fds[i].fd, "/communication_etablie", 23, 0);
                }
                else{
                  char tmp [500];
                  liste_utilisateurs(client0,i,tmp);
                  send (fds[i].fd, tmp, 500, 0);
                }
              }
              else{
                char msg[size+22];
                strcpy(msg,ptr->pseudo);
                msg[strlen(ptr->pseudo)]=' ';
                msg[strlen(ptr->pseudo)-1]='\0';
                strcat(msg," :");
                strcat(msg,buf);
                send (fds[ptr->com->i].fd, &msg, size+22, 0);

              }
              break;
            }

          }

        }
      }
    }



}




  return 0;
}
