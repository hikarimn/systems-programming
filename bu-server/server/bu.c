
#include "csapp.h"



/** You originally had this:
char* GetFileModificationTime(char *path) {
    struct stat attr;
    char time_buf[128]; 
    if(stat(path,&attr) != 0)
        printf("Error in stat call.\n");
    strcpy(time_buf, ctime(&attr.st_mtime));
    strncat(time_buf, "\n", 128);
    // printf("Last modified time: %s", ctime(&attr.st_mtime));
    return time_buf;
}
The problem here is that you are returning a pointer to a local array.
That local array will vanish as soon as you exit the function.
I fixed this for you: **/
void GetFileModificationTime(char *path,char* size_buffer) {
    struct stat attr;
    stat(path,&attr);
    sprintf(size_buffer,"%ld\n",attr.st_mtime);
    printf("Last modified time: %ld\n", attr.st_mtime);
}


int FileExists (char *filename) {
  struct stat buffer;   
  if (stat (filename, &buffer) == 0) return 1;
  return 0;
}


void communicate(int connfd) {
    size_t n; 
    char buf[MAXLINE], path_buf[MAXLINE], size_buf[MAXLINE]; 
    rio_t rio;
    char* word;
    FILE *fp;
    
    Rio_readinitb(&rio, connfd);
    // while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { //line:netp:echo:eof
	    n = Rio_readlineb(&rio, buf, MAXLINE);
        printf("received string: %s\n", buf);
        printf("server received %d bytes\n", (int)n);

        // When it is a status call 
        if(strcmp(buf, "status\n") == 0){
            strcpy(path_buf, "copy/");
            Rio_readlineb(&rio, buf, MAXLINE); // read the file name
            /** You had
            strncat(path_buf, buf, MAXLINE);
            This will cause a problem similar to the problem you had in fwd.c, because
            buf has a \n at the end of it. I fixed this for you. **/
            buf[strlen(buf)-1] = '\0'; // Replace \n with \0
            strcat(path_buf,buf);
            printf("status!: %s\n", path_buf);

            // when the file exists
            if (FileExists(path_buf)){
                /** You had
                Rio_writen(connfd, GetFileModificationTime(path_buf), MAXLINE);
                printf("%s exists.\n", path_buf);
                printf("Modified at %s\n", GetFileModificationTime(path_buf));
                I fixed this to match the changes I made to GetFileModificationTime. **/
                GetFileModificationTime(path_buf,size_buf);
                Rio_writen(connfd,size_buf,strlen(size_buf));
            } else {
                /** You had
                Rio_writen(connfd, "0\n", MAXLINE);
                I fixed this. **/
                Rio_writen(connfd, "0\n", 2);
                printf("The file does not exist\n");
            }

        } else if(strcmp(buf, "upload\n") == 0){
            Rio_readlineb(&rio, buf, MAXLINE); // read the file name
            strcpy(path_buf, "copy/");
            /** You had
            strncat(path_buf, buf, MAXLINE);
            buf has a newline character at the end of it. We want to not include that
            in the path buffer. **/
            strncat(path_buf, buf, strlen(buf)-1);
            Rio_readlineb(&rio, buf, MAXLINE); // read the file size
            strcpy(size_buf, buf);
            /** Once again, the buffer you received from the client has a \n at the end of it.
                We need to get rid of it: **/ size_buf[strlen(size_buf)-1] = '\0';
            int size = 0;
            /** You had
        	size = *(unsigned int*)(size_buf);
            You can't convert a string to an int by doing a type cast. I fixed this for you. **/
            size = atoi(size_buf);
            /** There is no guarantee that the file the client sent you will fit in buf.
            What you have to do instead is to malloc a buffer large enough to receive the file.
            Also, you need to use Rio_readn here instead of Rio_readlineb. 
            Rio_readlineb(&rio, buf, MAXLINE); // read the file contents
            I fixed this: **/
            char* file_buf = malloc(size);
            Rio_readn(connfd,file_buf,size);
            /** You had
            fp = fopen(path_buf, "w");
            fseek(fp, size , SEEK_SET); 
            fprintf(fp,  "%s", buf); 
            fclose(fp);
            This will fail because the text you received does not have a terminating \0 at
            the end of it. I fixed this for you. **/
            printf("Path is %s\n",path_buf);
            int fd = Open(path_buf,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
            Write(fd,file_buf,size);
            Close(fd);
            free(file_buf);
            /** You had
            Rio_writen(connfd, "stored\n", n);
            I fixed this. **/
            char* msg = "stored\n";
            Rio_writen(connfd, "stored\n", strlen(msg));
        } 
}

int main(int argc, char **argv){
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
    	clientlen = sizeof(struct sockaddr_storage); 
	    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                        client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
	    communicate(connfd);
	    Close(connfd);
    }
    exit(0);
}
