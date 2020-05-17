/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh 
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 *
 * Updated 04/2020 greggj
 *   - Added becomeDaemon(), changed printf calls to write to log file,
 *     added code to read configuration information from a config file,
 *     added code to handle SIGHUP.
 */
#include "csapp.h"

int becomeDaemon(char *home);
int readConfig(char *port,char *home);
int logOpen();
void logMessage(char *msg);
void logClose();
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/* Locations of important files */
const char* pidFileLoc = "/run/tiny.pid";
const char* confFileLoc = "/etc/tiny.conf";
const char* logFileLoc = "/var/log/tiny.log";
FILE *logFile;
    
/* Global SIGHUP received flag */
static volatile sig_atomic_t hupReceived = 0;

void sighupHandler(int sig)
{
    hupReceived = 1;
}

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    char myPort[MAXLINE], myHome[MAXLINE];
    char logBuffer[MAXLINE];
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
  
    /* Open socket and serve requests */     
    listenfd = Open_listenfd(myPort);
    while (1) {
      if(hupReceived != 0) {
        logClose();
        logOpen();
        hupReceived = 0;
      }
	  clientlen = sizeof(clientaddr);
	  connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
      Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
      sprintf(logBuffer,"Accepted connection from (%s, %s)", hostname, port);
      logMessage(logBuffer);
	  doit(connfd);
	  Close(connfd);
    }
}

/*
 * doit - handle one HTTP request/response transaction
 */
void doit(int fd) 
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE], logBuffer[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  
        return;
    sprintf(logBuffer,"%s", buf);
    logMessage(logBuffer);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
      clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
      return;
    }
    read_requesthdrs(&rio);

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
	  clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
	  return;
    }

    if (is_static) { /* Serve static content */          
	  if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
	    clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
	    return;
	  }
	  serve_static(fd, filename, sbuf.st_size);
    }
    else { /* Serve dynamic content */
	  if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
	    clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
	    return;
	}
	serve_dynamic(fd, filename, cgiargs);
    }
}

/*
 * read_requesthdrs - read HTTP request headers
 */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE], logBuffer[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    sprintf(logBuffer,"%s", buf);
	logMessage(logBuffer);
    while(strcmp(buf, "\r\n")) {
	  Rio_readlineb(rp, buf, MAXLINE);
	  sprintf(logBuffer,"%s", buf);
	  logMessage(logBuffer);
    }
    return;
}

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {  /* Static content */
	  strcpy(cgiargs, "");
	  strcpy(filename, ".");
	  strcat(filename, uri);
	  if (uri[strlen(uri)-1] == '/')
	    strcat(filename, "home.html");
	  return 1;
    } else {  /* Dynamic content */
	  ptr = index(uri, '?');
	  if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	  } else 
	    strcpy(cgiargs, "");
	  strcpy(filename, ".");
	  strcat(filename, uri);
	  return 0;
    }
}

/*
 * serve_static - copy a file back to the client 
 */
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", filesize);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
    Rio_writen(fd, buf, strlen(buf));

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
	  strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	  strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
	  strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	  strcpy(filetype, "image/jpeg");
    else
	  strcpy(filetype, "text/plain");
}  

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
  
    if (Fork() == 0) { /* Child */
	  /* Real server would set all CGI vars here */
	  setenv("QUERY_STRING", cgiargs, 1);
	  Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */
	  Execve(filename, emptylist, environ); /* Run CGI program */
    }
    Wait(NULL); /* Parent waits for and reaps child */
}

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
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
int readConfig(char *port,char *home) {
  FILE *config;

  config = fopen(confFileLoc,"r");
  if(config == NULL)
    return -1;
  fscanf(config,"%s",port);
  fscanf(config,"%s",home);
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

