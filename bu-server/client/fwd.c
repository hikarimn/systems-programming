#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include "csapp.h"

int run;
/* Locations of important files */
const char* pidFileLoc = "/run/fwd.pid";
const char* confFileLoc = "/etc/fwd.conf";
const char* logFileLoc = "/var/log/fwd.log";
FILE *logFile;

void handleTERM(int x) {
  run = 0;
}

void upload_file(char* dir,char* fileName,int filesize) {
    int srcfd, clientfd;
    char *host, *port, *cmd, *srcp;
    char buffer[256];
    rio_t rio;

    host = "127.0.0.1";
    port = "8000";
    cmd = "upload\n";
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    Rio_writen(clientfd, cmd, strlen(cmd));
    strcpy(buffer,fileName);
    strcat(buffer,"\n");
    Rio_writen(clientfd, buffer, strlen(buffer));
    sprintf(buffer,"%d\n",filesize);
    Rio_writen(clientfd, buffer, strlen(buffer));

	strcpy(buffer,dir);
    strcat(buffer,"/");
    strcat(buffer,fileName);
    srcfd = Open(buffer, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd); 
    Rio_writen(clientfd, srcp, filesize);
    Munmap(srcp, filesize);          

	Rio_readlineb(&rio, buffer, 256);
	Close(clientfd); 
}

void check_file(char* dir,char* fileName,int time,int size) {
    int clientfd;
    char *host, *port, *cmd, *srcp;
    char buffer[256];
    rio_t rio;
    int serverTime;

    host = "127.0.0.1";
    port = "8000";
    cmd = "status\n";
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    Rio_writen(clientfd, cmd, strlen(cmd));
    strcpy(buffer,fileName);
    strcat(buffer,"\n");
    Rio_writen(clientfd, buffer, strlen(buffer));
    
	Rio_readlineb(&rio, buffer, 256);
	Close(clientfd);
    sscanf(buffer,"%d",&serverTime);
    if(serverTime < time)
      upload_file(dir,fileName,size);
}  

/*
 * readConfig - read the configuration file
 */
int readConfig(char *path, char *address,char *port) {
  FILE *config;

  config = fopen(confFileLoc,"r");
  if(config == NULL)
    return -1;
  fscanf(config,"%s",path);
  fscanf(config,"%s",address);
  fscanf(config,"%s",port);
  fclose(config);
  return 0;
}

/*
 * logOpen - open the log file
 */
int logOpen() {
  logFile = fopen(logFileLoc,"a");
  if(logFile == NULL)
    return -1;
  return 0;
}

/* 
 * logMessage - write a message to the log file
 */
void logMessage(char *msg) {
  if(logFile != NULL) {
   fprintf(logFile,"%s\n",msg);
   fflush(logFile);
  }
}

/*
 * logClose - close the log file
 */
void logClose() {
  if(logFile != NULL)
    fclose(logFile);
}

int main(int argc, char **argv)
{
  struct sigaction action, old_action;
  time_t timer;
  int buModTime;
  DIR* watchDIR;
  struct dirent* entp;
  struct stat s;
  char path[1024];

  /* Install the handler for SIGTERM */
  action.sa_handler = handleTERM;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGTERM, &action, &old_action);

  /* Run the initial scan. */
  watchDIR = opendir(argv[1]);
  while((entp = readdir(watchDIR))!=NULL) {
    strcpy(path,argv[1]);
	strcat(path,"/");
	strcat(path,entp->d_name);
	stat(path,&s);
	/* Only pay attention to files, and ignore directories. */
	if(S_ISREG(s.st_mode)) 
	  check_file(argv[1],entp->d_name,s.st_mtime,s.st_size);
  }
  closedir(watchDIR);

  /* Start watching for changes. **/
  run = 1;
  while(run == 1) {
    /* Note the time and go to sleep. */
    time(&timer);
    sleep(60);
    /* Scan the watched directory for changes. */
    watchDIR = opendir(argv[1]);
    while((entp = readdir(watchDIR))!=NULL) {
      strcpy(path,argv[1]);
      strcat(path,"/");
      strcat(path,entp->d_name);
      stat(path,&s);
      /* Only pay attention to files, and ignore directories. */
      if(S_ISREG(s.st_mode)) {
        /* Upload recently modified files. */
        if(difftime(s.st_mtime,timer) > 0) {
          upload_file(argv[1],entp->d_name,s.st_size);
        }
      }
    }
    closedir(watchDIR);
  }
  return 0;
}
