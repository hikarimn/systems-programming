#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h> 

int main(int argc, char *argv[]){
    int fd;
    struct stat st;
    char *bufp;

    fd = open("hello.txt", O_RDONLY);

    if (fd == -1)
        printf("Error opening file for writing");
    if (fstat(fd, &st) == -1)
        printf("Error getting the file size");
    if (st.st_size == 0)
        printf("Error: File is empty");
	if (mmap(bufp, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0) == NULL)
        printf("Mmap error");
    
    *bufp = 'J';
    munmap(bufp, st.st_size);

    exit(0);
}


