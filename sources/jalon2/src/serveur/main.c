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
#define n_co 2



int main (int argc, char ** argv){

  char  q[6]="/quit";

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
  struct pollfd fds[n_co];
  for (int j=0; j<n_co;j++){
    fds[j].fd = sock;
    fds[j].events = POLLIN;
  }

  int k=0;

  while (1){

    poll(fds,n_co,-1);

    for (int i =0 ; i<k+1;i++){

      if (fds[i].revents == POLLIN){
        if (fds[i].fd==sock){

          if (k<n_co){
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

          if (strncmp(buf,q,5)==0){
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

//    int newsock= accept (sock, &addr, &addrlen);





}




  return 0;
}
