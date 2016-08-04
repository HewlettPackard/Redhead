#include <math.h>

#include <unistd.h>
#include "include/TxHPC/RAID_6/raid6/include/r6_utils.h"

//#NVRAM
#define MMAP_DIR "/lfs/"
//#define MMAP_DIR "/dev/shm/r6"
//#define MMAP_DIR "lfs"
#define METAfile "/lfs/meta"
//#define METAfile "/dev/shm/r6/meta"
//#define METAfile "lfs/meta"

#define DRAM

int failure;
int *shelf_descriptor;
int meta_descriptor;
char **mem;
char **data_shelf;
long long unsigned ** memcast;
long long unsigned fk;         
char **shelf_names;
double tl;
                                           
void print_region(long long unsigned ** filler, meta * meta_info)
{
	int i, j;
	
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
 	       for(j = 0; j < (int)(meta_info->blocksize)/sizeof(long long unsigned); j++)
               {
                       printf("%llu ", filler[i][j]);
			fflush(NULL);
               }
		printf("\n");
        }
	printf("\n");
	
}

void fill_region(long long unsigned ** filler, meta * meta_info)
{
	int i, j;
        long long unsigned * scratchpad;
        scratchpad = (long long unsigned *)get_scratchpad();
	scratchpad[0] = fk;
	for(i = 0; i < meta_info->no_of_data_devices; i++)
	{
		for(j = 0; j < (int)(meta_info->blocksize)/sizeof(long long unsigned); j++)
		{
			filler[i][j] = fk;
			fk++;
		}
	}
}

void do_nothing(long long unsigned ** filler, long long unsigned *** b_filler, long long unsigned *** a_filler,  meta * meta_info)
{
	return;
}

