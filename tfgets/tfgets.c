#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define SLEEP 5

sigjmp_buf tfgets_buf; // sigjpm_buf: store state information about the set of blocked signal

typedef void handler_t(int);

void unix_error(char *msg){
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

// /** Modified Signal function copied from csapp.c. This differs from
//     the original Signal in that the option to restart system calls
//     is not automatically turned on. Instead, the caller has to say whether
//     or not to restart system calls. **/
handler_t *Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);  /* Block sigs of type being handled */
    
    action.sa_flags = SA_RESTART;

    if (sigaction(signum, &action, &old_action) < 0)
      unix_error("Signal error");
    return (old_action.sa_handler);
}

void sig_handler(int something){
    siglongjmp(tfgets_buf, 1);
}

char* tfgets(char* ch, int size, FILE *file){
    /* transfers control directly from one function to another currently executing 
function. The setjmp function saves the current calling environment in the env buffer,
for later use by longjmp, and returns a 0.
*/
    if(sigsetjmp(tfgets_buf, 1) == 0){
        Signal(SIGALRM, sig_handler);
        alarm(SLEEP);
        return fgets(ch, size, file);
    } else {
        return NULL;
    }
}

int main(int argc, char* argv[]){
    char buffer[256];
    printf("Type your name within %d seconds:\n", SLEEP);
    if(tfgets(buffer, 256, stdin))
        printf("Hello %s", buffer);
    else
        printf("Timeout");
}
