#include <sys/types.h>
#include <sys/stat.h>
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


#define len 1000



struct personnes{
  char pseudo[21];
  struct personnes* next;
};

struct file{
  int file;
  char name[50];
  struct personnes* first;
  volatile struct file* next;
};

struct send_file{
  int file;
  char add[50];
  char name[50];
  int port;
  struct send_file* next;
  int count;
};

struct rcv_file{
  char** argv;
  int* portrd;
};

struct thread_arg{
  char pseudo[22];
  char pseudo_envoi[22];
  int sock;
  volatile int b;
  volatile int c;
  struct file *file;
  char** argv;
  int* portrd;

};


void *s_file(struct send_file *arg){
  char msg[1024];
  while(1){
    if (arg->count!=0){
      int sock = socket( AF_INET, SOCK_STREAM, 0 );
      struct sockaddr_in sock_host;
      memset(&sock_host, '\0', sizeof (struct sockaddr_in));
      if (sock == -1){
        printf ("erreur socket");
        perror ("socket");
        return 0;
      }
      sock_host.sin_family = AF_INET;
      sock_host.sin_port = htons(arg->next->port);
      sock_host.sin_addr.s_addr = inet_addr(arg->next->add);
      int connexion = connect(sock,(struct sockaddr*)&sock_host,sizeof(sock_host));
      strcpy(msg,arg->next->name);
      char aa[2];
      send(sock,msg,1023,0);
      recv(sock,aa,1,0);
      int r=1;
      while(r!=0){
        r=read(arg->next->file,msg,1023);
        msg[r]='\0';
        send(sock,msg,r,0);
      }
      lseek(arg->next->file,0,SEEK_SET);
      struct send_file* a=arg->next;
      arg->next=arg->next->next;
      (arg->count)--;
      free(a);
      close(sock);

    }

  }
}

void *r_file(struct rcv_file* arg){

  int sock = socket( AF_INET, SOCK_STREAM, 0 );
  struct sockaddr_in mon_adresse;
  memset(&mon_adresse, '\0', sizeof(struct sockaddr_in));
  mon_adresse.sin_family = AF_INET;
  *(arg->portrd)=rand()%(65535-1025) +1025;
  mon_adresse.sin_port = htons(*(arg->portrd));
  mon_adresse.sin_addr.s_addr = inet_addr((arg->argv)[2]);

  while(-1== bind (sock, (struct sockaddr*)&mon_adresse, sizeof(struct sockaddr_in))){
    *(arg->portrd)=rand()%(65535-1025) +1025;
    mon_adresse.sin_port = htons(*(arg->portrd));
  }
  listen(sock,1);
  struct sockaddr addr;
  socklen_t addrlen;
  while(1){
    int sock2=accept(sock,&addr,&addrlen);
    char msg[1024];
    int r=-2;
    r=recv(sock2,msg,1023,0);
    char name[1028];
    msg[r]='\0';
    strcpy(name,"recu");
    strcat(name,msg);
    send(sock2," ",1,0);
    int i=open(name,O_WRONLY|O_CREAT,S_IRWXU);
    while( r!= 0){
      r=recv(sock2,msg,1023,0);
      msg[r]='\0';
      write(i,msg,r);
    }
    close(i);
    close(sock2);
  }
}

