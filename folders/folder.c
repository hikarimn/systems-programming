#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAXARGS 128

extern char *environ[];

int mysystem(char* command){
    pid_t pid;
    pid = fork();

    if(pid < 0){
        fprintf(stderr, "execve sh error.\n");
        exit(0);
    } else if (pid == 0){
        char *argv[MAXARGS];
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = command;
        argv[3] = NULL;
        if (execve("/bin/sh", argv, environ) < 0){
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

}

char * concatenate(const char *a, const char *b, const char *d){
    size_t len = strlen(a) + strlen(b) + strlen(d);
    char* str = malloc(len+1);

    strcpy(str, a);
    strcat(str, b);
    strcat(str, d);

    return str;
}

int main(int argc, const char * argv[]){
    FILE* input;
    char buffer[62];
    const char str0[] = "mkdir -p ";
    char str1[10];
    char str2[30];
    printf("Making a directory: %d\n\n", mysystem("mkdir Folders"));
   
    input = fopen("names.txt", "r");

    while(!feof(input)){

        // mkdir Folders/Bock
        fscanf(input, "%s", buffer);

        strcpy(str1, "Folders/");
        strcpy(str2, buffer);
        // printf("%s/n", buffer);
        char* str = concatenate(str0, str1, str2);
        // printf("%s\n", str);
        printf("Making folder: %d\n", mysystem(str));
        free(str);
        // fprintf(stdout, "\n");
        // print(mysystem(""));
    }
    return 0;
}
