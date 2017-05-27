#include <malloc.h>
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
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/tcp.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>

/*
server input:
-p - port
-a - start interval
-b - end interval
-n - num of comp
*/

#define BACKLOG 16
#define MAX_TREADS 16

int con_sock[MAX_TREADS];

int server (int port, double begin, double end, int num);

char dump_str[] = "hi there";
int num_pc = 0;
int num_num = 0;

int main (int argc, char* argv[])
{
    double start = 0;
    double end = 5;
    int port = 0;
    int n = 1;
    int i;
    char* endptr;
    int j = 0;


    for (i=1; i<argc; i++)
    {
        if ( strcmp(argv[i], "-p") == 0)
        {
            /*sscanf(argv[i+1], "%d", &port);
            j++;
            printf("The port is %d\n", port );*/
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
        if ( strcmp(argv[i], "-a") == 0 )
        {
            sscanf(argv[i+1], "%lf", &start);
            j++;
            printf("The start is %lf\n", start );
        }

        if ( strcmp(argv[i], "-b") == 0)
        {
            sscanf(argv[i+1], "%lf", &end);
            j++;
            printf("The end is %lf\n", end );
        }

        if ( strcmp(argv[i], "-n") == 0)
        {
            if (argv[i+1] == NULL)
            {
                printf("Not enough args\n");
    			return 1;
            }
            int val = (size_t)strtol(argv[i+1], &endptr, 10);
            if ((errno == ERANGE && (val == INT_MAX || val == INT_MIN))
                   || (errno != 0 && val == 0) || endptr == argv[1] || *endptr != '\0' ||
                   val > 16 || val <= 0)
            {
               printf("Incorrect number of pcs\n");
               return -1;
            }
            n = val;
            printf("The number is %d\n", n);
            j++;
        }
    }

    if ( (port == 0) || (j != 4) )
        {
			printf("Invalid input\n");
			return 1;
		}

    num_num = n;

    struct timespec time_s;
    struct timespec time_e;

    clock_gettime(CLOCK_REALTIME, &time_s);
    server (port, start, end, n);
    clock_gettime(CLOCK_REALTIME, &time_e);
    double all_time = ( time_e.tv_sec - time_s.tv_sec ) + ((double)(time_e.tv_nsec - time_s.tv_nsec))/ 1000000000L;
    printf("time: %lf\n", all_time);

    return 0;
}

void check_connect(int signo)
{
	if (num_pc != (num_num-1))
	{
		printf("Time's out:No clients connected\n");
		exit(0);
	}
}

int server (int port, double begin, double end, int num)
{
    char buffer[256];
    int val_on = 1;
    int i = 0;
    int n = 0;
    int socketfd = 0;
    int th_num[MAX_TREADS];
    int th_all = 0;
    double start = begin;
    double loc_start = start;
    double loc_end = 0;
    struct pollfd pfd;
    int broadOn = 1;
    struct sockaddr_in addr, serv_addr, cli_addr;
    int keepcnt, keepidle, keepintvl;

    int ip;
    struct ifreq all_ip;

    struct sigaction alarm_hdl;
    sigset_t mask;

    alarm_hdl.sa_handler = check_connect;
    sigfillset(&alarm_hdl.sa_mask);
    sigaction(SIGALRM, &alarm_hdl, NULL);


/*######## UDP ########*/
    ip = socket(AF_INET, SOCK_DGRAM, 0);
    all_ip.ifr_addr.sa_family = AF_INET;
    strncpy(all_ip.ifr_name, "en0", IFNAMSIZ-1);

    ioctl(ip, SIOCGIFBRDADDR, &all_ip);

    close(ip);
    printf("%s\n", inet_ntoa(((struct sockaddr_in *)&all_ip.ifr_addr)->sin_addr));

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr =((struct sockaddr_in *)&all_ip.ifr_addr)->sin_addr;

    addr.sin_port=htons(port);

    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
            perror("server_udp socket\n");
            exit(1);
    }

    if(setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, &broadOn, 4) != 0)
    {
            perror("server_udp setsockopt\n");
            exit(1);
    }

    if (sendto(socketfd, dump_str,9, 0,(struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
            perror("server_udp sendto\n");
            exit(1);
    }

    close (socketfd);

/*######## TCP ########*/
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);


    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
            perror("server_tcp socket\n");
            exit(1);
    }

    if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &val_on, sizeof val_on) != 0)
    {
            perror("server_tcp setsockopt\n");
            exit(1);
    }

    keepcnt = 1;
    keepidle = 1;
    keepintvl = 1;
    if (setsockopt (socketfd, SOL_SOCKET, SO_KEEPALIVE, &val_on, sizeof (val_on)) != 0)
    {
        printf ("setsockopt KEEPALIVE");
        exit(1);
    }
    if (setsockopt (socketfd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(int)) != 0)
    {
        perror ("setsockopt KEEPCNT\n");
        exit(1);
    }
    if (setsockopt (socketfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int)) != 0)
    {
        perror ("setsockopt KEEPIDLE\n");
        exit(1);
    }
    if (setsockopt (socketfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int)) != 0)
    {
        perror ("setsockopt KEEPINTVL\n");
        exit(1);
    }

    if(bind(socketfd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) < 0)
    {
            close(socketfd);
            perror("server_tcp bind\n");
            return 0;
    }

    alarm(5);
    for (num_pc = 0; num_pc < num; num_pc++) {
            if (listen(socketfd, BACKLOG) == -1)
            {
                    close(socketfd);
                    perror ("server_tcp listen\n");
                    return 0;
            }

            size_t addr_size = sizeof(cli_addr);

            if ((con_sock[num_pc] = accept(socketfd, (struct sockaddr *)&cli_addr, (socklen_t *)&addr_size)) < 0)
            {
                    perror("server_tcp accepting connection\n");
                    return 0;
            }
    }
    alarm_hdl.sa_handler = SIG_IGN;
    sigfillset(&alarm_hdl.sa_mask);
    sigaction(SIGALRM, &alarm_hdl, NULL);

