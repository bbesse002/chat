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
#include <pthread.h>
#include <fcntl.h>


#define len 100

struct thread_arg{
  char pseudo[22];
  char pseudo_envoi[22];
  int sock;
  volatile int b;

};

void *envoi(struct thread_arg *arg){
  char message[len];
  *(arg->pseudo+strlen(arg->pseudo)-1)='\0';
  *(arg->pseudo_envoi+strlen(arg->pseudo_envoi)-1)='\0';
  while(arg->b){


    strcpy(message,"");

    printf("message de %s pour %s (max 100 caractères)\n",arg->pseudo, arg->pseudo_envoi);
    fflush(stdout);
    fgets (message, len, stdin);

    int l = strlen(message);



    int len_sent = send ( arg->sock,message, l+1, 0);
  }
  free(arg);
}





int main (int argc, char ** argv){

  char  q[]="/quit\n";
  char w[]="/who\n";
  char wi[]="/wois";
  char n[]="/nick";

  if (argc!=3){
    printf ("arguments");
    return 0;
  }

  const char *IP =argv[1];
  int Port = atoi(argv[2]);



  char recu[len];
  char pseudo[22];
  char liste[500];
  char test_b[] = "0\n";
  char pseudo_envoi[22];
  char co_part[500];
  char t[]="0";
  int q1=0;
  int flags;

  int sock = socket( AF_INET, SOCK_STREAM, 0 );
  struct sockaddr_in sock_host;
  memset(&sock_host, '\0', sizeof (struct sockaddr_in));
  if (sock == -1){
    printf ("erreur socket");
    perror ("socket");
    return 0;
  }

  struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN;





  sock_host.sin_family = AF_INET;
  sock_host.sin_port = htons(Port);
  sock_host.sin_addr.s_addr = inet_addr(IP);




  int connexion = connect(sock,(struct sockaddr*)&sock_host,sizeof(sock_host));


  if (connexion == -1){
    perror("erreur de connexion\n");
    return 0;
  }




  recv(sock, test_b, 2, 0);



  if (strcmp(test_b,t) == 0){
    printf("connexion refusee par le serveur\n");
    fflush(stdout);
    return 0;
  }




  strcpy(liste," ");
  strcpy(co_part," ");

  printf ("choisissez un pseudo (max. 20 car.):\n");
  fgets (pseudo, len, stdin);
  int l1 = strlen(pseudo);
  send ( sock, pseudo, l1+1, 0);
  recv ( sock, liste, 500, 0);
  while (strcmp(liste,"/client_non_cree")==0){
    printf ("pseudo invalide\n");
    fgets (pseudo, len, stdin);
    l1 = strlen(pseudo);
    send ( sock, pseudo, l1+1, 0);
    recv ( sock, liste, 500, 0);
  }

  while (1){

    printf ("%s\n",liste);
    fgets (pseudo_envoi, len, stdin);
    int l2 = strlen(pseudo_envoi);
    send ( sock, pseudo_envoi, l2+1, 0);
    q1= recv ( sock, co_part, 500, 0);
    while (strstr(co_part,"/communication_etablie")==NULL){
      printf ("%s\n",co_part);
      poll(fds,1,1);
      while (fds[0].revents==POLLIN){
        recv ( sock, &co_part, 500, 0);
        printf ("%s\n",co_part);
        fflush(stdout);
        poll(fds,1,1);
      }
      fflush(stdout);
      fflush(stdin);
      fgets (pseudo_envoi, len, stdin);
      fflush(stdin);
      l2 = strlen(pseudo_envoi);
      send ( sock, pseudo_envoi, l2+1, 0);
      recv ( sock, co_part, 500, 0);
    }
    printf ("%s\n",co_part);
    fflush(stdout);
      struct thread_arg *arg=malloc (sizeof(struct thread_arg));
      strcpy(arg->pseudo,pseudo);
      strcpy(arg->pseudo_envoi,pseudo_envoi);
      arg->sock=sock;
      arg->b=1;

    pthread_t th;
    pthread_create(&th, NULL, (void *)envoi,arg);

    while (1){

      strcpy(recu,"");


      int len_rcv= recv ( sock, recu, len, 0);

      if (strstr(recu, q)!=NULL){
        printf ("connexion terminée\n");
        fflush(stdout);
        void* ret;
        arg->b=0;
        pthread_exit( ret);
        return 0;
      }
      else if (strstr(recu, n)!=NULL){
        void* ret;
        arg->b=0;
        pthread_exit( ret);
        strcpy(liste," ");
        strcpy(co_part," ");
        strcpy(pseudo,(recu+6));
        break;
      }
      else if (strstr(recu,"personne de connecte pour le moment (envoyez un message pour rafaichir)\n")!=NULL){
        void* ret;
        arg->b=0;
        pthread_exit( ret);
        strcpy(liste," ");
        strcpy(co_part," ");
        break;
      }
      else if (strncmp(recu,w,4)==0){
        void* ret;
        arg->b=0;
        pthread_exit( ret);
        strcpy(liste," ");
        strcpy(co_part," ");
        break;
      }
      else{
        printf ("%s\n", recu);
      }

    }

  }
  return 1;
}
