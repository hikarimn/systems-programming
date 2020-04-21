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

    fd = open("hello.txt", O_RDWR, 0);
    fstat(fd, &st);
    bufp = mmap(NULL, st.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
    *bufp = 'J';
    munmap(bufp, st.st_size);

    exit(0);
}
