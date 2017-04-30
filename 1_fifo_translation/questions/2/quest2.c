#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
int main (int argc, char **argv)
{
    int fd;
    char buf[3];
    mkfifo( "fifo", 666 );
    fd = open("fifo", O_RDWR);
    write(fd, "333", 3);

    fd = open("fifo", O_RDONLY | O_NDELAY);

    write( 1, buf, read(fd, buf, 3) );
    close(fd);
}
