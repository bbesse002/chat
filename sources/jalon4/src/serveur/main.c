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
#include <time.h>

#define len 1000
#define nb_co_max 20

struct client{
  int i;
  char pseudo[22];
  struct client* com;
  struct client* next;
  struct tm instant;
  int count;

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
  if ((trouver_client(client0, pseudo) != NULL) || (strcmp(pseudo,"all\n")==0) || (strcmp(pseudo,"\n")==0)){
    return -1;
  }
  int k=0;
  while (*(pseudo+k)){
    if (*(pseudo+k)==' '){
      return -1;
    }
    k++;
  }
  struct client* a =trouver_client_indice(client0,i);
  if (a!=NULL && i!=-1){
    strcpy(a->pseudo,pseudo);
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
    time_t secondes;
    time(&secondes);
    client_actuel->instant=*gmtime(&secondes);
    if (strncmp(pseudo,"(salon)",7)==0){
      client_actuel->count=1;
    }
    else{
      client_actuel->count=0;
    }
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

int liste_utilisateurs(struct client* client0,int i, char tmp[len],int mode){
  struct client * client_actuel= client0->next;
  if (client_actuel ==NULL){
    return 0;
  }
  if (mode==0){
    strcpy(tmp, "choisissez un contact :\n\nall pour broadcast\n\n");
    if (client_actuel->next==NULL){
      strcpy(tmp,"personne de connecte pour le moment (envoyez un message pour rafaichir)\n\nall pour broadcast\n\n");
      return 2;
    }
  }
  else{
    strcpy(tmp, "/who \nall pour broadcast\n\n");
    if (client_actuel->next==NULL){
      strcpy(tmp,"/who personne de connecte pour le moment (envoyez un message pour rafaichir)\n\nall pour broadcast\n\n");
      return 2;
    }
  }
  while (client_actuel != NULL){
    if (client_actuel->i !=i){
      strcat(tmp,client_actuel->pseudo);
    }
    client_actuel=client_actuel->next;
  }
  return 1;
}

void trouver_client_com(struct client* client0,int i,struct client* tab[nb_co_max]){
  int j=0;
  struct client* client_concerne = trouver_client_indice(client0,i);
  struct client* client_actuel = client0->next;
  while(client_actuel!=NULL){
    if (client_actuel->com != NULL && client_actuel->com->i==i){
      tab[j]=client_actuel;
      j++;
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


  int co_max =nb_co_max;

  char  q[]="/quit\n";
  char w[]="/who\n";
  char wi[]="/whois";
  char n[]="/nick";
  char c[]="/connect";


  if (argc!=3){
    printf ("arguments");
    return 0;
  }

  const char *IP =argv[2];
  int Port = atoi(argv[1]);

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


          if (size==0){
            struct client** av = malloc (20*sizeof(struct client));
            close (fds[i].fd);
            k--;
            trouver_client_com(client0,i,av);
            supprimer_client(client0,i);
            int j=0;
            char host_lost[len];
            strcpy(host_lost,"/hostlost ");
            char tmp[len];
            liste_utilisateurs(client0,i,tmp,1);
            printf("connexion avec un client perdue\n");
            strcat(host_lost,tmp);
            while (av[j]!=NULL){
              av[j]->com=NULL;
              send(fds[av[j]->i].fd, host_lost,len,0);
              j++;
            }
            fds[i].fd = sock;
            fds[i].events = POLLIN;
            break;
          }


          printf ("%d :\n",fds[i].fd);
          for (int j=0; j<size; j++){
            printf ("%c", *(buf+j));
          }
          printf ("\n");
          fflush(stdout);
          if (strcmp(buf,q)==0){
            struct client** av = malloc (20*sizeof(struct client));
            int e4 = send (fds[i].fd, buf, size, 0);
            printf ("connexion avec un client fermée\n");
            fflush (stdout);
            close (fds[i].fd);
            trouver_client_com(client0,i,av);
            supprimer_client(client0,i);
            k--;
            fds[i].fd = sock;
            fds[i].events = POLLIN;
            int j=0;
            char tmp[len];
            liste_utilisateurs(client0,i,tmp,1);
            char host_lost[len];
            strcpy(host_lost,"/hostlost ");
            strcat(host_lost,tmp);
            while (av[j]!=NULL){
              av[j]->com=NULL;
              send(fds[av[j]->i].fd, host_lost,len,0);
              j++;
            }
            break;
          }

          else if (strncmp(buf,n,5)==0){
            if (creer_client(client0,i,buf+6)==1){
              struct client** av = malloc (20*sizeof(struct client));
              for (int k=0;k<20;k++){
                av[k]=NULL;
              }
              trouver_client_com(client0,i,av);
              char tmp[27];
              strcpy(tmp,"/nick");
              strcat(tmp,buf+6);
              send (fds[i].fd, tmp, 27, 0);

              char new[35];
              strcpy(new,"/newhostname ");
              strcat(new,buf+6);
              int j=0;
              while (av[j]!=NULL){
                send(fds[av[j]->i].fd, new,36,0);
                j++;
              }
              break;
            }


            else{
              send (fds[i].fd, "/fail_pseudo", 13, 0);
            }
          }
          else if (strncmp(buf,c,8)==0){
            struct client* correspondant = trouver_client(client0,buf+9);
            if (correspondant !=NULL){
              struct client* ptr = trouver_client_indice(client0,i);
              ptr->com=correspondant;
              char env[31];
              strcpy(env,"/new_host ");
              strcat(env,buf+9);
              send (fds[i].fd, env, 31, 0);
            }
            else if ((strcmp (buf+9,"all\n"))==0){
              struct client* ptr = trouver_client_indice(client0,i);
              ptr->com=client0;
              char env[31];
              strcpy(env,"/new_host ");
              strcat(env,buf+9);
              send (fds[i].fd, env, 31, 0);

            }
            else{
              send (fds[i].fd, "/fail_new_host", 15, 0);
            }
          }

          else if ((strncmp(buf,"/msg ",5))==0){
            int k=5;
            while ((*(buf+k)!=' ') && (*(buf+k)!='\0')){
              k++;
            }
            char tmp[22];
            for (int j=0;j<k-5;j++){
              tmp[j]=*(buf+5+j);
              tmp[j+1]='\n';
              tmp[j+2]='\0';
            }
            struct client* ptr = trouver_client_indice(client0,i);
            struct client* correspondant = trouver_client(client0,tmp);
            printf("%s",tmp);
            fflush(stdout);

            char msg[size+22];
            strcpy(msg,ptr->pseudo);
            msg[strlen(ptr->pseudo)]=' ';
            msg[strlen(ptr->pseudo)-1]='\0';
            strcat(msg," :");
            strcat(msg,buf+k+1);

            if ( correspondant !=NULL){
              send (fds[correspondant->i].fd, &msg, size+22, 0);
              send (fds[ptr->i].fd,"/msg ",6,0);
            }

            else if (strcmp(tmp,"all\n")==0){
              struct client* client_actuel =client0->next;
              while (client_actuel!=NULL){
                if (client_actuel->i != i){
                  send (fds[client_actuel->i].fd, &msg, size+22, 0);
                }
                client_actuel=client_actuel->next;
              }
              send (fds[ptr->i].fd,"/msg ",6,0);
            }

            else{
              send (fds[ptr->i].fd, "/msg destinataire introuvable\n", 31, 0);
            }
            break;
          }





          else if (strncmp(buf,wi,6)==0){
            char host_name[21];
            strcpy(host_name,buf+7);
            struct client* host;
            if ((host=trouver_client(client0,host_name))!=NULL){
              struct sockaddr_in* host_data=malloc(sizeof(struct sockaddr_in));
              socklen_t t=sizeof(struct sockaddr_in);
              getpeername(fds[host->i].fd,(struct sockaddr *)host_data,&t);
              int port_host = ntohs(host_data->sin_port);
              char addr_host[50];
              inet_ntop(AF_INET,&(host_data->sin_addr.s_addr),addr_host,50);
              char tmp[len];
              strcpy(tmp, "/whois ");
              host_name[strlen(host_name)-1]='\0';
              strcat(tmp,host_name);
              tmp[sizeof(tmp)-1]='\n';
              strcat(tmp," est connecté depuis le ");
              char a[50];
              sprintf(a,"%d/%d/%d %d:%d:%d ",host->instant.tm_mday,host->instant.tm_mon+1,host->instant.tm_year,host->instant.tm_hour+2,host->instant.tm_min,host->instant.tm_sec);
              strcat(tmp,a);
              strcat(tmp, "avec l'adresse ");
              strcat(tmp,addr_host);
              strcat(tmp," et le port ");
              char a2[50];
              sprintf(a2,"%d",port_host);
              strcat(tmp,a2 );
              send (fds[i].fd, tmp, len, 0);
            }
            else{
              send (fds[i].fd, "/fail_whois", 12, 0);
            }
            break;

          }

          else if(strncmp(buf,w,4)==0){
            char tmp [len];
            liste_utilisateurs(client0,i,tmp,1);
            send (fds[i].fd, tmp, len, 0);
          }







          else{
            if (trouver_client_indice(client0,i)==NULL){
              if (creer_client(client0,i,buf)==1){
                char tmp [len];
                liste_utilisateurs(client0,i,tmp,0);
                send (fds[i].fd, tmp, len, 0);
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
                if ((correspondant !=NULL) && (correspondant->i != i)){
                  ptr->com=correspondant;
                  send (fds[i].fd, "/communication_etablie", 23, 0);
                }
                else if(strcmp(buf,"all\n")==0){
                  ptr->com=client0;
                  send (fds[i].fd, "/communication_etablie", 23, 0);
                }
                else if(strncmp(buf,"create ",7)==0){
                  char name[28];
                  strcpy(name,"(salon)");
                  strcat(name,buf+7);
                  if (creer_client(client0,-1,name)==1){
                    ptr->com=trouver_client(client0,name);
                    send (fds[i].fd, "/communication_etablie", 23, 0);
                  }
                  else{
                    char tmp [len];
                    liste_utilisateurs(client0,i,tmp,0);
                    send (fds[i].fd, tmp, len, 0);
                  }
                }
                else{
                  char tmp [len];
                  liste_utilisateurs(client0,i,tmp,0);
                  send (fds[i].fd, tmp, len, 0);
                }
                break;
              }
              else{
                char msg[size+22];
                strcpy(msg,ptr->pseudo);
                msg[strlen(ptr->pseudo)]=' ';
                msg[strlen(ptr->pseudo)-1]='\0';
                strcat(msg," :");
                strcat(msg,buf);
                if (ptr->com==client0){
                  struct client* client_actuel =client0->next;
                  while (client_actuel!=NULL){
                    if (client_actuel->i != i){
                      send (fds[client_actuel->i].fd, &msg, size+22, 0);
                    }
                    client_actuel=client_actuel->next;
                  }
                }
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
