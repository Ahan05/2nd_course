#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/file.h>
#define MSG_SIZE 64
#define FIFO_NAME_SIZE 20

int main(int argc, char** argv){
char general_name[] = "general.fifo";

int in = open(argv[1], O_RDONLY);
int general_fifo, privat_fifo,privat_fifo_rd, dump,check;
char buf[MSG_SIZE], privat_fifo_name[FIFO_NAME_SIZE]={};
int bytes_read = -1;
int retv = 0;

if (argc != 2)
    {
        fprintf(stderr, "No file\n");
        return 0;
    }
//----------------------------------
// opening genral fifo and reading private fifo name from it
retv = mkfifo(general_name,  0666);

if (retv == -1)
    {
        if (errno != EEXIST)
          {
              perror (argv[0]);
              return 0;
          }
    }

general_fifo = open(general_name, O_RDONLY);
dump = open(general_name, O_WRONLY);
//*
bytes_read = read(general_fifo, privat_fifo_name,FIFO_NAME_SIZE);

if (bytes_read != FIFO_NAME_SIZE)
    {
        fprintf(stderr, "NO reader is availible\n");
        return 0;
    }
//----------------------------------
//creating private fifo and writing into it
if ( (mkfifo(privat_fifo_name, 0666) < 0) && (errno != EEXIST) )
    {
        fprintf(stderr, "Can not create privat fifo\n");
    }

privat_fifo_rd = open(privat_fifo_name, O_RDONLY | O_NONBLOCK);
privat_fifo = open(privat_fifo_name, O_WRONLY);

if (privat_fifo == -1)
{
    perror(argv[1]);
    close(in);
    close(dump);
    close(privat_fifo_rd);
    exit(0);
}

close(privat_fifo_rd);
//*
while (bytes_read != 0)
    {
        bytes_read = read(in, &buf, MSG_SIZE);
        check = write(privat_fifo, &buf, bytes_read);
        if (errno == EPIPE)
        {
            close(privat_fifo);
            close(in);
            close(dump);
            perror(argv[1]);
            exit(0);
        }
    }
close(privat_fifo);
close(in);
close(dump);
return 0;
}
