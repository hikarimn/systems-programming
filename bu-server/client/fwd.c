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

  /** You had
  while(1){
  I fixed this. **/
  while(run == 1) {
    /* Scan the watched directory for changes. */
    watchDIR = opendir(argv[1]);
    while((entp = readdir(watchDIR))!=NULL) {
      strcpy(path,argv[1]);
      strcat(path,"/");
      strcat(path,entp->d_name);
	  /** You had
      strcat(path,"\n");
      This puts a \n at the end of the path. You did this because 
      you were getting ready to send the file name to the server later.
      This is wrong for two reasons. The first reason is that you are going
      to use this path string in the stat call below. I suspected that having
      the \n at the end of the path would cause the stat call to fail. To
      test this suspicion I put some logic around the stat call to test for
      errors. Sure enough, when the stat call ran it always failed. The 
      second reason this is wrong is that you don't want to send the full path
      to the server. The only thing the server needs is the file name, and you
      can get that from just entp->d_name. **/
      strcpy(status_buf, "status\n");

      fprintf(logFile,"%s * \n", status_buf);
      
      
      if(stat(path,&s) != 0)
        printf("Error in stat call.\n");
      /* Only pay attention to files, and ignore directories. */
      if(S_ISREG(s.st_mode)) {
        
        /** You had
        fprintf(logFile,"Checking if %s exists. \n", status_buf);
        I fixed this. **/
        fprintf(logFile,"Checking if %s exists. \n", entp->d_name);

        clientfd = Open_clientfd(host, port);
        Rio_readinitb(&rio, clientfd);

        Rio_writen(clientfd, status_buf, strlen(status_buf));
        /** I added this: **/ strcpy(status_buf,entp->d_name); strcat(status_buf,"\n");
        Rio_writen(clientfd, status_buf, strlen(status_buf));
        // close(clientfd);
        
       /* You had

        // size_t n; 

        // This part is not working 
        // size_t n; 
        // while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { //line:netp:echo:eof
        //   Rio_readlineb(&rio, received_buf, MAXLINE);
         //  fprintf(logFile, "received string: %s\n", received_buf);
        // }
        
        Now that the server is fixed, we can go ahead and do the read. **/
        Rio_readlineb(&rio, received_buf, MAXLINE);
        /** Also, the server is designed to close the connection to the client after 
            each request is complete. We should now close our connection to the server. **/
        close(clientfd);
        
        /** You had
        char str1[] = "0";
        Note that even when the server sends you a 0 there will also be a \n at the end of the line. **/
        char* str1 = "0\n";
        // When the server does not have a copy of the file
		/** You need an additional case in your if statement. If the modification time 
            of the remote file is less than the modification time of the local file you 
            should upload as well. **/
        if (strcmp(str1, received_buf) == 0){
          /** Each separate request to the server requires a new connection. **/
          clientfd = Open_clientfd(host, port);
          strcpy(status_buf, "upload\n");
          // Sending the "upload" req
          Rio_writen(clientfd, status_buf, strlen(status_buf));

          /** You had
          strcpy(status_buf, argv[1]);
          strcat(status_buf, "/");
          strcat(status_buf,entp->d_name);
          strcat(status_buf, "\n");
          For an upload request you should only send the file name, not the full path. **/
          strcpy(status_buf, entp->d_name);
          strcat(status_buf, "\n");
          
          // Sending the file name
          Rio_writen(clientfd, status_buf, strlen(status_buf));
           
          /** You had
          strcpy(status_buf, s.st_size);
          This won't work, because st_size is an integer, not a string. I fixed this. **/
          sprintf(status_buf,"%ld\n",s.st_size);
          // Sending the file size
          Rio_writen(clientfd, status_buf, strlen(status_buf));

          // Reading the stuff in the file and sending the second part of updates
          char *contents_buff;
          
          FILE *fh = fopen(path, "rb");
          if ( fh != NULL ){
            fseek(fh, 0L, SEEK_END);
            long ftell_long = ftell(fh);
            rewind(fh);
            contents_buff = malloc(ftell_long);
            if ( contents_buff != NULL ){
              /** You had
              fread(contents_buff, ftell_long, 1, fh);
              I fixed this. **/
              fread(contents_buff, 1, ftell_long, fh);
              fclose(fh); 
              fh = NULL;
              // Sending the file contents
              /** You had
              Rio_writen(clientfd, contents_buff, strlen(contents_buff));
              When you read the file contents using fread, there will not be a 
              terminating \0 at the end of the buffer. This means that you can't use strlen here.
              I fixed this for you. **/
              Rio_writen(clientfd, contents_buff, (int) ftell_long);
              free(contents_buff);
            }
            if (fh != NULL) fclose(fh);
          } else
            printf("Could not open %s\n",path);

            Rio_readlineb(&rio, received_buf, MAXLINE);
            fprintf(logFile, "received string: %s\n", received_buf);
            close(clientfd);
          }
       
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
