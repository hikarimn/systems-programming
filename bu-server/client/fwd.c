#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include "csapp.h"

// - fw program (no changes)
//      ./fw start test 
// - fwd program  
//     Scans the watched directory.
//     For each file, send a (status) request to bu server to see 
//     if the server has a copy of that file
//     If the server doesn't have a backup file or the backup
//     file is old, fwd will upload a copy of the file to 
//     the server  using a (upload) request.
//     fwd checks the change every 5 minutes.
// - bu.c (new server application)
//     Listen for connection on port 8000.
//     Clients connect to the server and send a single request.
//     Server sends back a response to  the request, 
//     close the connection to the client,
//     and go back to listening for another client connection.%    

int run;

void handleTERM(int x) {
  run = 0;
}

int main(int argc, char **argv){
  FILE* logFile;
  struct sigaction action, old_action;
  time_t timer;
  DIR* watchDIR;
  struct dirent* entp;
  struct stat s;
  char path[1024];
  char *host, *port, buf[MAXLINE], status_buf[MAXLINE], received_buf[MAXLINE];
  int clientfd;
  rio_t rio;

  /* Install the handler for SIGTERM */
  action.sa_handler = handleTERM;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGTERM, &action, &old_action);

  /* Open the log file and start watching for changes. **/
  run = 1;
  logFile = fopen("fwd.log","a");
  host = "127.0.0.1";
  port = "8000";
  // clientfd = Open_clientfd(host, port);
  // Rio_readinitb(&rio, clientfd);

  while(1){
    
    /* Scan the watched directory for changes. */
    watchDIR = opendir(argv[1]);
    while((entp = readdir(watchDIR))!=NULL) {
      strcpy(path,argv[1]);
      strcat(path,"/");
      strcat(path,entp->d_name);

      strcpy(status_buf, "status ");
      strcat(status_buf, argv[1]);
      strcat(status_buf, "/");
      strcat(status_buf,entp->d_name);
      fprintf(logFile,"%s * \n", status_buf);
      
      stat(path,&s);
      /* Only pay attention to files, and ignore directories. */
      if(S_ISREG(s.st_mode)) {
        
        fprintf(logFile,"Checking if %s exists. \n", status_buf);
        
        clientfd = Open_clientfd(host, port);
        Rio_readinitb(&rio, clientfd);

        Rio_writen(clientfd, status_buf, strlen(status_buf));
        // close(clientfd);
        

        // size_t n; 

        // This part is not working 
        Rio_readlineb(&rio, received_buf, MAXLINE);
        fprintf(logFile, "received string: %s\n", received_buf);
        

        // char str1[] = "0";
        // // When the server does not have a copy of the file
        // if (strcmp(str1, received_buf) == 0){
        //   strcpy(status_buf, "upload ");
        //   strcat(status_buf, argv[1]);
        //   strcat(status_buf, "/");
        //   strcat(status_buf,entp->d_name);
        //   strcat(status_buf, " ");
        //   strcat(status_buf, s.st_size);

          // Sending the first part of updates
          // Rio_writen(clientfd, status_buf, strlen(status_buf));
          
          // Reading the stuff in the file and sending the second part of updates
          // char *buffer;
          // FILE *fh = fopen(path, "rb");
          // if ( fh != NULL ){
          //   fseek(fh, 0L, SEEK_END);
          //   long ftell = ftell(fh);
          //   rewind(fh);
          //   buffer = malloc(ftell);
          //   if ( buffer != NULL ){
          //     fread(buffer, ftell, 1, fh);
          //     fclose(fh); 
          //     fh = NULL;
 
          //     Rio_writen(clientfd, buffer, strlen(buffer));
          //     free(buffer);
          //   }
          //   if (fh != NULL) fclose(fh);
          // }

          // Rio_readlineb(&rio, received_buf, MAXLINE);
          // fprintf(logFile, "received string: %s\n", received_buf);
           close(clientfd);
        }
       
      }
    closedir(watchDIR);
    /* Note the time and go to sleep. */
    time(&timer);
    sleep(60);

    }

    
  
  
  fclose(logFile);
  return 0;

}
