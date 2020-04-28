#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAXARGS 128

char * concatenate(const char *a, const char *b, const char *d){
    size_t len = strlen(a) + strlen(b) + strlen(d);
    char* str = malloc(len+1);

    strcpy(str, a);
    strcat(str, b);
    strcat(str, d);

    return str;
}

void unix_error(char *msg){
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

int main(int argc, const char * argv[]){
    FILE* input;
    char buffer[62];
    const char str0[] = "";

    char str1[10];
    char str2[30];
   
    input = fopen("names.txt", "r");

    while(!feof(input)){

        fscanf(input, "%s", buffer);

        strcpy(str1, "Folders/");
        strcpy(str2, buffer);
        char* str = concatenate(str0, str1, str2);
        // printf("%s\n", str);

        struct stat sb;
        if (stat(str, &sb) == 0 && S_ISDIR(sb.st_mode)) {
            // do nothing
        } else {
            // If that student does not have a directory, use mkdir() to create one for them.
            int ret;
            ret = mkdir(str, S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH);
            if (ret != 0){
                unix_error("Mkdir error");
            }
        }
        free(str);
    }
    return 0;
}