void iterate_compute(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)
{                                                                                                    
	int i, j;                                                                                           
	int eraser;
	time_t t;
        srand((unsigned)time(&t));
	long long unsigned ** filler;                                                                       
	long long unsigned *** b_filler;                                                                     
	long long unsigned *** a_filler;                                                                     
	filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);  
	b_filler = (long long unsigned ***)malloc(sizeof(long long unsigned**)*meta_info->prefetch_before);
	a_filler = (long long unsigned ***)malloc(sizeof(long long unsigned**)*meta_info->prefetch_after);

	long long unsigned * scratchpad;
        scratchpad = (long long unsigned *)get_scratchpad();
	
	for(i = 0; i < meta_info->prefetch_before; i++)
        {
		b_filler[i] = (long long unsigned**)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
	}
	for(i = 0; i < meta_info->prefetch_after; i++)
        {
		a_filler[i] = (long long unsigned**)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
	}

	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
        }
	
	for(i = 0; i < meta_info->prefetch_before; i++)
	{
		for(j =0; j < meta_info->no_of_data_devices; j++)
		{
			if((*before) != NULL)
                	{
                	        b_filler[i][j] = (long long unsigned*)(*before)[j];
                	}
		}
	}
	
        for(i = 0; i < meta_info->prefetch_before; i++)
        {
		for(j =0; j < meta_info->no_of_data_devices; j++)
                {
 	        	if((*after) != NULL)
        	        {
                	        a_filler[i][j] = (long long unsigned*)(*after)[j];
                	}
		}
         }


	if(meta_info->counter == 0)
	{
		fill_region(filler, meta_info);
	}
	else if(meta_info->counter == 1)
	{
		do_nothing(filler, b_filler, a_filler, meta_info);
	}
	else if(meta_info->counter < 7)
	{
		long long unsigned prev = 1;
		long long unsigned cur = 1;
		for(i = 0; i < meta_info->no_of_data_devices; i++)
		{
			for(j = 0; j < (int)meta_info->blocksize/sizeof(long long unsigned); j++)
			{
				cur = filler[i][j];
				if(j == 0)
				{
					if(i == 0)
					{
						if((*before) != NULL)
						{	
							filler[i][j] = filler[i][j] + filler[i][j + 1] + b_filler[0][(meta_info->no_of_data_devices - 1)][(int)(meta_info->blocksize/sizeof(long long unsigned) - 1)];
						}
						else
						{
							filler[i][j] = filler[i][j] + filler[i][j + 1];
						}
					}
					else
					{
						filler[i][j] = filler[i][j] + filler[i][j + 1] + prev;
					}
				}
				else if(j == ((int)meta_info->blocksize/sizeof(long long unsigned) - 1))
				{
					if(i == (meta_info->no_of_data_devices - 1))
					{
						if((*after) != NULL)
						{
							filler[i][j] = filler[i][j] + prev + a_filler[0][0][0];
						}
						else
						{
							filler[i][j] = filler[i][j] + prev + filler[i][j] + (filler[i][j] - prev);
						}
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
		if(meta_info->counter%2 == 0 && stripe_id == 0)
		{
			scratchpad[(meta_info->counter) - 1] = filler[0][2];
	//		printf("The scratchpad has %llu\n", scratchpad[(meta_info->counter) - 1]);
	//		fflush(NULL);
		}
		else if (meta_info->counter%2 != 0 && stripe_id == (meta_info->prefetch_before + meta_info->prefetch_after + 1))
		{
			scratchpad[(meta_info->counter) - 1] = filler[0][2];
	//		printf("The scratchpad has %llu\n", scratchpad[(meta_info->counter) - 1]);
	//		fflush(NULL);
		}
	}
	else if(meta_info->counter < 10)
	{
		 for(i = 0; i < meta_info->no_of_data_devices; i++)
                 {
                        for(j = 0; j < (int)meta_info->blocksize/sizeof(long long unsigned); j++)
                        {	
				filler[i][j] = (int)filler[i][j]/scratchpad[meta_info->counter - 6];
			}
		}
	}
	
/*	if(meta_info->counter%2 == 0)
	{
		if(stripe_id == 0)
		{
			printf("The data region after step %lu is\n", meta_info->counter);
		}
		printf("Stripe id: %d\n", stripe_id);
		print_region(filler, meta_info);
		if(stripe_id == (meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 2))
		{
			printf("\n");
		}
	}
*/
	for(i = 0; i < meta_info->prefetch_before; i++)
        {
		free(b_filler[i]);
	}
	for(i = 0; i < meta_info->prefetch_after; i++)
        {
		free(a_filler[i]);
	}
	free(a_filler);
	free(b_filler);
	free(filler);

	if((rand()%2 != 0) && (meta_info->counter != 0) &&  (failure < 2))
	{
		while(1)
	        {
	                eraser = (rand()%30);
	                if(eraser < meta_info->total_devices)
	                {
				if(failure == 1)
				{
					eraser = (eraser + 5)%meta_info->total_devices;	
				}
	                        erase(eraser);
	                        erased_device(eraser);
				failure++;
	                        break;
	                }
		}
	}
}

void typecast_mem(meta * meta_info)
{
	int i, j, k;
	//if(meta_info->counter == 0)
        {
		memcast = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices*(meta_info->no_of_stripes - meta_info->prefetch_before - meta_info->prefetch_after - 1));
	}


	k = 0;
        for(i = 0; i < (meta_info->no_of_stripes - meta_info->prefetch_before - meta_info->prefetch_after - 1); i++)
        {
               for(j = 0; j < meta_info->no_of_data_devices; j++)
               {
                       memcast[k + j] = (long long unsigned*)mem[k + j];
               }
               k += meta_info->no_of_data_devices;
        }
	
}

void retrieve_values(void * args, meta * meta_info)
{
	//typecast_mem(meta_info);

	
	if(meta_info->counter == 9)
	{
		meta_info->end = true;
	}
	
	//free(memcast);
} 

void open_data_shelves(int total_devices, long long unsigned mem_allocate)
{
        int i, res;
        char buf[5];
        shelf_names = (char **)malloc(sizeof(char *)*total_devices);
        for(i = 0; i < total_devices; i++)
        {
                shelf_names[i] = (char *)malloc(sizeof(char)*30);
                strcpy(shelf_names[i], MMAP_DIR);
                snprintf(buf, 5, "%d", i);
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
                res = ftruncate(shelf_descriptor[i], (long long unsigned)mem_allocate/total_devices);
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
               data_shelf[i] = (char *)mmap(0, (long long unsigned)mem_allocate/total_devices, PROT_READ|PROT_WRITE, MAP_SHARED, shelf_descriptor[i], 0);
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

void alloc_data_shelves(int total_devices, long long unsigned mem_allocate)
{
	int i;	
        for(i = 0; i < total_devices; i++)
        {
               data_shelf[i] =(char *)malloc((long long unsigned)mem_allocate/total_devices);
	}
}

void alloc_meta_shelf(long long unsigned pad, int total_devices)
{
	data_shelf[total_devices] = (char *)malloc((sizeof(meta) + pad + sizeof(char)*64));
}

void open_meta_shelf(long long unsigned pad, int total_devices)
{
        int res;
      
        meta_descriptor = open(METAfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(meta_descriptor < 0)
        {
                 error("Shelf creation failed\n");
        }
#ifdef DEBUG
debug("Meta shelf created\n");
#endif
        res = ftruncate(meta_descriptor, (sizeof(meta) + pad + sizeof(char)*64));
        if (res < 0)
        {
                error("ftruncate failed due to invalid meta_info->devicesize");
        }
#ifdef DEBUG
debug("Meta shelf truncated\n");
#endif

        data_shelf[total_devices] = (char *)mmap(0, (sizeof(meta) + pad + sizeof(char)*64), PROT_READ|PROT_WRITE, MAP_SHARED, meta_descriptor, 0);
        if(data_shelf[total_devices] == NULL)
        {
                error("Couldn't allocate memory for meta lfs\n");
        }
#ifdef DEBUG
debug("Meta shelf mapped\n");
#endif
        if(close(meta_descriptor) == -1)
        {
                error("File close failed\n");
        }
#ifdef DEBUG
debug("Meta shelf closed\n");
#endif
}


int main(int argc, char*argv[])
{
	int crasher;
	clock_t begin, end;

	int total_devices;
	int no_of_data_devices;
	int no_of_coding_devices;
	long long unsigned mem_allocate;
	long long unsigned pad;
	int i;
	
	fk = 1;

	sscanf(argv[1], "%llu", &mem_allocate);
	sscanf(argv[2], "%d", &no_of_data_devices);
	sscanf(argv[4], "%d", &no_of_coding_devices);
	total_devices = no_of_coding_devices + no_of_data_devices;
	shelf_descriptor = (int *)malloc(sizeof(int)*total_devices);
	data_shelf = (char **)malloc(sizeof(char *)*(total_devices + 1));

	sscanf(argv[8], "%llu", &pad);
	sscanf(argv[9], "%d", &crasher);                                             /*Number of coding(parity) devices*/

#ifdef DRAM
alloc_meta_shelf(pad, total_devices);
alloc_data_shelves(total_devices, mem_allocate);
#endif

#ifdef NVRAM
	open_meta_shelf(pad, total_devices);
	open_data_shelves(total_devices, mem_allocate);
#endif
	
	if(crasher == 0)
        {
		failure = 0;
        }
	else if(crasher == 1)
	{
		failure = 2;
	}
	begin = clock();
	mem = initialize_raid6_parameters(argc, argv, data_shelf, (total_devices + 1));
	end = clock();
	print_configs();
	fflush(NULL);

	if(crasher == 1)
	{
		long long unsigned * scratchpad;
	        scratchpad = (long long unsigned *)get_scratchpad();
		fk = scratchpad[0];
	}

	printf("The time for initializing the RAID-6 parameters is %f seconds.\n", calculate_time(begin, end));
	fflush(NULL);
	

	i = 0;
	begin = clock();
	while(i < (meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 1))
        {i++;}
	end = clock();
	tl = calculate_time(begin,end);

	begin = clock();
	run(&iterate_compute, NULL, &retrieve_values, NULL);
	end = clock();
	printf("The time for execution is %f seconds.\n", calculate_time(begin, end));
	fflush(NULL);
	
	
	destroy_raid6_parameters();
	free(data_shelf);
	//free(shelf_descriptor);
	return 0;

}
