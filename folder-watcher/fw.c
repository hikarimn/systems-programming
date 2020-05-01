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
        pid = getpid();
        if(pid < 0){
            fprintf(stderr, "execve sh error.\n");
            exit(0);
        } else if (pid == 0){
            printf("START\n");
            // printf("%ld %ld\n", (long)getpid(), (long)getppid());
            printf("%s\n", command[2]); 

            // int pid = getpid();
            char * mypid = malloc(6);   
            sprintf(mypid, "%d", pid);

            FILE * fp;
            fp = fopen("process_id.txt", "w");
            if (fp == NULL){
                printf("Unable to create file.\n");
                exit(0);
            }
            fputs(mypid, fp);
            fclose(fp);
            free(mypid);

            if (execve("./fwd", &command[1], environ) < 0){
                fprintf(stderr, "execve sh error.\n");
                exit(0);
            }
        }
        int status;
        if (waitpid(pid, &status, 0) < 0){
            fprintf(stderr, "waitpid error.\n");
            exit(0);
        } 
        if(WIFEXITED(status))
            return WEXITSTATUS(status);
        if(WIFSIGNALED(status))
            return WTERMSIG(status);
        return status;

    } else if (strcmp(command[0], "./fw" ) == 0 && strcmp(command[1], "stop") == 0){
            printf("STOP\n");
            printf("%s\n", command[1]); 

            char *new_pid = NULL;
            size_t n;
            FILE *fp;
            if ((fp = fopen("process_id.txt", "r")) == NULL) {
                printf("Error! opening file");
                exit(1);
            }
            getline(&new_pid, &n, fp);
            printf("Data from the file:%s\n", new_pid);
            fclose(fp);

            printf("kill -SIGKILL %s\n",new_pid);
            int new_pid_raw = atoi(new_pid);
            int new_pid_int = (int)(new_pid_raw);
            
            kill(new_pid_int, SIGTERM);
            free(new_pid);
        }
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
