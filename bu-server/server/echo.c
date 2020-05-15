/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"

int StartsWith(const char *a, const char *b){
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

int FileExists (char *filename) {
  struct stat buffer;   
  if (stat (filename, &buffer) == 0) return 1;
  return 0;
}

char* GetFileModificationTime(char *path) {
    struct stat attr;
    stat(path, &attr);
    // printf("Last modified time: %s", ctime(&attr.st_mtime));
    return ctime(&attr.st_mtime);
}

void echo(int connfd) {
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
        // if(StartsWith(buf, "status")){

        // When it is a status call 
        if(StartsWith(buf, "status")){
            word = strtok(buf, " ");
            strcpy(path_buf, "copy/");
            // printf("status!: %s\n", buf);
            word = strtok(NULL, " ");
            // word = strtok(buf, " ");
            strncat(path_buf, word, MAXLINE);
            printf("status!: %s\n", path_buf);

            // when the file exists
            if (FileExists(path_buf)){
                // Rio_writen(connfd, buf, n);
                Rio_writen(connfd, GetFileModificationTime(path_buf), n);
                printf("%s exists\n", path_buf);
                printf("Modified at %s\n", GetFileModificationTime(path_buf));
            } else {
                Rio_writen(connfd, "0", n);
                printf("The file does not exist\n");
            }

        } else if(StartsWith(buf, "upload")){
            // receiving the first half of the uploads
            word = strtok(buf, " ");
            strcpy(path_buf, "copy/");
            // printf("status!: %s\n", buf);
            word = strtok(NULL, " ");
            // word = strtok(buf, " ");
            strncat(path_buf, word, MAXLINE);
            word = strtok(NULL, " ");
            strncat(size_buf, word, MAXLINE);
            int size = 0;
        	size = *(unsigned int*)(size_buf);

            fp = fopen(path_buf, "w");
            fseek(fp, size , SEEK_SET);
            fputc('\0', fp);
            
        } else {
            // receiving the second half of the uploads
            if(fp == NULL)
                exit(-1);
            fprintf(fp, buf);
            fclose(fp);
            Rio_writen(connfd, "stored", n);
        }
}
/* $end echo */
