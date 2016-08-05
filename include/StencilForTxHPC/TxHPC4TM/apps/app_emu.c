
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define MMAP_DIR "/lfs/"
//#define MMAP_DIR "/dev/shm/r6/"
//#define MMAP_DIR "lfs"
#define METAfile "/lfs/meta";
//#define METAfile "/dev/shm/r6/meta"
//#define METAfile "lfs/meta"

int failure;
int *shelf_descriptor;
int meta_descriptor;
char **mem;
char **data_shelf;
long long unsigned ** memcast;
long long unsigned fk;      
long long unsigned ** filler;   
int total_devices;
int no_of_data_devices;
int no_of_coding_devices;
long long unsigned mem_allocate;
long long unsigned pad;
long long unsigned * scratchpad;
char **shelf_names;
long long unsigned devicesize;

double calculate_time(clock_t begin, clock_t end)                                                                                                       /*Calculate the time using CPU cycles*/
{
        double total;
        total = ((double)(end - begin)/CLOCKS_PER_SEC);
        return total;
}
                                           
void print_region()
{
	int i, j;
	
        for(i = 0; i < no_of_data_devices; i++)
        {
 	       for(j = 0; j < devicesize/sizeof(long long unsigned); j++)
               {
                       printf("%llu ", filler[i][j]);
			fflush(NULL);
               }
		printf("\n");
        }
	printf("\n");
	
}

void fill_region()
{
	int i, j;
	for(i = 0; i < no_of_data_devices; i++)
	{
		for(j = 0; j < devicesize/sizeof(long long unsigned); j++)
		{
//		printf("Here\n");
//		fflush(NULL);
			filler[i][j] = fk;
			fk++;
		}
	}
//	printf("The data region is\n");
//	print_region();
}

void iterate_compute()
{                                                                                                    
	int i, j, k;                                                                                           
	time_t t;
		fill_region();
		long long unsigned prev = 1;
		long long unsigned cur = 1;
	for(k = 0; k < 5; k++)
	{
		for(i = 0; i <no_of_data_devices; i++)
		{
			for(j = 0; j < devicesize/sizeof(long long unsigned); j++)
			{
				cur = filler[i][j];
				if(j == 0)
				{
					if(i == 0)
					{
						filler[i][j] = filler[i][j] + filler[i][j + 1];
					}
					else
					{
						filler[i][j] = filler[i][j] + filler[i][j + 1] + prev;
					}
				}
				else if(j == (devicesize/sizeof(long long unsigned) - 1))
				{
					if(i == no_of_data_devices - 1)
					{
						filler[i][j] = filler[i][j] + prev + (filler[i][j] +(filler[i][j] - prev));
					}
					else
					{
						filler[i][j] = filler[i][j] + prev + filler[i + 1][0];
					}
				}
				else
				{
					filler[i][j] = filler[i][j] + prev + filler[i][j + 1];
				}
				prev = cur;
			}
		}
		scratchpad[k] = filler[0][2];
	//	printf("The scratchpad has %llu\n", scratchpad[k]);
	//	print_region();
	}
	for(k = 0; k < 2; k++)	
	{
		 for(i = 0; i < no_of_data_devices; i++)
                 {
                        for(j = 0; j < devicesize/sizeof(long long unsigned); j++)
                        {	
				filler[i][j] = (long long unsigned)filler[i][j]/scratchpad[k];
			}
		}
//		printf("The data region is\n");
//		print_region();
	}
	
}

void open_data_shelves(int total_devices, long long unsigned memory_allocate)
{
        int i, res;
        char buf[5];
        shelf_names = (char **)malloc(sizeof(char *)*total_devices);
        for(i = 0; i < total_devices; i++)
        {
                shelf_names[i] = (char *)malloc(sizeof(char)*30);
                strcpy(shelf_names[i], MMAP_DIR);
                snprintf(buf, 5, "%c", (i+97));
                strcat(shelf_names[i], buf);
        }
#ifdef DEBUG
debug("Shelf names given\n");
#endif

        for(i = 0; i < total_devices; i++)
        {
                shelf_descriptor[i] = open(shelf_names[i], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (shelf_descriptor[i] < 0)
                {
                         error("Shelf creation failed\n");
                }
        }
#ifdef DEBUG
debug("Allocating books from LFS for data shelves\n");
#endif
        for(i = 0; i < total_devices; i++)
        {
                res = ftruncate(shelf_descriptor[i], (long long unsigned)memory_allocate/total_devices);
                if (res < 0)
                {
                        error("ftruncate failed due to invalid devicesize");
                }
        }
#ifdef DEBUG
debug("Allocating memory for booksfrom LFS\n");
#endif

	for(i = 0; i < total_devices; i++)
	{
               data_shelf[i] = (char *)mmap(0, (long long unsigned)memory_allocate/total_devices, PROT_READ|PROT_WRITE, MAP_SHARED, shelf_descriptor[i], 0);
	       filler[i] = (long long unsigned *)data_shelf[i];
               if(data_shelf[i] == NULL)
               {
                       error("Couldn't allocate memory for the data device %d\n", i);
               }
               if(close(shelf_descriptor[i]) == -1)
               {
                       error("File close failed\n");
               }
        }

}


int main(int argc, char*argv[])
{
	int crasher;
	clock_t begin, end;
	int i;

	
	fk = 1;

	sscanf(argv[1], "%llu", &mem_allocate);
	sscanf(argv[2], "%d", &no_of_data_devices);
	shelf_descriptor = (int *)malloc(sizeof(int)*no_of_data_devices);
	data_shelf = (char **)malloc(sizeof(char *)*(no_of_data_devices));
	filler = (long long unsigned **)malloc(sizeof(long long unsigned *)*(no_of_data_devices));
	sscanf(argv[3], "%llu", &pad);
	scratchpad = (long long unsigned *)malloc(pad);

	open_data_shelves(no_of_data_devices, mem_allocate);
	for(i =0; i < no_of_data_devices; i++)
	{
	//	filler[i] = (long long unsigned *)data_shelf[i];
	//	filler[i] = (long long unsigned *)malloc((long long unsigned)mem_allocate/no_of_data_devices);
	}
	devicesize = (long long unsigned)mem_allocate/no_of_data_devices;

	begin = clock();
	iterate_compute();	
	end = clock();
	
	printf("The time for execution is %f seconds.\n", calculate_time(begin, end));
	
	//free(data_shelf);
	for(i = 0; i < no_of_data_devices; i++)
	{
//		free(data_shelf[i]);
	//	free(filler[i]);
	}
	free(data_shelf);
//	free(shelf_names);
//	free(shelf_descriptor);
	free(filler);
	return 0;

}
