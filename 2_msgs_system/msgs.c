#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#define SIZE_OF_PID_NAME 10

int main(int argc, char ** argv){

char* str;
char* endptr;
int i, msqid;
key_t key;
long int n;
int len;
pid_t p = 5;
char pathname[] = "msgs.c";
//char pid_name[10]={};

struct mymsg
    {
        long mtype;
        char mtext[10];
    } mybuf;
//----------------
//checking input from command line

if (argc != 2)
{
    fprintf(stderr, "Error, too few arguments\n");
    return 0;
}
str = argv[1];
n = strtol(str, &endptr,10);
if ( ((*endptr != '\0')) || (errno == ERANGE))
{
    fprintf(stderr, "Invalid input");
    return 0;
}
//----------------
//creating msg queue

if( (key = ftok(pathname,0)) < 0 )
{
    printf("Can\'t generate key\n");
    exit(-1);
}

if( (msqid = msgget(key, 0666 | IPC_CREAT)) < 0)
{
    printf("Can\'t get msqid\n");
    exit(-1);
}
//----------------
// creating children pids and sending pids
for (i=0; i<n; i++)
{
    p = fork();
    if (p == 0)
        break;
}

if (p == 0)
{
    if( ( len = msgrcv(msqid, (struct msgbuf *) &mybuf, 0, i+1, 0) < 0) )
    {
        printf("Can\'t receive message from queue\n");
        exit(-1);
    }

    printf("%d ", i);
    fflush(stdout);

    mybuf.mtype = i+n+1;
    if (msgsnd(msqid, (struct msgbuf *) &mybuf, 0, 0) < 0)
    {
        fprintf(stderr, "Can\'t send message to queue\n");
        perror(argv[1]);
        msgctl(msqid, IPC_RMID, NULL);
        exit(-1);
    }
}

if (p>0)
{
    for (i=0; i<n; i++)
    {
        mybuf.mtype = i+1;
        if (msgsnd(msqid, (struct msgbuf *) &mybuf, 0, 0) < 0)
        {
            fprintf(stderr, "Can\'t send message to queue\n");
            perror(argv[1]);
            msgctl(msqid, IPC_RMID, NULL);
            exit(-1);
        }

        if( ( len = msgrcv(msqid, (struct msgbuf *) &mybuf, 0, i+n+1, 0) < 0))
        {
            printf("Can\'t receive message from queue\n");
            perror(argv[1]);
            exit(-1);
        }
    }
    msgctl(msqid, IPC_RMID, NULL);
}


return 0;
}
