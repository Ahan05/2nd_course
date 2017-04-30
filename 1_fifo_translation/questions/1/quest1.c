#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
int main (int argc, char **argv)
{
    struct stat buf1, buf2;
    int file1, file2;

    if (argc != 3)
    {
        printf("Need more files");
        exit(1);
    }
    file1 = open(argv[1], O_RDONLY);
    file2 = open(argv[2], O_RDONLY);

    fstat(file1, &buf1);
    fstat(file2, &buf2);

    if ( (buf1.st_ino == buf2.st_ino) && (buf1.st_dev == buf2.st_dev) )
    {
        printf("The same file\n" );
    }

    close(file1);
    close(file2);

}
