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
struct sembuf mybuf[NUM_OF_OP] = {};
//int i;
struct message *shar_mem;

//###########################################
//init SEMS
if( (key_sem = ftok(pathmame_sems,0) ) < 0)
    {
		printf("Can\'t generate key\n");
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
// leaving one client alive
mybuf[0].sem_op = 0;
mybuf[0].sem_num = 3;
mybuf[0].sem_flg = IPC_NOWAIT;

mybuf[1].sem_op = 1;
mybuf[1].sem_num = 3;
mybuf[1].sem_flg = SEM_UNDO;
if (semop(semid, mybuf, 2) == -1)
{
    printf("The client already exist\n");
    exit(-1);
}

//###########################################
// init shared mem
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
//waiting for init

mybuf[0].sem_op = -1;
mybuf[0].sem_num = 4;
mybuf[0].sem_flg = 0;
semop(semid, mybuf, 1);


//###########################################
//writing in shared mem

mybuf[0].sem_op = 1;
mybuf[0].sem_num = 0;
mybuf[0].sem_flg = SEM_UNDO;

mybuf[1].sem_op = -1;
mybuf[1].sem_num = 0;
mybuf[1].sem_flg = 0;
semop(semid, mybuf, 2);

mybuf[0].sem_op = 1;
mybuf[0].sem_num = 1;
mybuf[0].sem_flg = 0;

mybuf[1].sem_op = -1;
mybuf[1].sem_num = 1;
mybuf[1].sem_flg = SEM_UNDO;
semop(semid, mybuf, 2);

while(1)
{
    mybuf[0].sem_op = -1;
    mybuf[0].sem_num = 0;
    mybuf[0].sem_flg = 0;

    mybuf[1].sem_op = 1;
    mybuf[1].sem_num = 0;
    mybuf[1].sem_flg = 0;

    mybuf[2].sem_op = 0;
    mybuf[2].sem_num = 1;
    mybuf[2].sem_flg = IPC_NOWAIT;

    if (semop(semid, mybuf, 3) == -1)
    {
        semctl(semid, 0, IPC_RMID, 0);
        shmctl(shmid, IPC_RMID, 0);
        exit(-1);
    }


    write(1,shar_mem->text,shar_mem->num);
	if (!shar_mem->num) break;

    mybuf[0].sem_op = -1;
    mybuf[0].sem_num = 0;
    mybuf[0].sem_flg = 0;

    mybuf[1].sem_op = 0;
    mybuf[1].sem_num = 1;
    mybuf[1].sem_flg = IPC_NOWAIT;

    if (semop(semid, mybuf, 2) == -1)
    {
        semctl(semid, 0, IPC_RMID, 0);
        shmctl(shmid, IPC_RMID, 0);
        exit(-1);
    }
}

semctl(semid, 0, IPC_RMID, 0);
shmctl(shmid, IPC_RMID, 0);
return 0;
}
