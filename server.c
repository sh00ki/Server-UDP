// Yoad Shiran - 302978713

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

#include "slist.h"
#define BUFFER 4096


//client data for queue
typedef struct clientData {
    struct sockaddr cli;
    socklen_t cli_len;
    char buff [BUFFER];
}clientData_t;

char *blabla(char *string);
void createwordTolist(slist_t *list, char *buff, int len);
void sig_handler(int signo);
int validation(int argc, char** argv);


slist_t* slist = NULL;
int main(int argc, char** argv)
{
    if (validation(argc,argv) == EXIT_FAILURE)
    {
      fprintf(stderr, "usage: ./server <port>\n");
      exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    slist = (slist_t*)calloc(1,sizeof(slist_t));
    if(slist==NULL)
    {
        perror("ERROR in calloc(slist)\n");
        exit(1);
    }
    struct sockaddr_in srv;    /* used by recvfrom() */
    //socklen_t cli_len = sizeof(cli);  /* used by recvfrom() */
    int fd;
    int nbytes;             /* used by recvfrom() */
    if ((fd=socket(PF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("socket");
        exit(1);
    }
    memset((char *) &srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    //cli.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = htonl(INADDR_ANY);
    if( bind(fd , (struct sockaddr*)&srv, sizeof(srv) )<0)
    {
        perror("bind");
        exit(1);
    }
    fd_set readset,writeset;
    slist_init(slist);  // check if enter to NULL.
    if (signal(SIGINT, sig_handler) == SIG_ERR){}
    while(1)
    {
        FD_ZERO(&readset);
        FD_SET(fd, &readset);

        printf("server is ready to read\n");
        if(select(fd+1, &readset, &writeset, 0, 0) < 0) {
            perror("select");
            exit(1);
        }
        clientData_t *cl = (clientData_t*)malloc(sizeof(clientData_t));
        cl->cli_len = sizeof(cl->cli);
        if(FD_ISSET(fd, &readset))
        {
            memset(cl->buff, '\0', BUFFER);
            nbytes = recvfrom(fd, cl->buff, BUFFER, 0 ,(struct sockaddr*) &cl->cli, &cl->cli_len);

            if (nbytes < 0)
            {
                perror("recvfrom");
                free(cl);
                exit(1);
            }
            blabla(cl->buff);
            slist_append(slist, cl);
        }
        FD_SET(fd, &writeset);
        if(FD_ISSET(fd, &writeset))
        {
            clientData_t *lolo = slist_pop_first(slist);
            nbytes = sendto(fd, lolo->buff, BUFFER, 0,(struct sockaddr*) &lolo->cli, sizeof(lolo->cli));
            if(nbytes < 0) {
                perror("sendto");
                exit(1);
            }
            free(lolo);
        }
        FD_ZERO(&writeset);
    }
    return 0;
}
char *blabla(char *string)
{
    if (string!=NULL)
    {
        int size = strlen(string);
        int i=0;
        while(i<size)
        {
            if (string[i] >= 97 && string[i] <= 122)
            {
                string[i] = string[i] -32;
            }
            i++;
        }
    }
return string;
}
void createwordTolist(slist_t *list, char *buff, int len)
{
  if(buff == NULL)
  {
    perror("buff NULL 1");
    exit(1);
  }

  int size = len;
  char *temp = NULL;
  temp = (char*)calloc(size+1,2);
  if(temp == NULL)
  {
    perror("calloc");
    exit(1);
  }
  memset(temp, '\0', size);
  strcpy(temp,buff);
  if(slist_append(list, temp) != 0)
  {
    perror("slist_append");
    exit(1);
  }
}
void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        slist_destroy(slist ,SLIST_FREE_DATA);
        free(slist);
        exit(1);
    }
}

int validation(int argc, char** argv)
{
    if (argc==1 && argc!=2)
    {
        printf("usage: serverudp  <port>\n");
        return EXIT_FAILURE;
    }

    if(atoi(argv[1]) <= 0 || atoi(argv[1]) > 65535)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
