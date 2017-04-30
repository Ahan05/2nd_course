#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define CHILD_BUF_SIZE 65536

// struct which contains info of a child in parent proc
typedef struct child_struct
{
    int wr;
    int rd;
    char* buf;
    int buf_len;
    int wr_cl;
    int rd_cl;
    int buf_size;
    char* buf_len_point;
} child_info;

// arrays which at first contains descriptors
int (*parent_rd_wr)[2];
int (*child_rd_wr)[2];
int i;
int n; // number of childs

int power(int k)
{
    int x=1;
    int i;
    for (i=0; i<k; i++)
    {
        x = x*3;
    }
    return x;
}


//###################################
// in child
void child (int child_num, int parent_fd[2], int child_fd[2])
{
    char buf[CHILD_BUF_SIZE];
    int rd_len, wr_len;
    int i;
// closing useless descriptors, so that everything would close
// if any child suddenly ended working

    close(parent_fd[0]);
    close(parent_fd[1]);

    for(i=0;i<n;i++)
    {
        if(child_num != i)
        {
            close(child_rd_wr[i][0]);
	        close(child_rd_wr[i][1]);
	        close(parent_rd_wr[i][0]);
	        close(parent_rd_wr[i][1]);
        }
    }

// reading and writing from descriptors
    while(1)
    {

        if ((rd_len = read(child_fd[0],buf,sizeof(buf)))<0)
        {
            perror("ERROR in child reading");
            exit(-1);
        }
        else if (rd_len == 0)
        {
            close(child_fd[0]);
            close(child_fd[1]);
            exit(0);
        }
        else if (rd_len > 0)
        {
            if ((wr_len = write(child_fd[1], buf, rd_len))<0)
            {
                perror("ERROR in child reading");
                exit(-1);
            }
        }
    }
}
//###################################

