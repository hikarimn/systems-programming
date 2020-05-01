#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <sys/types.h>
#include <sys/wait.h>

// https://airtower.wordpress.com/2010/06/16/catch-sigterm-exit-gracefully/
volatile sig_atomic_t done = 0;

void term(int signum){
    done = 1;
}

int main(int argc, char **argv){

    if( argc >= 0 ){
        printf("The arguments supplied are:\n");
        printf("%ld %ld\n", (long)getpid(), (long)getppid());
        
        if(strcmp(argv[0], "./fw" ) == 0 && strcmp(argv[1], "start") == 0){
            printf("%s\n", argv[0]); 
        } else {
            printf("%s\n", argv[0]); 
        }

    } else {
        printf("argument list is empty.\n");
    }
    printf("fwd\n"); 

    return 0;
}
