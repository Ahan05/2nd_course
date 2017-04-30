#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

char word;
int bit_num;
pid_t p;

//is called if bit == 1
void dot(int sig)
{
    word |= bit_num;
    bit_num = bit_num>>1;
    kill(p, SIGUSR1);
}

//is called if bit == 0
void dash(int sig)
{
    bit_num = bit_num>>1;
    kill(p, SIGUSR1);
}

//is called when child recieves SIGUSR1
void nothing(int sig)
{

}

// is called in parent when child dies
void child_death(int sig)
{
    exit(0);
}

int main(int argc, char ** argv)
{

    sigset_t mask;
    int fd;
    pid_t ppid = getpid();

    if (argc != 2)
    {
        exit(-1);
    }

    if ((fd = open(argv[1], O_RDONLY))<0)
    {
        perror("No FILE");
        exit(-1);
    }

    if (getppid() == 1)
    {
        printf("The parent dies\n");
        exit(-1);
    }


    struct sigaction child_die;
    memset(&child_die, 0, sizeof(child_die));
    child_die.sa_handler = child_death;
    sigfillset(&child_die.sa_mask);
    sigaction(SIGCHLD, &child_die, NULL);

    struct sigaction react_u1;
    memset(&react_u1, 0, sizeof(react_u1));
    react_u1.sa_handler = dot;
    sigfillset(&react_u1.sa_mask);
    sigaction(SIGUSR1, &react_u1, NULL);

    struct sigaction react_u2;
    memset(&react_u2, 0, sizeof(react_u2));
    react_u2.sa_handler = dash;
    sigfillset(&react_u2.sa_mask);
    sigaction(SIGUSR2, &react_u2, NULL);

    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    bit_num=1;
    bit_num = bit_num<<7;
    word = 0;
    p = fork();

//In child
    if (p==0)
    {
        char in;
        sigemptyset(&mask);

        struct sigaction Nothing;
        memset(&Nothing, 0, sizeof(Nothing));
        Nothing.sa_handler = nothing;
        sigfillset(&Nothing.sa_mask);
        sigaction(SIGUSR1, &Nothing, NULL);

        while(read(fd, &in, 1) > 0)
        {
            int i;
            for(i = 128; i!=0; i=i>>1)
            {
                if( in & i )
                {
                    kill(ppid, SIGUSR1);
                }
                else
                {
                    kill(ppid, SIGUSR2);
                }
                sigsuspend(&mask);
            }
        }
        exit(0);
    }

//In parent
    else
    {
        sigemptyset(&mask);
        while(1)
        {
            if (bit_num==0)
            {
                write(1, &word, 1);
                fflush(stdout);
                bit_num=128;
                word=0;
            }
            sigsuspend(&mask);
        }
        exit(0);
    }
}
/*
SIGPIPE - write on a pipe with no reader +
SIGCHLD +
SIGHUP +
--------------
SIGCONT - перед sighub, анологично+

SIGPOLL/SIGIO - ioctl был установлен флаг I_SETSIG.
После этого, нам посылается этот сигнал когда изменятеся статус открытого файла.
Изменение статуса открытого файла значит, что если мы произведенм чтение/запись в неблокирующем режиме, то мы успешно прочтем/запишем
если же в блокирующем режиме то мы не заблокируюемся при чтении/записи

prctl
*/
