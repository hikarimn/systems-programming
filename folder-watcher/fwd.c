#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


volatile sig_atomic_t done = 0;
extern char *environ[];

char * concatenate(const char *a, const char *b, const char *d){
    size_t len = strlen(a) + strlen(b) + strlen(d);
    char* str = malloc(len+1);

    strcpy(str, a);
    strcat(str, b);
    strcat(str, d);

    return str;
}

int main(int argc, char **argv){

    if (argv[1] !=  NULL){
        // start <path>
        printf("%s\n", argv[1]); 
        int loop = 0;
        char * mypid = malloc(6);   
        sprintf(mypid, "%d", getpid());
        printf( "%s\n", mypid);

        FILE * fp;
        fp = fopen("process_id.txt", "w");
        if (fp == NULL){
            printf("Unable to create file.\n");
            exit(0);
        }
        fprintf(fp, "%s\n", mypid);
        fclose(fp);
        free(mypid);

        while (!done){
            // printf("%ld %ld\n", (long)getpid(), (long)getppid());
            
            // Recording time to log file
            FILE * fp;
            fp = fopen("changes.log", "a");
            if (fp == NULL){
                printf("Unable to create log file.\n");
                exit(0);
            }
            
            struct tm *tm_now;

            time_t time_now = time (0);
            tm_now = gmtime (&time_now);
            struct tm dt_now;
            dt_now = *tm_now;

            char time_buff[100];
            strftime (time_buff, 100, "%Y-%m-%d %H:%M:%S.000", tm_now);
          
            fprintf(fp, "Time: %s\n", time_buff);
            fclose(fp);

            // Iterating through files in directory
            DIR *folder;
            struct dirent *entry;

            folder = opendir(argv[1]);
            if(folder == NULL) {
                perror("Unable to read directory");
                return(1);
            }

            while( (entry=readdir(folder)) ) {

                char* path_to_file = concatenate(argv[1], "/", entry->d_name);

                //Observing changes on files
                struct tm dt_modified;
                struct tm *tm_modified;
                struct stat attr;
                double seconds;
                time_t time_modified;

                if (stat(path_to_file, &attr) == 0){
                    time_modified = attr.st_mtime;
                    
                    
                    seconds = difftime(time_now, time_modified);
                    printf("Difference = %f\n", seconds);
                    if(seconds < 5){                    
                        tm_modified = gmtime(&attr.st_mtime);
                        dt_modified = *(gmtime(&attr.st_mtime));

                        FILE * fp;
                        fp = fopen("changes.log", "a");
                        if (fp == NULL){
                            printf("Unable to create log file.\n");
                            exit(0);
                        }

                        char time_modified_buff[100];
                        strftime (time_modified_buff, 100, "%Y-%m-%d %H:%M:%S.000", tm_modified);

                        fprintf(fp, "File Name: %s\n", entry->d_name);
                        fprintf(fp, "Modified at: %s\n", time_modified_buff);

                        fclose(fp);
                    }

                } else {
                    printf("Unable to get file properties.\n");
                    printf("Please check whether '%s' file exists.\n", path_to_file);
                }
            }
            closedir(folder);

            int t = sleep(5);
            while (t > 0) {
                printf("Loop run was interrupted with %d "
                       "sec to go, finishing...\n", t);
                t = sleep(t);
            }
            printf("Finished loop run %d.\n", loop++);
        }
 
        printf("done.\n");

    } else {
        printf("argument list is empty.\n");
    }

    return 0;
}

