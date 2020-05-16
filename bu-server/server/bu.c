#include "csapp.h"

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        serve_request(connfd);  
        Close(connfd); 
    }
}

void serve_request(int connfd)
{
    char buffer[MAXLINE], fileName[MAXLINE];
    char *fileBuf;
    int size;
    rio_t rio;
    int fd;
    struct stat s;

    /* Read request */
    Rio_readinitb(&rio, connfd);
    if (!Rio_readlineb(&rio, buffer, MAXLINE))
        return;
    if (strcmp(buffer, "upload\n") == 0) {
        Rio_readlineb(&rio, fileName, MAXLINE);
        Rio_readlineb(&rio, buffer, MAXLINE);
        printf("Request to upload %s\n",fileName);
        sscanf(buffer,"%d",&size);
        fileBuf = Malloc(size);
        if(fileBuf != NULL) {
           Rio_readnb(&rio,fileBuf,size);
           fd = Open(fileName,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
           Write(fd,fileBuf,size);
           Close(fd);
           Free(fileBuf);
        }
        strcpy(buffer,"stored\n");
        Rio_writen(connfd,buffer,strlen(buffer));
    } else if(strcmp(buffer,"status\n") == 0) {
        Rio_readlineb(&rio, fileName, MAXLINE);
        printf("Request to stat %s\n",fileName);
        fileName[strlen(fileName)-1] = '\0';
        if(stat(fileName,&s) == 0)
          sprintf(buffer,"%d\n",s.st_mtime);
        else
          strcpy(buffer,"0\n");
        Rio_writen(connfd,buffer,strlen(buffer));
    }
        
}