void *envoi(struct thread_arg *arg){
  char message[len];

  while (1){
    if (arg->c){
      free (arg);
      return NULL;
    }
    if(arg->b){


      strcpy(message,"");

      printf("message de %s pour %s (max 1000 caractères)\n",arg->pseudo, arg->pseudo_envoi);
      fflush(stdout);
      fgets (message, len, stdin);
      if (strcmp(message,"\n")!=0){
        int l = strlen(message);
        if (strncmp(message, "/send ",6)==0){
          char name[21];
          char file[50];
          int j=0;
          while (*(message+6+j)!=' ' && *(message+6+j)!='\0'){
            j++;
          }
          if (*(message+6+j)==' '){
            for (int i=0;i<j;i++){
              name[i]=message[i+6];
            }
            name[j]='\n';
            name[j+1]='\0';
            int k=j+1;
            while (*(message+6+k)!=' ' && *(message+6+k)!='\0'){
              k++;
            }
            if (*(message+6+k)=='\0'){
              for (int i=j+1;i<k;i++){
                file[i-j-1]=message[i+6];
              }
              file[k-j-2]='\0';
              struct file* fichier=arg->file;
              if (fichier==NULL){
                int des = open(file,O_RDONLY);
                if (des != -1){
                  arg->file=malloc(sizeof(struct file));
                  fichier=arg->file;
                  fichier->file=des;
                  fichier->first=malloc(sizeof(struct personnes));
                  strcpy(fichier->first->pseudo,name);
                  fichier->first->next=NULL;
                  strcpy(fichier->name,file);
                  if (strcmp(name,"all\n")==0){
                    printf ("impossible d'envoyer un fichier en broadcast\n");
                    fflush (stdout);
                  }
                  else if(strncmp(name,"(salon)",7)==0){
                    printf("impossible d'envoyer un fichier dans un salon\n");
                    fflush(stdout);
                  }
                  else if(strncmp(name,arg->pseudo,strlen(name)-1)==0){
                    printf("vous ne pouvez pas vous envoyer un fichier\n");
                    fflush(stdout);
                  }
                  else{
                    printf("demande d'envoi de %s à %s",file,name);
                    fflush(stdout);
                    arg->b=0;
                    int len_sent = send ( arg->sock,message, l+1, 0);

                  }
                }
                else{
                  printf ("fichier %s introuvable\n",file);
                  fflush(stdout);
                }


              }
              else{
                while (fichier->next!=NULL){
                  if (strcmp(fichier->name,file)==0){
                    break;
                  }
                  else{
                    fichier=fichier->next;
                  }
                }
                if (strcmp(fichier->name,file)!=0){
                  int des = open(file,O_RDONLY|O_NONBLOCK);
                  if (des != -1){
                    fichier->next=malloc(sizeof(struct file));
                    fichier=fichier->next;
                    fichier->file=des;
                    fichier->first=malloc(sizeof(struct personnes));
                    strcpy(fichier->first->pseudo,name);
                    fichier->first->next=NULL;
                    strcpy(fichier->name,file);
                    if (strcmp(name,"all\n")==0){
                      printf ("impossible d'envoyer un fichier en broadcast\n");
                      fflush (stdout);
                    }
                    else if(strncmp(name,"(salon)",7)==0){
                      printf("impossible d'envoyer un fichier dans un salon\n");
                      fflush(stdout);
                    }
                    else if(strncmp(name,arg->pseudo,strlen(name)-1)==0){
                      printf("vous ne pouvez pas vous envoyer un fichier\n");
                      fflush(stdout);
                    }
                    else{
                      printf("demande d'envoi de %s à %s",file,name);
                      fflush(stdout);
                      arg->b=0;
                      int len_sent = send ( arg->sock,message, l+1, 0);
                    }
                  }
                  else{
                    printf ("fichier %s introuvable\n",file);
                    fflush(stdout);
                  }

                }
                else{
                  struct personnes *per=fichier->first;
                  while (per!=NULL){
                    if (strcmp(per->pseudo,name)==0){
                      break;
                    }
                    else{
                      per=per->next;
                    }
                  }
                  if (per==NULL){
                    per=malloc(sizeof(struct personnes));
                    strcpy(per->pseudo,name);
                    per->next=NULL;
                  }
                  if (strcmp(name,"all\n")==0){
                    printf ("impossible d'envoyer un fichier en broadcast\n");
                    fflush (stdout);
                  }
                  else if(strncmp(name,"(salon)",7)==0){
                    printf("impossible d'envoyer un fichier dans un salon\n");
                    fflush(stdout);
                  }
                  else if(strncmp(name,arg->pseudo,strlen(name)-1)==0){
                    printf("vous ne pouvez pas vous envoyer un fichier\n");
                    fflush(stdout);
                  }
                  else{
                    printf("demande d'envoi de %s à %s",file,name);
                    fflush(stdout);
                    int len_sent = send ( arg->sock,message, l+1, 0);
                    arg->b=0;
                  }
                }
              }
            }
          }
        }
        else if( strncmp(message,"/accept ",8)==0){
          strcat(message," ");
          char pp[10];
          sprintf(pp,"%d",*(arg->portrd));
          strcat(message,(pp));
          strcat(message," ");
          strcat(message,(arg->argv)[2]);
          int len_sent = send ( arg->sock,message, l+50, 0);
        }
        else{
          int len_sent = send ( arg->sock,message, l+1, 0);

          if (strstr(message,"/quit")!=NULL){
            exit;
          }
          else if (strncmp(message, "/nick", 5)==0){
            arg->b=0;
          }
          else if (strncmp(message,"/connect",8)==0){
            arg->b=0;
          }
          else if (strncmp(message,"/whois",6)==0){
            arg->b=0;
          }
          else if (strncmp(message,"/who",4)==0){
            arg->b=0;
          }
          else if (strncmp(message,"/msg ",5)==0){
            arg->b=0;
          }
          else if (strncmp(message,"/create ",8)==0){
            arg->b=0;
          }
          else if (strncmp(message, "/leave ",7)==0){
            arg->b=0;
          }
        }
      }

    }
  }

}





