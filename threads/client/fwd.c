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

static sem_t mutex;
/* Global SIGHUP received flag */
static volatile sig_atomic_t hupReceived = 0;
/* Global pointer for a linked list */
struct node* currentNode;

void handleTERM(int x) {
  run = 0;
}

static void init_echo_cnt(void){
        Sem_init(&mutex, 0, 1);
        // byte_cnt = 0;
}

struct node {
  char* text;
  struct node *next;
};

/* Make a new node to hold a copy of txt. */
struct node *make_node(char* txt) {
  struct node *new_node;
  new_node = (struct node*) malloc(sizeof(struct node));
  new_node->text = (char*) malloc(strlen(txt));
  strcpy(new_node->text,txt);
  new_node->next = NULL;
  return new_node;
}

/* Attach a new node to the end of a chain of nodes. */
void append_node(struct node *chain,char* txt) {
  struct node *current = chain;
  while(current->next != NULL)
    current = current->next;
  current->next = make_node(txt);
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

/** You had
void upload_file(char* dir,char* fileName,int filesize) {
I fixed this. **/
void upload_file(char* folder,char* dir,char* fileName,int filesize) {
    int srcfd, clientfd;
    char *host, *port, *cmd, *srcp;
    char buffer[256];
    rio_t rio;

    /** This code will break as soon as the user changes the hostname or the
        port listed in the configuration file. You should either make those two
        strings be globals and read them once in the readConfig function, or
        arrange to pass the correct values of host and port as parameters to this
        function. **/
    host = "127.0.0.1";
    port = "8000";
    cmd = "upload\n";
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    Rio_writen(clientfd, cmd, strlen(cmd));
    /** You need to insert some code here that sends the folder name to the server. **/
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

/** You had
void check_file(char* dir,char* fileName,int time,int size) {
I fixed this. **/
void check_file(char* dir,char* path,char* fileName,int time,int size) {
    int clientfd;
    char *host, *port, *cmd, *srcp;
    char buffer[256];
    rio_t rio;
    int serverTime;

    /** Same comment as above. **/
    host = "127.0.0.1";
    port = "8000";
    cmd = "status\n";
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    Rio_writen(clientfd, cmd, strlen(cmd));
    /** You need to insert some code here that sends the dir name to the server. **/
    strcpy(buffer,fileName);
    strcat(buffer,"\n");
    Rio_writen(clientfd, buffer, strlen(buffer));

	Rio_readlineb(&rio, buffer, 256);
	Close(clientfd);
    sscanf(buffer,"%d",&serverTime);
    if(serverTime < time)
      upload_file(dir,fileName,size); /** Should be upload_file(dir,path,fileName,size);
}



/*
readConfig - read the configuration file
A line for each directory fwd has to watch.
On that line will be the path to the directory in question, a space, and a name to use for the folder on the bu server.
A blank line at the end end of the list of directories.
The address of the bu server.
The port number of the bu server.
 */
/*
 * readConfig - read the configuration file
 */
int *readConfig(char* address, char* port,struct node* *ptr) {
  /* Code to open config file goes here. */
  FILE *config;
  config = fopen(confFileLoc,"r");
  if(config == NULL)
    printf("unable to read the config file");
    return 1;

  char buffer[256];
  char folderNameBuf[256];
  /* Code to read a path and a folder name into buffer goes here. */
  fscanf(config, "%s %s", buffer, folderNameBuf);
  strcat(buffer, " ");
  strcat(buffer, folderNameBuf);
  *ptr = make_node(buffer);
  while(1) {
    /* Code to read a path into buffer goes here. */
    fscanf(config, "%s %s", buffer, folderNameBuf);
    strcat(buffer, " ");
    strcat(buffer, folderNameBuf);
    if(strlen(buffer) == 0) /* Stop when you hit a blank line. */
       break;
    append_node(*ptr,buffer);
  }

  /* Code to read host and port goes here. */
  fscanf(config,"%s",address);
  fscanf(config,"%s",port);
  fclose(config);
  return 0;
}

int readConfigBefore(char *path, char *address,char *port) {
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
  P(&mutex);
  if(logFile != NULL) {
   fprintf(logFile,"%s\n",msg);
   fflush(logFile);
  }
  V(&mutex);
}

/*
 * logClose - close the log file
 */
void logClose() {
  if(logFile != NULL)
    fclose(logFile);
}

void sighupHandler(int sig){
    hupReceived = 1;
}

void process(struct node *pointer){
  struct sigaction action, old_action;
  DIR* watchDIR;
  struct dirent* entp;
  time_t timer;
  char path[1024];
  struct stat s;

  char * pathName = strtok(pointer->text," ");
  char * folderName = strtok(pointer->text," ");

 /** The code starting here... **/
  /* Open log file */
    if(logOpen() != 0) {
        fprintf(stderr, "Could not open log file\n");
        exit(1);
    }

    /* Switch to the background */
  if(becomeDaemon(pathName) != 0) {
      logMessage("Failed becomeDaemon\n");
	    exit(1);
    }

  /* Install the handler for SIGTERM */
  action.sa_handler = handleTERM;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGTERM, &action, &old_action);
 /** ..down to here needs to go in main(), not here. This code will run for every
     thread, and this code needs to run just once for the entire application. **/

  /* Run the initial scan. */
  watchDIR = opendir(folderName); /** Replace folderName with pathName here. **/
  while((entp = readdir(watchDIR))!=NULL) {
    strcpy(path,folderName); /** Replace folderName with pathName here. **/
	strcat(path,"/");
	strcat(path,entp->d_name);
	stat(path,&s);
	/* Only pay attention to files, and ignore directories. */
	if(S_ISREG(s.st_mode))
	  check_file(folderName,entp->d_name,s.st_mtime,s.st_size);
  }
  closedir(watchDIR);

  /* Start watching for changes. **/
  run = 1;
  while(run == 1) {
    /* Note the time and go to sleep. */
    time(&timer);
    sleep(60);
    /* Scan the watched directory for changes. */
    watchDIR = opendir(folderName); /** Replace folderName with pathName here. **/
    while((entp = readdir(watchDIR))!=NULL) {
      strcpy(path,folderName); /** Replace folderName with pathName here. **/
      strcat(path,"/");
      strcat(path,entp->d_name);
      stat(path,&s);
      /* Only pay attention to files, and ignore directories. */
      if(S_ISREG(s.st_mode)) {
        /* Upload recently modified files. */
        if(difftime(s.st_mtime,timer) > 0) {
          upload_file(folderName,entp->d_name,s.st_size); /** This should be upload_file(folderName,pathName,entp->d_name,s.st_size); **/
        }
      }
    }
    closedir(watchDIR);
  }
  return NULL;

}

void *thread(void *vargp){
  Pthread_detach(pthread_self());
  Free(vargp);
  process(currentNode);
  return NULL;
}

int main(int argc, char **argv){
  char myPort[MAXLINE], myPath[MAXLINE], myAddress[MAXLINE];
  struct sigaction sa;
  struct node *pointer;
  pthread_t tid;

/** Install SIGHUP handler **/
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sighupHandler;
	if (sigaction(SIGHUP, &sa, NULL) == -1) {
      fprintf(stderr, "Failed to install SIGHUP handler\n");
      exit(1);
  }

    // /* Read configuration information */
  /** Change pointer to &pointer in the statement below. **/
  if(readConfig(myAddress, myPort, pointer) != 0) {
    fprintf(stderr, "Failed to read config file\n");
    exit(1);
  }

  static pthread_once_t once = PTHREAD_ONCE_INIT;
  Pthread_once(&once, init_echo_cnt);

  currentNode = pointer;

  while(currentNode->next != NULL) { /** Should be while(currentNode != NULL) **/
    Pthread_create(&tid, NULL, thread, NULL); /** The last parameter here should be currentNode, not NULL **/
    /** Calling Pthread_join causes your program to block until
        the thread you just started finishes. Since the thread you
        just started is designed to run forever, you will stay blocked
        here forever, and will never get a chance to launch a second or
        later threads. The fix for this is to move this next statement
        after the end of the loop. **/
    Pthread_join(tid, NULL);
    currentNode = currentNode->next;
  }

}
