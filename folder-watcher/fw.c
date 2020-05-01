#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>

#define DATA_SIZE 128
#define MAXARGS 128  
extern char *environ[];

int mysystem(char** command){
    if(strcmp(command[0], "./fw" ) == 0 && strcmp(command[1], "start") == 0){
        pid_t pid;
        pid = fork();
        if(pid < 0){
            fprintf(stderr, "fork error.\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0){
            printf("START\n");
            // printf("%s\n", command[2]); 

            if (execve("./fwd", &command[1], environ) < 0){
                fprintf(stderr, "execve sh error.\n");
                exit(0);
            }
            _exit(EXIT_SUCCESS);
        } else {
            int status;
            (void)waitpid(pid, &status, 0);
        }
    
        return EXIT_SUCCESS;

    } else if (strcmp(command[0], "./fw" ) == 0 && strcmp(command[1], "stop") == 0){
            printf("STOP\n");
            // printf("%s\n", command[1]); 

            char *new_pid = NULL;
            size_t n;
            FILE *fp;
            if ((fp = fopen("process_id.txt", "r")) == NULL) {
                printf("Error! opening file");
                exit(1);
            }
            getline(&new_pid, &n, fp);
            fclose(fp);

            printf("kill -SIGKILL %s\n",new_pid);
            int new_pid_raw = atoi(new_pid);
            int new_pid_int = (int)(new_pid_raw);
            
            kill(new_pid_int, SIGTERM);
            free(new_pid);
    }
    exit(0);
    return 0;
}
    
int main(int argc, char **argv){
    if( argc >= 2 ){
        mysystem(argv);
    } else {
        printf("argument list is empty.\n");
    }
    return 0;
}
