#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAXARGS 128
extern char *environ[];
    
int main(int argc, char **argv){
    if( argc >= 2 ){
        if(strcmp(argv[0], "./fw" ) == 0 && strcmp(argv[1], "start") == 0){
            printf("START\n");
            printf("%ld %ld\n", (long)getpid(), (long)getppid());
            printf("%s\n", argv[2]); 
            if (execve("./fwd", &argv[1], environ) < 0){
                fprintf(stderr, "execve sh error.\n");
                exit(0);
            }
            // return mysystem(argv[2]);
        } else if (strcmp(argv[0], "./fw" ) == 0 && strcmp(argv[1], "stop") == 0){
            printf("STOP\n");
            printf("%ld %ld\n", (long)getpid(), (long)getppid());
            printf("%s\n", argv[1]); 
            if (execve("./fwd", &argv[1], environ) < 0){
                fprintf(stderr, "execve sh error.\n");
                exit(0);
            }
            // return mysystem(argv[1]);
        }
    } else {
        printf("argument list is empty.\n");
    }
    return 0;
}
