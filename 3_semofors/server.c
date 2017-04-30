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
#define SHARED_MEM_SIZE 1024
#define NUM_OF_SEMS 5
#define NUM_OF_OP 3

struct message {
    char text[SHARED_MEM_SIZE-4];
    int num;
};

int main(int argc, char ** argv){

int semid; /* дескриптор для массива семафоров */
key_t key_sem, key_mem;
int shmid;
char pathmame_sems[] = "client.c";
char pathname_mem[] = "server.c";
int bytes_read = SHARED_MEM_SIZE;
//char *shar_mem;
//char buf[SHARED_MEM_SIZE] = {};
struct sembuf mybuf[NUM_OF_OP] = {};
//int i;
union semun arg;
int in;
struct message *shar_mem;

//###########################################
in = open(argv[1], O_RDONLY);

if (argc != 2)
{
    printf("No file\n");
    exit(-1);
}

if( (key_sem = ftok(pathmame_sems,0) ) < 0)
{
	printf("Can't generate key\n");
	exit(-1);
}

if( (semid = semget(key_sem, NUM_OF_SEMS, 0666 | IPC_CREAT|IPC_EXCL) ) < 0)
{
    if(errno != EEXIST)
    {
        printf("Can't create SEMS\n");
        exit(-1);
    }
    else
    {
        if( (semid = semget(key_sem, NUM_OF_SEMS, 0)) < 0)
        {
            printf("Can't join SEMS\n");
            exit(-1);
        }

    }
}

//###########################################
// leaving one server alive
mybuf[0].sem_op = 0;
mybuf[0].sem_num = 2;
mybuf[0].sem_flg = IPC_NOWAIT;

mybuf[1].sem_op = 1;
mybuf[1].sem_num = 2;
mybuf[1].sem_flg = SEM_UNDO;
if (semop(semid, mybuf, 2) == -1)
{
    printf("The server already exist\n");
    exit(-1);
}

//###########################################
// create shared mem
if( (key_mem = ftok(pathname_mem,0) ) < 0)
    {
        printf("Can't generate key\n");
        exit(-1);
    }

if( (shmid = shmget(key_mem, SHARED_MEM_SIZE-4, 0666|IPC_CREAT|IPC_EXCL) ) < 0)
    {
        if(errno != EEXIST)
            {
                printf("Can't create shared memory\n");
                exit(-1);
            }
        else
            {
                if( (shmid = shmget(key_mem,SHARED_MEM_SIZE-4, 0) ) < 0)
                    {
                        printf("Can't find shared memory\n");
                        exit(-1);
                    }
            }
    }

if( (shar_mem = (struct message*)shmat(shmid, NULL, 0)) == (struct message *)(-1) )
    {
       printf("Can't attach shared memory\n");
       exit(-1);
    }

//###########################################
//init sems

arg.val = 0;
semctl(semid, 0 , SETVAL, arg.val);

arg.val = 0;
semctl(semid, 1 , SETVAL, arg.val);

mybuf[0].sem_op = 1;
mybuf[0].sem_num = 4;
mybuf[0].sem_flg = 0;
semop(semid, mybuf, 1);

//###########################################
//writing in shared mem
mybuf[0].sem_op = 1;
mybuf[0].sem_num = 0;
mybuf[0].sem_flg = 0;

mybuf[1].sem_op = -1;
mybuf[1].sem_num = 0;
mybuf[1].sem_flg = SEM_UNDO;
semop(semid, mybuf, 2);

mybuf[0].sem_op = 1;
mybuf[0].sem_num = 1;
mybuf[0].sem_flg = 0;

mybuf[1].sem_op = -1;
mybuf[1].sem_num = 1;
mybuf[1].sem_flg = SEM_UNDO;
semop(semid, mybuf, 2);


while( bytes_read != 0)
{

    mybuf[0].sem_op = 0;
    mybuf[0].sem_num = 0;
    mybuf[0].sem_flg = 0;

    mybuf[1].sem_op = 0;
    mybuf[1].sem_num = 1;
    mybuf[1].sem_flg = IPC_NOWAIT;

    if (semop(semid, mybuf, 2) == -1)
    {
        semctl(semid, 0, IPC_RMID, 0);
        shmctl(shmid, IPC_RMID, 0);
        close(in);
        exit(-1);
    }

    if ( (bytes_read=read(in,shar_mem->text,1024-4) )<0 )
    {
	    perror(argv[1]);
	    break;
	}
	shar_mem->num=bytes_read;

    mybuf[0].sem_op = 1;
    mybuf[0].sem_num = 0;
    mybuf[0].sem_flg = 0;

    mybuf[1].sem_op = 0;
    mybuf[1].sem_num = 1;
    mybuf[1].sem_flg = IPC_NOWAIT;

    if (semop(semid, mybuf, 2) == -1)
    {
        semctl(semid, 0, IPC_RMID, 0);
        shmctl(shmid, IPC_RMID, 0);
        close(in);
        exit(-1);
    }

}

semctl(semid, 0, IPC_RMID, 0);
shmctl(shmid, IPC_RMID, 0);
close(in);
return 0;
}
