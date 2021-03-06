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

/* Global SIGHUP received flag */
static volatile sig_atomic_t hupReceived = 0;

void handleTERM(int x) {
  run = 0;
}

/*
 * becomeDaemon - convert this process to a daemon
 */
int becomeDaemon(char *home)
{
  int fd;
  pid_t pid;
  FILE* pidFile;

  if((pid = Fork()) != 0) { /* Become background process */
    exit(0);  /* Original parent terminates */
  }

  if(setsid() == -1) /* Become leader of new session */
    return -1;

  if((pid = Fork()) != 0) { /* Ensure we are not session leader */
	/** Prepare pid file and terminate **/
	pidFile = fopen(pidFileLoc,"w");
	fprintf(pidFile,"%d",pid);
	fclose(pidFile);
    exit(0);
  }

  chdir(home); /* Change to home directory */

  Close(STDIN_FILENO); /* Reopen standard fd's to /dev/null */

  fd = Open("/dev/null", O_RDWR, 0);

  if (fd != STDIN_FILENO)         /* 'fd' should be 0 */
    return -1;
  if (Dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
    return -1;
  if (Dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
    return -1;

  return 0;
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

void sighupHandler(int sig)
{
    hupReceived = 1;
}

int main(int argc, char **argv){
  struct sigaction action, old_action;    
  char myPort[MAXLINE], myPath[MAXLINE], myAddress[MAXLINE];
  time_t timer;
  int buModTime;
  DIR* watchDIR;
  struct dirent* entp;
  struct stat s;
  char path[1024];
  struct sigaction sa;

/** Install SIGHUP handler **/
	  sigemptyset(&sa.sa_mask);
	  sa.sa_flags = SA_RESTART;
	  sa.sa_handler = sighupHandler;
	  if (sigaction(SIGHUP, &sa, NULL) == -1) {
        fprintf(stderr, "Failed to install SIGHUP handler\n");
        exit(1);
      }

    /* Read configuration information */
    if(readConfig(myPath,myAddress,myPort) != 0) {
        fprintf(stderr, "Failed to read config file\n");
        exit(1);
    }

    /* Open log file */
    if(logOpen() != 0) {
        fprintf(stderr, "Could not open log file\n");
        exit(1);
    }

    /* Switch to the background */
	if(becomeDaemon(myPath) != 0) {
        fprintf(stderr, "Failed becomeDaemon\n");
	    exit(1);
    }

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
