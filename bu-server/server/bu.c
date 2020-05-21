#include "csapp.h"

/* Locations of important files */
const char* pidFileLoc = "/run/bu.pid";
const char* confFileLoc = "/etc/bu.conf";
const char* logFileLoc = "/var/log/bu.log";
FILE *logFile;

/* Global SIGHUP received flag */
static volatile sig_atomic_t hupReceived = 0;

void sighupHandler(int sig)
{
    hupReceived = 1;
}

void serve_request(int connfd){
    char buffer[MAXLINE], fileName[MAXLINE], filePath[MAXLINE];
    char *fileBuf;
    int size;
    rio_t rio;
    int fd;
    struct stat s;

    strcpy(filePath, "/var/bu/");
    /* Read request */
    Rio_readinitb(&rio, connfd);
    if (!Rio_readlineb(&rio, buffer, MAXLINE))
        return;
    if (strcmp(buffer, "upload\n") == 0) {
        Rio_readlineb(&rio, fileName, MAXLINE);
        Rio_readlineb(&rio, buffer, MAXLINE);
        printf("Request to upload %s\n",fileName);
        strcat(filePath, fileName);
        printf("FilePath: %s\n",filePath);
        sscanf(buffer,"%d",&size);
        fileBuf = Malloc(size);
        if(fileBuf != NULL) {
           Rio_readnb(&rio,fileBuf,size);
           fd = Open(filePath,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
           Write(fd,fileBuf,size);
           Close(fd);
           Free(fileBuf);
        }
        strcpy(buffer,"stored\n");
        Rio_writen(connfd,buffer,strlen(buffer));
    } else if(strcmp(buffer,"status\n") == 0) {
        Rio_readlineb(&rio, fileName, MAXLINE);
        printf("Request to stat %s\n",fileName);
        strcat(filePath, fileName);
        printf("FilePath: %s\n",filePath);
        filePath[strlen(filePath)-1] = '\0';
        if(stat(filePath,&s) == 0)
          sprintf(buffer,"%d\n",s.st_mtime);
        else
          strcpy(buffer,"0\n");
        Rio_writen(connfd,buffer,strlen(buffer));
    }
        
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

/*
 * readConfig - read the configuration file
 */
int readConfig(char *port, char *path) {
  FILE *config;

  config = fopen(confFileLoc,"r");
  if(config == NULL)
    return -1;
  fscanf(config,"%s",port);
  fscanf(config,"%s",path);
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


int main(int argc, char **argv){
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    char myPort[MAXLINE], myHome[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
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
    if(readConfig(myPort,myHome) != 0) {
        fprintf(stderr, "Failed to read config file\n");
        exit(1);
    }

    /* Open log file */
    if(logOpen() != 0) {
        fprintf(stderr, "Could not open log file\n");
        exit(1);
    }

    /* Switch to the background */
	if(becomeDaemon(myHome) != 0) {
        fprintf(stderr, "Failed becomeDaemon\n");
	    exit(1);
    }

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
