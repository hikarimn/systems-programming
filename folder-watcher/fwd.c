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

int printFileProperties(struct stat stats){
    struct tm dt;

    // // File permissions
    // printf("\nFile access: ");
    // if (stats.st_mode & R_OK)
    //     printf("read ");
    // if (stats.st_mode & W_OK)
    //     printf("write ");
    // if (stats.st_mode & X_OK)
    //     printf("execute");

    // File size
    // printf("\nFile size: %lld", stats.st_size);

    // Get file creation time in seconds and 
    // convert seconds to date and time format
    // dt = *(gmtime(&stats.st_ctime));
    // printf("\nCreated on: %d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, 
    //                                           dt.tm_hour, dt.tm_min, dt.tm_sec);

    // File modification time
    dt = *(gmtime(&stats.st_mtime));
    printf("\nModified on: %d-%d-%d %d:%d:%d\n", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, 
                                              dt.tm_hour, dt.tm_min, dt.tm_sec);
    return 0;
}

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
        while (!done){
            printf("%ld %ld\n", (long)getpid(), (long)getppid());
            
            // Recording time!
            FILE * fp;
            fp = fopen("changes.txt", "w");
            if (fp == NULL){
                printf("Unable to create log file.\n");
                exit(0);
            }
            char buff[20];
            struct tm *sTm;

            time_t time_now = time (0);
            sTm = gmtime (&time_now);
            struct tm dt_now;
            dt_now = *sTm;
            printf("\nCurrent Time: %d-%d-%d %d:%d:%d\n", dt_now.tm_mday, dt_now.tm_mon, dt_now.tm_year + 1900, 
                                              dt_now.tm_hour, dt_now.tm_min, dt_now.tm_sec);
            // printf("row time: %s\n", sTm);

            // strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
            // printf ("%s %s\n", buff, "Event occurred now");
            // fprintf (fp,"%s %s\n", buff, "Event occurred now");
            // fprintf(fp, "print this");

            // Iterating through files!
            DIR *folder;
            struct dirent *entry;
            int files = 0;

            folder = opendir(argv[1]);
            if(folder == NULL) {
                perror("Unable to read directory");
                return(1);
            }

            while( (entry=readdir(folder)) ) {
                files++;
                printf("File %3d: %s\n", files, entry->d_name);

                char* path_to_file = concatenate(argv[1], "/", entry->d_name);
                printf( "%s\n", path_to_file);

                //Observing changes!
                struct tm dt_modified;
                struct stat attr;
                double seconds;
                time_t time_modified;

                if (stat(path_to_file, &attr) == 0){
                    // printFileProperties(stats);
                    time_modified = attr.st_mtime;
                    dt_modified = *(gmtime(&attr.st_mtime));
                    printf("\nModified on: %d-%d-%d %d:%d:%d\n", dt_modified.tm_mday, dt_modified.tm_mon, dt_modified.tm_year + 1900, 
                                              dt_modified.tm_hour, dt_modified.tm_min, dt_modified.tm_sec);

                    seconds = difftime(time_now, time_modified);
                    printf("Difference = %f\n", seconds);

                } else {
                    printf("Unable to get file properties.\n");
                    printf("Please check whether '%s' file exists.\n", path_to_file);
                }
            }

            closedir(folder);

            int t = sleep(5);
            /* sleep returns the number of seconds left if
             * interrupted */
            while (t > 0) {
                printf("Loop run was interrupted with %d "
                       "sec to go, finishing...\n", t);
                t = sleep(t);
            }
            printf("Finished loop run %d.\n", loop++);
            fclose(fp);
        }
 
        printf("done.\n");

    } else {
        printf("argument list is empty.\n");
    }
    printf("fwd\n"); 
    

    return 0;
}