int main(int argc, char **argv)
{
    int file;
    pid_t p;
    int* general_fd_1;
    int* general_fd_2;
    fd_set setR;
    fd_set setW;
    int max_fd;
    int io_len;
    child_info *link;
    int end;
    int i, j;
    char* endptr;

//###################################
// checking the right input
    if (argc != 3)
    {
        fprintf(stderr, "Error input, wrong number of args\n");
        return 0;
    }

    n = strtol(argv[1], &endptr,10);

    if ( ((*endptr != '\0')) || (errno == ERANGE))
    {
        fprintf(stderr, "Invalid input. Firstly input number of childs, then the file name\n");
        return 0;
    }

//###################################
// creating a lot of descriptors
    parent_rd_wr = malloc(sizeof(int)*(2*n+1));
    child_rd_wr = malloc(sizeof(int)*(2*n));

    general_fd_1 = malloc(sizeof(int)*(2*n));
    general_fd_2 = malloc(sizeof(int)*(2*n));

// init pipes and open file
    if ((file = open(argv[2], O_RDONLY))<0)
    {
        perror("ERROR opening the file");
        return -1;
    }

    j=0;
    for(i=0;i<2*(n)+1;i=i+2, j++)
    {
	    if(pipe(general_fd_1+i)<0)
        {
		    perror("Cannot create pipe\n");
		    return -1;
	    }

	    if(pipe(general_fd_2+i)<0)
        {
		    perror("Cannot create pipe\n");
	        return -1;
	    }

        if (j==0)
        {
            parent_rd_wr[j][1] = -1;
            child_rd_wr[j][0] = file;

            parent_rd_wr[j][0] = general_fd_2[i];
            child_rd_wr[j][1] = general_fd_2[i+1];
        }
        else if (j == n)
        {
            parent_rd_wr[j][1] = 1;
            parent_rd_wr[j][0] = -1;
        }
        else
        {
            parent_rd_wr[j][1] = general_fd_1[i+1];
            child_rd_wr[j][0] = general_fd_1[i];

            parent_rd_wr[j][0] = general_fd_2[i];
            child_rd_wr[j][1] = general_fd_2[i+1];
        }
    }

    free(general_fd_1);
    free(general_fd_2);
//fprintf(stderr, "Ended creating pipes\n");
//###################################
// forking

    for(i=0;i<n;i++)
    {
        p=fork();

        if(p<0)
        {
            perror("Cannot create a child\n");
            exit (-1);
        }

        if(p==0)
        {
            child(i, parent_rd_wr[i],child_rd_wr[i]);
            exit(1);
        }
    }

//###################################
// in parent preparing

// setting non_block flags to needed descriptors
    for(i=1;i<(n);i++)
    {
        fcntl(parent_rd_wr[i][0],F_SETFL,O_NONBLOCK);
        fcntl(parent_rd_wr[i][1],F_SETFL,O_NONBLOCK);
    }
    fcntl(parent_rd_wr[0][0],F_SETFL,O_NONBLOCK);
    fcntl(parent_rd_wr[n][1],F_SETFL,O_NONBLOCK);

// creating child info structures
    link = malloc(sizeof(child_info)*n);

    for(i=0;i<n;i++)
    {
        close(child_rd_wr[i][1]);
	    close(child_rd_wr[i][0]);
    }

    for(i=0; i<n; i++)
    {
	    link[i].rd=parent_rd_wr[i][0];
	    link[i].wr=parent_rd_wr[i+1][1];
        link[i].wr_cl=0;
	    link[i].buf_len=0;
	    link[i].rd_cl=0;
        link[i].buf_size = power(n-i);
        link[i].buf = malloc(sizeof(char)*link[i].buf_size);
    }


//###################################
// reading and writing into childs and file
    end=0; // tells us when we need to stop, if end>0 then stop

    while(!end)
    {
        FD_ZERO(&setR);
	    FD_ZERO(&setW);

	    max_fd = 0;

	    for(i=0;i<n;i++)
        {

        // checking if the descriptors is able to be readed
            if( (link[i].buf_len < link[i].buf_size ) && (link[i].rd_cl == 0) )
            {
                FD_SET(link[i].rd, &setR);

                if(max_fd < link[i].rd)
                    max_fd=link[i].rd;

        	}

        // checking if the descriptors should be closed
            if( (link[i].buf_len==0) && (link[i].rd_cl != 0) && (link[i].wr_cl == 0))
            {
                close(link[i].wr);
                link[i].wr_cl=1;
            }

        // checking if the descriptors is able to be writed
            if( (link[i].buf_len>0) && (link[i].wr_cl == 0) )
            {
                FD_SET(link[i].wr,&setW);

                if(max_fd<link[i].wr)
                    max_fd=link[i].wr;
    	    }

    	}

        if (max_fd == 0)
            end = 1;

    // choosing the descriptors and writing in them or reading
    	if(max_fd!=0)
        {
        	if( select(max_fd+1, &setR, &setW,NULL,NULL)<0 )
            {
        	    perror("select error\n");
                end = 1;
        	    for(i=0;i<n-1;i++)
                {
            	    close(link[i].wr);
            	    close(link[i].rd);
            	    link[i].rd_cl=1;
            	    link[i].wr_cl=1;
    		    }
    	    }
        }

        if (!end)
        {
        	for(i=0;i<n;i++)
            {

            // writing into available descriptors, which were chosen by select
                if( (FD_ISSET(link[i].wr,&setW)) && (link[i].wr_cl == 0) )
                {
            		if ((io_len=write(link[i].wr, link[i].buf, link[i].buf_len))<=0)
                    {
            	    	perror("ERROR in parent write\n");
            		    close(link[i].wr);
            		    close(link[i].rd);
            		    link[i].rd_cl=1;
            		    link[i].wr_cl=1;
            		}
            		if(io_len>0)
                    {
            		    memcpy(link[i].buf, &link[i].buf[io_len], link[i].buf_len-io_len);
            		    link[i].buf_len-=io_len;
            		}
            	}

            // reading from available descriptors, which were chousen by select
                if(FD_ISSET(link[i].rd, &setR) && (link[i].rd_cl == 0))
                {
        			if ( (io_len=read(link[i].rd, &link[i].buf[link[i].buf_len], link[i].buf_size -link[i].buf_len))<0)
                    {
                        perror("ERROR in parent read\n");
                        close(link[i].wr);
                        link[i].wr_cl = 1;
                    }

                    else if(io_len>0)
                    {
            		    link[i].buf_len+=io_len;
        		    }
        		    else if (io_len == 0)
                    {
            		    close(link[i].rd);
            		    link[i].rd_cl=1;
        		    }
        		}
        	}
    	}
    }

// freeing allocated memory
    free(parent_rd_wr);
    free(child_rd_wr);
    for (i=0; i<n; i++)
    {
        free(link[i].buf);
    }
    free(link);
}