/*######## CONNECTION ########*/
    for (i = 0; i < num; i++)
    {
            pfd.fd = con_sock[i];
            pfd.events = POLLIN | POLLHUP | POLLRDNORM;
            if (poll (&pfd, 1, -1) > 0)
            {
                    n = read (con_sock[i], buffer, 255);
                    if (n <= 0)
                    {
                            perror ("CONNECTION_ERROR reading from socket");
                            return 0;
					}
            }
            memcpy (&th_num[i], buffer, sizeof (int));
    }

    for (i = 0; i < num; i++)
            th_all += th_num[i];

    double width = (end - begin) / th_all;

    for (i = 0; i < num; i++)
    {

            start = loc_start;
            loc_end = loc_start + width * th_num[i];
            loc_start = loc_end;
            memcpy (buffer, &start, sizeof (double));
            memcpy (buffer + sizeof (double), &loc_end, sizeof (double));

            n = write (con_sock[i], buffer, 2 * sizeof (double));
            if (n <= 0)
            {
                    perror ("CONNECTION_ERROR writing to socket");
                    return 0;
			}
    }

    double tmp = 0;
    double res = 0;
    for (i = 0; i < num; i++)
    {
            bzero (buffer, 256);

            pfd.fd = con_sock[i];
            pfd.events = POLLIN | POLLHUP | POLLRDNORM;
            if (poll (&pfd, 1, -1) > 0)
            {
                    n = read (con_sock[i], buffer, 255);
                    if (n <= 0)
                    {
                            perror ("CONNECTION_ERROR reading from socket");
                            return 0;
					}

                    memcpy (&tmp, buffer, n);
                    res += tmp;
            }
    }

    printf ("result: %lf\n", res);
    return 0;
}
