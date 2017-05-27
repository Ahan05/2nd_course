#define _GNU_SOURCE
#define __USE_GNU
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <sched.h>

void * Integral(void * info);

struct INFO
{
	double a;
	double b;
	int num;
	double res;
	pthread_t th_pid;
};

int main(int argc, const char * argv[])
{
	double result=0;
	double a=0;
	double b=2800;
	char* endptr;
	int i, err, num, j;
	double h;
	int num_cpus;

	struct INFO* info;
	num_cpus=sysconf(_SC_NPROCESSORS_ONLN);

	if (argc != 2 )
	{
		fprintf(stderr, "Not enough args\n");
		return 0;
	}
	num = strtol(argv[1], &endptr, 10);

    if ( ((*endptr != '\0')) || (errno == ERANGE))
    {
    	fprintf(stderr, "Error input");
        return 0;
    }

	if (num <= 0)
	{
		printf("ERROR");
		return -1;
	}

	if (num >= num_cpus)
	{
		info = (struct INFO*) malloc(sizeof(struct INFO)*num);
		if (info == NULL)
		{
			printf("ERROR in malloc\n");
			return 1;
		}
	}
	else
	{
		info = (struct INFO*) malloc(sizeof(struct INFO)*num_cpus);
		if (info == NULL)
		{
			printf("ERROR in malloc\n");
			return 1;
		}
	}

	h=(b-a)/num;

	for (i=0; i<num; i++)
	{
		info[i].a = a + i*h;
		info[i].b = a + (i+1)*h;
		info[i].num = i%num_cpus;
		info[i].res = -1;
		err = pthread_create( &info[i].th_pid, NULL, Integral, (void*) &info[i]);
		if (err != 0)
		{
			fprintf(stderr,"error in pth_creat\n");
			return 1;
		}

	}

	for (i=num, j=0; i<num_cpus; i++, j++)
	{
		info[i].a = a + j*h;
		info[i].b = a + (j+1)*h;
		info[i].num = i%num_cpus;
		info[i].res = -1;
		err = pthread_create( &info[i].th_pid, NULL, Integral, (void*) &info[i]);
		if (err != 0)
		{
			fprintf(stderr,"error in pth_creat\n");
			return 1;
		}
	}

	for(i=0; i<num; i++)
	{
		void * temp;
		pthread_join(info[i].th_pid, &temp);
		result += info[i].res;
	}
	printf("Result = %lf\n", result);
	free(info);
	return 0;
}
void * Integral(void * info)
{
	double R = 0;
	double v;

	double a = (*((struct INFO *)info)).a;
	double b = (*((struct INFO *)info)).b;
	int num = (*((struct INFO *)info)).num;

	cpu_set_t my_set;
	CPU_ZERO(&my_set);
	CPU_SET(num, &my_set);
	sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

	for (v = a; v < b; v += 1e-5 )
	{
		R += v * (1e-5);
	}

	(*((struct INFO *)info)).res = R;
	pthread_exit (NULL);

}

/*
cpu_set_t my_set;
CPU_ZERO(&my_set);
CPU_SET(num, &my_set);
sched_setaffinity(0, sizeof(cpu_set_t), &my_set);
*/
