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
int general_fifo, privat_fifo,privat_fifo_wr,privat_fifo_rd;
char buf[MSG_SIZE], privat_fifo_name[FIFO_NAME_SIZE];
int bytes_read = -1;
int retv = 0;
pid_t pid;

//----------------------------------
//creating general fifo
retv = mkfifo(general_name,  0777);
general_fifo = open(general_name, O_WRONLY);

if (general_fifo == -1)
{
    perror(argv[1]);
}
//----------------------------------
// creating private fifo
pid = getpid();

snprintf(privat_fifo_name,FIFO_NAME_SIZE,"%d", pid);

if ( (mkfifo(privat_fifo_name, 0666) < 0) && (errno != EEXIST) )
    {
        fprintf(stderr, "Can not create privat fifo\n");
    }

privat_fifo_rd = open(privat_fifo_name, O_RDONLY | O_NONBLOCK);
privat_fifo_wr = open(privat_fifo_name, O_WRONLY);

close(privat_fifo_rd);

privat_fifo = open(privat_fifo_name, O_RDONLY);
if (privat_fifo == -1)
{
    perror(argv[1]);
    close(privat_fifo);
    exit(0);
}
close(privat_fifo_wr);
//----------------------------------
//*
write(general_fifo, privat_fifo_name, FIFO_NAME_SIZE); // sending private fifo name
// reading from fifo
sleep(3);
//*
while( bytes_read != 0)
    {
        bytes_read = read(privat_fifo, buf, MSG_SIZE);
        if (errno == EPIPE)
        {
            perror(argv[1]);
            close(privat_fifo);
            close(general_fifo);
            exit(0);
        }
        write(1, &buf, bytes_read);
    }

close(privat_fifo);
close(general_fifo);
unlink(privat_fifo_name);
return 0;
}