int main (int argc, char ** argv){

  char  q[]="/quit\n";
  char w[]="/who ";
  char wi[]="/whois";
  char n[]="/nick";

  if (argc!=3){
    printf ("arguments");
    return 0;
  }

  const char *IP =argv[2];
  int Port = atoi(argv[1]);



  char recu[len];
  char pseudo[22];
  char liste[len];
  char test_b[] = "0\n";
  char pseudo_envoi[29];
  char co_part[len];
  char t[]="0";
  int q1=0;
  int flags;
  int port_rd;

  struct send_file *jalon5=malloc(sizeof(struct rcv_file));
  jalon5->next=NULL;
  jalon5->file=0;
  jalon5->port=0;
  jalon5->add[0]='\0';
  jalon5->name[0]='\0';
  jalon5->count=0;
  struct rcv_file * jalon5r=malloc(sizeof (struct rcv_file));
  jalon5r->argv=argv;
  jalon5r->portrd=&port_rd;
  struct file *file=NULL;

  pthread_t th1;
  pthread_create(&th1, NULL, (void *)s_file,jalon5);

  pthread_t th2;
  pthread_create(&th2, NULL, (void *)r_file,jalon5r);


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
  recv ( sock, liste, len, 0);
  while (strcmp(liste,"/client_non_cree")==0){
    printf ("pseudo invalide\n");
    fgets (pseudo, len, stdin);
    l1 = strlen(pseudo);
    send ( sock, pseudo, l1+1, 0);
    recv ( sock, liste, len, 0);
  }

  struct thread_arg arg;

  strcpy(arg.pseudo,pseudo);
  *(arg.pseudo+strlen(arg.pseudo)-1)='\0';
  strcpy(arg.pseudo_envoi," ");
  arg.sock=sock;
  arg.b=0;
  arg.c=0;
  arg.file=file;
  arg.argv=argv;
  arg.portrd=&port_rd;

  pthread_t th;
  pthread_create(&th, NULL, (void *)envoi,&arg);

  while (1){

//    printf ("%s\n",liste);
//    fflush(stdout);
//    fgets (pseudo_envoi, len, stdin);
//    int l2 = strlen(pseudo_envoi);
//
    strcpy(pseudo_envoi,"all\n");
    int l2= 5;
//
//

    send ( sock, pseudo_envoi, l2+1, 0);

    q1= recv ( sock, co_part, len, 0);
    while (strstr(co_part,"/communication_etablie")==NULL){
      printf("%s",co_part);
      fflush(stdout);
      printf ("%s\n",co_part);
      poll(fds,1,1);
      while (fds[0].revents==POLLIN){
        recv ( sock, &co_part, len, 0);
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
      recv ( sock, co_part, len, 0);
    }
//    printf ("%s\n",co_part);
//    fflush(stdout);


      strcpy(arg.pseudo,pseudo);
      *(arg.pseudo+strlen(arg.pseudo)-1)='\0';
      if (strncmp(pseudo_envoi,"create ",7)==0){
        *(pseudo_envoi)='(';
        *(pseudo_envoi+1)='s';
        *(pseudo_envoi+2)='a';
        *(pseudo_envoi+3)='l';
        *(pseudo_envoi+4)='o';
        *(pseudo_envoi+5)='n';
        *(pseudo_envoi+6)=')';
      }
      strcpy(arg.pseudo_envoi,pseudo_envoi);
      *(arg.pseudo_envoi+strlen(arg.pseudo_envoi)-1)='\0';
      arg.sock=sock;
      arg.b=1;



    while (1){

      strcpy(recu,"");


      int len_rcv= recv ( sock, recu, len, 0);

      if (strstr(recu, q)!=NULL){
        printf ("connexion terminée\n");
        fflush(stdout);
        arg.c=1;
        return 0;
      }
      else if (strstr(recu, n)!=NULL){
        arg.b=0;
        strcpy(pseudo,(recu+5));
        strcpy(arg.pseudo,(recu+5));
        *(arg.pseudo+strlen(arg.pseudo)-1)='\0';
        arg.b=1;
      }
      else if (strncmp(recu,"personne de connecte pour le moment",73)==0){
        arg.b=0;
        strcpy(liste,recu);
        strcpy(co_part,liste);
        break;
      }
      else if (strncmp(recu,wi,6)==0){
        printf("%s\n",recu+7);
        fflush(stdout);
        arg.b=1;
      }
      else if (strncmp(recu,w,4)==0){
        printf("\n/connect pseudo pour changer d'interlocuteur\n/whois pseudo pour obtenir des informations\n%s",(recu+5));
        arg.b=1;
      }
      else if(strncmp(recu,"/newhostname",12)==0){
        pseudo_envoi[strlen(pseudo_envoi)-1]='\0';
        printf ("%s devient %s\n",pseudo_envoi,(recu+13));
        strcpy(pseudo_envoi,(recu+13));
        strcpy(arg.pseudo_envoi,(recu+13));
        *(arg.pseudo_envoi+strlen(arg.pseudo_envoi)-1)='\0';
      }
      else if(strstr(recu,"/hostlost")!=NULL){
        arg.b=0;
        printf("connexion interrompue: envoyez un message pour rafraichir puis choisissez un contact :\n");
        fflush(stdout);
        strcpy(liste,"");
        strcpy(co_part,liste);
        break;
      }
      else if(strncmp(recu,"/left ",6)==0){
        arg.b=0;
        printf("Salon quitté\n");
        fflush(stdout);
        char tmp[50];
        strcpy(tmp,recu+6);
        if (strstr(tmp,arg.pseudo_envoi)!=NULL){
          printf("Entrer pour rafraichir\n");
          fflush(stdout);
          strcpy(liste,"");
          strcpy(co_part,liste);
          break;
        }
        arg.b=1;
      }
      else if(strncmp(recu,"/new_host",9)==0){
        printf("nouvelle connexion établie\n");
        char tmp[21];
        strcpy(tmp,recu+10);
        tmp[strlen(tmp)-1]='\0';
        strcpy(pseudo_envoi,tmp);
        strcpy(arg.pseudo_envoi,tmp);
        arg.b=1;
      }
      else if(strncmp(recu,"/fail_new_host",14)==0){
        printf ("utilisateur introuvable\n");
        arg.b=1;
      }
      else if(strncmp(recu,"/fail_create",12)==0){
        printf("impossible de créer ce salon\n");
        fflush(stdout);
        arg.b=1;
      }
      else if(strncmp(recu,"/fail_pseudo",12)==0){
        printf("pseudo non valable\n");
        arg.b=1;
      }
      else if(strncmp(recu,"/fail_whois",11)==0){
        printf("utilisateur introuvable\n");
        arg.b=1;
      }
      else if ((strncmp(recu,"/msg ",5))==0){
        printf("%s",recu+5);
        arg.b=1;
      }
      else if((strncmp(recu,"/ok",3))==0){
        arg.b=1;
      }
      else if((strncmp(recu,"/pasok",7))==0){
        printf("utilisateur introuvable\n");
        fflush(stdout);
        arg.b=1;
      }
      else if((strncmp(recu,"/send ",6))==0){
        char name[21];
        char file[50];
        int j=0;
        while (*(recu+6+j)!=' ' && *(recu+6+j)!='\0'){
          j++;
        }
        if (*(recu+6+j)==' '){
          for (int i=0;i<j;i++){
            name[i]=recu[i+6];
          }
          name[j-1]='\0';
          int k=j+1;
          while (*(recu+6+k)!=' ' && *(recu+6+k)!='\0'){
            k++;
          }
          if (*(recu+6+k)=='\0'){
            for (int i=j+1;i<k;i++){
              file[i-j-1]=recu[i+6];
            }
            file[k-j-2]='\0';
            printf("%s veut vous envoyer %s : /accept %s %s pour accepter\n",name,file,name,file);
            fflush(stdout);
          }
        }
      }
      else if ((strncmp(recu,"/accept ",8))==0){
        char port_str[50];
        int port;
        char adresse[50];
        char name[21];
        char file[50];
        int j=0;
        while (*(recu+8+j)!=' ' && *(recu+8+j)!='\0'){
          j++;
        }
        if (*(recu+8+j)==' '){
          for (int i=0;i<j;i++){
            name[i]=recu[i+8];
          }
          name[j]='\0';
          int k=j+1;
          while (*(recu+8+k)!=' ' && *(recu+8+k)!='\0'){
            k++;
          }
          if (*(recu+8+k)==' '){
            for (int i=j+1;i<k;i++){
              file[i-j-1]=recu[i+8];
            }
            file[k-j-1]='\0';
            int k2=k+1;
            while (*(recu+8+k2)!=' ' && *(recu+8+k2)!='\0'){
              k2++;
            }
            if (*(recu+8+k2)==' '){
              for(int i=k+1;i<k2;i++){
                port_str[i-k-1]=recu[i+8];
              }
              port_str[k2-k-1]='\0';
              port=atoi(port_str);
              int k3=k2+1;
              while (*(recu+8+k3)!=' ' && *(recu+8+k3)!='\0'){
                k3++;
              }
              if (*(recu+8+k3)=='\0'){
                for(int i=k2+1;i<k3;i++){
                  adresse[i-k2-1]=recu[i+8];
                }
                adresse[k2-k2-1]='\0';
                struct file* f=arg.file;

                while (f!=NULL){
                  if (strcmp(f->name,file)==0){
                    break;
                  }
                  f=f->next;
                }
                if (f!=NULL){
                  struct personnes* p= f->first;
                  while (p!=NULL){
                    if (strcmp(p->pseudo,name)==0){
                      break;
                    }
                    p=p->next;
                  }
                  if (p!=NULL){
                    struct send_file* v = jalon5;
                    while(v->next!=NULL){
                      v=v->next;
                    }
                    struct send_file *v2=malloc(sizeof(struct send_file));
                    strcpy(v2->name,file);
                    v2->port=port;
                    v2->next=NULL;
                    strcpy(v2->add,adresse);
                    v2->file=f->file;
                    v2->count=0;
                    v->next=v2;
                    (jalon5->count)++;
                  }
                }
              }
            }
          }
        }
      }
      else{
        printf ("%s\n", recu);
      }

    }

  }
  return 1;
}
