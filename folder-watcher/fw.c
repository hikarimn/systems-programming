#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <sys/types.h>
#include <sys/wait.h>

#define DATA_SIZE 128
#define MAXARGS 128  
extern char *environ[];
    
int main(int argc, char **argv){
    if( argc >= 2 ){
        if(strcmp(argv[0], "./fw" ) == 0 && strcmp(argv[1], "start") == 0){
            printf("START\n");
            printf("%ld %ld\n", (long)getpid(), (long)getppid());
            printf("%s\n", argv[2]); 

            int pid = getpid();
            char * mypid = malloc(6);   // ex. 34567
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

            if (execve("./fwd", &argv[1], environ) < 0){
                fprintf(stderr, "execve sh error.\n");
                exit(0);
            }
        } else if (strcmp(argv[0], "./fw" ) == 0 && strcmp(argv[1], "stop") == 0){
            printf("STOP\n");
            // printf("%ld %ld\n", (long)getpid(), (long)getppid());
            printf("%s\n", argv[1]); 

            char *new_pid = NULL;
            size_t n;
            FILE *fp;
            if ((fp = fopen("process_id.txt", "r")) == NULL) {
                printf("Error! opening file");
                // Program exits if file pointer returns NULL.
                exit(1);
            }
            // fscanf(fp, "%[^\n]", new_pid);
            getline(&new_pid, &n, fp);
            printf("Data from the file:%s\n", new_pid);
            fclose(fp);

            printf("kill -SIGKILL %s\n",new_pid);
            free(new_pid);
            if (execve("kill -SIGKILL %s", &new_pid, environ) < 0){
                fprintf(stderr, "execve sh error.\n");
                exit(0);
            }
            // if (execve("./fwd", &argv[1], environ) < 0){
            //     fprintf(stderr, "execve sh error.\n");
            //     exit(0);
            // }
        }
    } else {
        printf("argument list is empty.\n");
    }
    return 0;
}
