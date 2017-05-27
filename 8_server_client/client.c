#define _GNU_SOURCE
#define __USE_GNU
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <malloc.h>
#include <sched.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/times.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/tcp.h>

/*
server input:
-p - port
-tn - threads num of comp
*/
struct INFO
{
	double a;
	double b;
	int num;
	double res;
	pthread_t th_pid;
};


int client(int port, int th_num);
void integ_calc (double start, double end, double* res, int th_num);
void * Integral(void * info);


int main (int argc, char* argv[])
{
    int port = 0;
    int th_num = 0;
    int i;
    int j = 0;
	char* endptr;

    for (i=1; i<argc; i++)
    {
        if ( strcmp(argv[i], "-p") == 0)
        {
			int val = (size_t)strtol(argv[i+1], &endptr, 10);
            if ((errno == ERANGE && (val == INT_MAX || val == INT_MIN))
                   || (errno != 0 && val == 0) || endptr == argv[1] || *endptr != '\0' ||
                                val <= 0)
            {
               printf("Incorrect port\n");
               return -1;
            }
            port = val;
            printf("The port is %d\n", port );
            j++;
        }

        if ( strcmp(argv[i], "-tn") == 0)
        {
			if (argv[i+1] == NULL)
            {
                printf("Not enough args\n");
    			return 1;
            }
			int val = (size_t)strtol(argv[i+1], &endptr, 10);
            if ((errno == ERANGE && (val == INT_MAX || val == INT_MIN))
                   || (errno != 0 && val == 0) || endptr == argv[1] || *endptr != '\0' ||
                                val <= 0)
            {
               printf("Incorrect port\n");
               return -1;
            }
            th_num = val;
            j++;
            printf("The number of threads is %d\n", th_num);
        }
    }
    if ( (port == 0) || (j != 2) )
        {
			printf("Invalid input\n");
			return 1;
		}

    client (port, th_num);
    return 0;
}

int client(int port, int th_num)
{
    struct sockaddr_in gen_add, cli_add;
    struct pollfd pfd;
    char buffer[256];
    int n = 0;
    int opt = 1;
    int sockfd;
    double start = 0;
    double end = 0;
    double res = 0;
    unsigned int len;
    int keepcnt, keepidle, keepintvl;

/*######## UDP ########*/

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
            perror("error socket create");
			exit(1);
    }

    gen_add.sin_family = AF_INET;
    gen_add.sin_port = htons(port);
    gen_add.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) == -1)
	{
            perror("setsockopt");
			exit(1);
	}

    if (bind(sockfd, (struct sockaddr *) &gen_add, sizeof(struct sockaddr)) < 0)
	{
          perror("error on binding");
		  exit(1);
	 }

    memset(&buffer,0,sizeof(buffer));
    len = sizeof(cli_add);

    n = recvfrom (sockfd, buffer, 255, 0, (struct sockaddr*) &cli_add, &len);
    if (n <= 0)
	{
        perror ("error reading from socket");
		exit(1);
	}

    close (sockfd);

/*######## TCP ########*/


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
            perror("error socket");
			exit(1);
	}

    if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) == -1)
	{
        perror("setsockopt");
		exit(1);
	}

    keepcnt = 1;
    keepidle = 1;
    keepintvl = 1;

	if (setsockopt (sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof (opt)) != 0)
	{
		perror ("setsockopt KEEPALIVE");
		exit(1);
	}
    if (setsockopt (sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(int)) != 0)
	{
        perror ("setsockopt KEEPCNT\n");
		exit(1);
	}
    if (setsockopt (sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int)) != 0)
	{
        perror ("setsockopt KEEPIDLE\n");
		exit(1);
	}
    if (setsockopt (sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int)) != 0)
	{
        perror ("setsockopt KEEPINTVL\n");
		exit(1);
	}

    bzero((char *) &gen_add, sizeof(gen_add));
    gen_add.sin_family = AF_INET;
    bcopy((char *) &cli_add.sin_addr.s_addr, (char *)&gen_add.sin_addr.s_addr, sizeof (cli_add));

    gen_add.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &gen_add, sizeof(gen_add)) < 0)
	{
            perror("perror connecting");
			exit(1);
	}

/*######## CONNECTION ########*/

    pfd.fd = sockfd;
    pfd.events = POLLOUT | POLLHUP | POLLRDNORM;
    memcpy (buffer, &th_num, sizeof (int));
    if (poll (&pfd, 1, -1) > 0)
    {
            n = write (sockfd, buffer, sizeof (int));
            if (n <= 0)
			{
                    perror ("error writing to socket");
					exit(1);
			}
    }
    bzero (buffer, 256);
    pfd.fd = sockfd;
    pfd.events = POLLIN | POLLHUP | POLLRDNORM;
    if (poll (&pfd, 1, -1) > 0)
    {
            n = read (sockfd, buffer, 255);
            if (n <= 0)
			{
                    perror ("error reading from socket");
					exit(1);
			}
    }

    memcpy (&start, buffer, sizeof (double));
    memcpy (&end, buffer + sizeof (double), sizeof (double));

    integ_calc (start, end, &res, th_num);

    bzero (buffer, 256);
    memcpy (buffer, &res, sizeof (double));

    pfd.fd = sockfd;
    pfd.events = POLLOUT | POLLHUP | POLLRDNORM;
    if (poll (&pfd, 1, -1) > 0)
    {
            n = write (sockfd, buffer, sizeof (double));
            if (n <= 0)
			{
                    perror ("error writing to socket");
					exit(1);
			}
    }

    close(sockfd);

    return 0;
}

 void integ_calc (double start, double end, double* res, int th_num)
 {
	 double result=0;
	 double a=start;
	 double b=end;
	 int i, err, num, j;
	 double h;
	 int num_cpus;

	 struct INFO* info;
	 num_cpus=sysconf(_SC_NPROCESSORS_ONLN);

	 num = th_num;

	 if (num <= 0)
	 {
		 printf("ERROR");
		 exit(-1);
 	 }

	 if (num >= num_cpus)
	 {
		 info = (struct INFO*) malloc(sizeof(struct INFO)*num);
		 if (info == NULL)
		 {
			 printf("ERROR in malloc\n");
			 exit(-1);
		 }
	 }
	 else
	 {
		 info = (struct INFO*) malloc(sizeof(struct INFO)*num_cpus);
		 if (info == NULL)
		 {
			 printf("ERROR in malloc\n");
			 exit(-1);
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
			 exit(-1);
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
			 exit(-1);
		 }
	 }

	 for(i=0; i<num; i++)
	 {
		 void * temp;
		 pthread_join(info[i].th_pid, &temp);
		 result += info[i].res;
	 }

	 (*res) = result;
		free(info);
		return;
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
