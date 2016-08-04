//raid6/src/r6.c
//Author: Onkar Patil
/*This file implements all the functionalities and algorithms
necessary for enabling TransHPC on persistent memory. It has 
user facing as well as internal functions along with some 
supplementary functions for testing and experimental purpose.*/

/*Including all the header and library files that are required
for implementing TransHPC*/

#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "r6_utils.h"

/*The globals exported from r6_utils.h
Check raid6/include/r6_utils.h*/

stripe *data_stripe;
stripe **free_band_gap;
meta *meta_info;
meta *meta_lfs;

int *bitmatrix;
char **data_region;
int **smart_schedule;
int **dumb_schedule;
int ***schedule_cache;
bool crashRecover;
char ** temp_buf;
char **data;
char **coding;
double tl;

void print_configs()																	/*Print all the configurations of TransHPC*/
{
	printf("For wordsize %llu and %d number of words per block\n", meta_info->wordsize, meta_info->no_of_words_in_block);
	printf("No. of data devices: %d.\n", meta_info->no_of_data_devices);
	printf("Size of each device: %llu bytes.\n", meta_info->devicesize);
	printf("Size of data devices: %llu bytes.\n", meta_info->devicesize*meta_info->no_of_data_devices);
	printf("No. of coding devices: %d.\n", meta_info->no_of_parity_devices);
	printf("Size of coding devices: %llu bytes.\n", meta_info->devicesize*meta_info->no_of_parity_devices);
	printf("Total size of all devices: %llu bytes.\n", meta_info->devicesize*(meta_info->no_of_data_devices + meta_info->no_of_parity_devices));
	printf("Total memory available: %llu bytes.\n", meta_info->memory_allocate);
	printf("Total number of stripes: %d\n", (int)((meta_info->devicesize)/meta_info->blocksize));
	printf("Stripesize: %llu bytes.\n", meta_info->stripesize);
	printf("Size of data stripe: %llu bytes\n", meta_info->datastripesize);

}

void print_stats()																	/*Print the statistics for encoding and decoding*/
{
	int i;
	double stats[3];
	jerasure_get_stats(stats);
	printf("The stats for jerasure operations are:\nXOR\tMultiply\tCopied (bytes)\n");
	for(i = 0; i < 3; i++)
	{
		printf("%f ", stats[i]);
	}
	printf("\n");
}

void erase(int shelf_id)																/*Wipe the data shelf to simulate an erasure*/
{
	memset(meta_info->data_shelf[shelf_id], 0, meta_info->devicesize);
	printf("Erasure at shelf %d\n", shelf_id);
}

void erased_device(int shelf_id)															/*Keep track of the failed devices*/
{
	int i;
	for(i = 0; i < (meta_info->no_of_parity_devices + 1); i++)
	{
		if(meta_info->erasures[i] == shelf_id)
		{
			break;
		}
		else if(meta_info->erasures[i] == -1)
		{

			meta_info->erasures[i] = shelf_id;
			meta_lfs->erasures[i] = meta_info->erasures[i];
#ifdef DEBUG
			debug("erasure logged");
#endif
			break;
		}
	}
	if(i == (meta_info->no_of_parity_devices + 1))
	{
		printf("More than %d device failures. Cannot Recover.\n", meta_info->no_of_parity_devices);
	}
}

void destroy_raid6_parameters()																/*Free the memory allocated for all TransHPC parameters*/
{
	int i,j;

	if(data_stripe == NULL || bitmatrix == NULL || dumb_schedule == NULL || smart_schedule == NULL || schedule_cache == NULL || data_region == NULL)
	{
		error("Trying to destroy a NULL pointer\n");
	}
	for(i = 0; i < meta_info->no_of_stripes; i++)
	{
		for(j = 0; j < meta_info->no_of_data_devices; j++)
		{
			free(data_stripe[i].mapped_base[j]);
		}
		free(data_stripe[i].base);
		free(data_stripe[i].mapped_base);
	}


	free(data_stripe);
	for(i = 0; i < meta_info->total_devices; i++)
	{
		free(temp_buf[i]);
	}
	free(temp_buf);

	free(data);
	free(coding);
#ifdef DEBUG
	debug("freed datastripe\n");
#endif
	free(bitmatrix);
#ifdef DEBUG
	debug("freed bitmatrix\n");
#endif
	jerasure_free_schedule(dumb_schedule);
	jerasure_free_schedule(smart_schedule);
#ifdef DEBUG
	debug("freed smart_schedule\n");
#endif
	jerasure_free_schedule_cache(meta_info->no_of_data_devices, meta_info->no_of_parity_devices, schedule_cache);
#ifdef DEBUG
	debug("freed schedule cache\n");
#endif
	free(free_band_gap);
#ifdef DEBUG
	debug("freed the band gap\n");
#endif
	free(data_region);
#ifdef DEBUG
	debug("freed data\n");
#endif
}

double calculate_time(clock_t begin, clock_t end)													/*Calculate the time using CPU cycles*/
{
	double total;
	total = ((double)(end - begin)/CLOCKS_PER_SEC);
	return total;
}

void initialize_data_shelves(char ** shelves)                                                                                                          /*Initialize the data shelves from the anchors provided from the application*/
{
	meta_info->data_shelf = shelves;
	meta_lfs->data_shelf = meta_info->data_shelf;
#ifdef DEBUG
	debug("Initialized data devices\n");
#endif
}

void initialize_data_stripes()                                                                                                          		/*Initialize the data stipes to the data shelf offsets*/
{
	int i, j;
	int offset = 0;
	char * eyecatcher;
	char * dest;
	eyecatcher = "!!@@This is an eyecatcher of 64 bytes. Do not screw this up.@@!!";
	data_stripe = (stripe *)malloc(sizeof(stripe)*meta_info->no_of_stripes);
#ifdef DEBUG
	debug("created datastripe\n");
#endif

	if(data_stripe == NULL)
	{
		error("Couldn't allocate memory for the data stripes\n");
	}

	for(i = 0; i < meta_info->no_of_stripes; i++)
	{
		data_stripe[i].base = (char **)malloc(sizeof(char*)*meta_info->total_devices);								/*Allocate the pointers for data stripes*/
		data_stripe[i].mapped_base = (char **)malloc(sizeof(char*)*meta_info->no_of_data_devices);
		if(data_stripe[i].base == NULL)
		{
			error("Couldn't allocate memory for the data stripe %d\n", i);
		}
		if(!crashRecover)															/*Mark all stripes as empty*/
		{
			data_stripe[i].empty = true;
		}
		else
		{
			data_stripe[i].empty = false;
		}
	}
	for(i = 0; i < meta_info->no_of_stripes; i++)
	{
		for (j = 0; j < meta_info->no_of_data_devices; j++)
		{
			data_stripe[i].mapped_base[j] = (char *)malloc(sizeof(char)*meta_info->blocksize);
		}
	}
#ifdef DEBUG
	debug("Datastripe initialiazed\n");
#endif
	for (i = 0; i < meta_info->no_of_stripes; i++)
	{
		for(j = 0; j < meta_info->total_devices; j++)
		{
			data_stripe[i].base[j] = (char *)(meta_info->data_shelf[j] + offset);
			if(!crashRecover)
			{
				dest = (char *)data_stripe[i].base[j];										/*Add an eye-catcher to the beginning and end of the block*/
				memcpy(dest, eyecatcher, sizeof(char)*64);
				dest += (meta_info->blocksize - sizeof(char)*64);
				memcpy(dest, eyecatcher, sizeof(char)*64);
			}
		}
		offset += meta_info->blocksize;
	}
#ifdef DEBUG
	debug("Eyecatchers in place\n");
#endif
	temp_buf = (char **)malloc(sizeof(char*)*meta_info->total_devices);
	if(temp_buf == NULL)
	{
		error("Could not allocate memory for temporary stripe\n");
	}
#ifdef DEBUG
	debug("temporary buf allocated\n");
#endif
	for(i = 0; i < meta_info->total_devices; i++)
	{
		temp_buf[i] = (char *)malloc(sizeof(char)*meta_info->blocksize);
		if(temp_buf[i] == NULL)
		{
			error("Could not create memory for temp_buf\n");
		}
	}
#ifdef DEBUG
	debug("temporary buf created\n");
#endif
	free_band_gap = (stripe **)malloc(sizeof(stripe*)*(meta_info->prefetch_before + meta_info->prefetch_after + 1));			/*Allocate memory for the free band gap*/

	data_region = (char **)malloc(sizeof(char*)*((meta_info->no_of_stripes - meta_info->prefetch_before - meta_info->prefetch_after - 1)*meta_info->no_of_data_devices));
	/*Allocate pointers for mapping the data region*/
#ifdef DEBUG
	debug("Data region allocated\n");
#endif
	if(data_region == NULL)
	{
		error("Couldn't allocate memory for the data region\n");
	}
	meta_info->data_region_size = meta_info->no_of_data_devices*(meta_info->no_of_stripes - meta_info->prefetch_before - meta_info->prefetch_after - 1)*meta_info->blocksize;
	meta_lfs->data_region_size = meta_info->data_region_size;

	data = (char**)malloc(sizeof(char *)*(meta_info->no_of_data_devices));									/*Allocate data and coding pointers Jerasure 2.0*/
	coding = (char**)malloc(sizeof(char *)*(meta_info->no_of_parity_devices));

}

void set_free_band_gap(int begin)														/*Set the Free Band Gap to the begin index stripe*/
{
	int i;
	for(i = 0; i < (meta_info->prefetch_after + meta_info->prefetch_before + 1); i++)
	{
		if(meta_info->counter%2 == 0)
		{
			free_band_gap[i] = &data_stripe[i + begin];
		}
		else
		{
			free_band_gap[i] = &data_stripe[begin - i];
		}
	}
#ifdef DEBUG
	debug("free band gap defined\n");
#endif

}

void calculate_parity(char ** ca_stripe)                                                                                                       /*Calculate the parity for the given data stripe*/
{
	int i;
	for(i = 0; i < meta_info->no_of_data_devices; i++)
	{
		data[i] = (char *)ca_stripe[i];
	}
	for(i = meta_info->no_of_data_devices; i < meta_info->total_devices; i++)
	{
		coding[i - meta_info->no_of_data_devices] = (char *)ca_stripe[i];
	}

	if(meta_info->no_of_parity_devices == 2)
	{
#ifdef DEBUG
		debug("All devices schedule encoded for the stripe\n");
#endif
		jerasure_schedule_encode(meta_info->no_of_data_devices, meta_info->no_of_parity_devices, meta_info->wordsize, smart_schedule, data, coding, meta_info->blocksize, sizeof(long));
		//	print_stats();															/*Encode the stripe using smart schedule for 2 parity devices*/
	}
	else
	{
#ifdef DEBUG
		debug("All devices bitmatrix encoded for the stripe\n");
#endif
		jerasure_bitmatrix_encode(meta_info->no_of_data_devices, meta_info->no_of_parity_devices, meta_info->wordsize, bitmatrix, data, coding, meta_info->blocksize, sizeof(long));
		//	print_stats();															/*Encode the stripe using the bitmatrix for other than 2 parity devices*/
	}

}

void recover_device(char ** re_stripe)                                                                                            		/*Recover a data device after erasures for a given stripe*/
{
	int i;
	for(i = 0; i < meta_info->no_of_data_devices; i++)
	{
		data[i] = (char *)re_stripe[i];
	}
#ifdef DEBUG
	debug("created the data pointer for recovery\n");
#endif
	for(i = meta_info->no_of_data_devices; i < meta_info->total_devices; i++)
	{
		coding[i - meta_info->no_of_data_devices] = (char *)re_stripe[i];
	}
#ifdef DEBUG
	debug("created the coding pointer for recovery\n");
#endif

	if(meta_info->no_of_parity_devices == 2)
	{
#ifdef DEBUG
		debug("All devices cache decoded for the stripe\n");
#endif
		jerasure_schedule_decode_cache(meta_info->no_of_data_devices, meta_info->no_of_parity_devices, meta_info->wordsize, schedule_cache, meta_info->erasures, data, coding, meta_info->blocksize, sizeof(long));
//		print_stats();															/*Decode the stripe using the schedule cache for 2 parity devices*/
	}
	else
	{
#ifdef DEBUG
		debug("All devices lazy decoded for the stripe\n");
#endif

		jerasure_schedule_decode_lazy(meta_info->no_of_data_devices, meta_info->no_of_parity_devices, meta_info->wordsize, bitmatrix, meta_info->erasures, data, coding, meta_info->blocksize, sizeof(long), 1);
//		print_stats();															/*Decode the stripe using the bitmatrix for other than 2 parity devices*/
	}

}

void create_data_region()															/*Create the data region*/
{
	int i, j, k;

	k = 0;
	for(i = 0; i < (meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 1); i++)
	{
		for(j = 0; j < meta_info->no_of_data_devices; j++)
		{
			if(meta_info->counter%2 == 0)
			{
				data_region[k + j] = data_stripe[i].base[j];
			}
			else
			{
				data_region[k + j] = data_stripe[i + meta_info->prefetch_after + meta_info->prefetch_before + 1].base[j];
			}
		}
		k += meta_info->no_of_data_devices;
	}
#ifdef DEBUG
	debug("Data region created\n");
#endif
}

meta* get_meta_info()																/*Retrieve the meta information pointer in DRAM */
{
	return meta_info;
}

void * get_scratchpad()																/*Get the scratchpad from the meta data shelf*/
{
	void * scratch;
	scratch = ((char*)meta_lfs + sizeof(meta));
	return scratch;
}

void initialize_jerasure()															/*Initialize the jerasure data structures*/
{
	int i;

	bitmatrix = liberation_coding_bitmatrix(meta_info->no_of_data_devices, meta_info->wordsize);                                            /*Creating a bitmatrix for the meta_info->no_of_data_devices and meta_info->wordsize*/
#ifdef DEBUG
	debug("Bitmatrix created\n");
#endif
	if(bitmatrix == NULL)
	{
		error("Could not create bitmatrix for %d devices and %d Word size\n", meta_info->no_of_data_devices, meta_info->wordsize);
	}

	if(meta_info->no_of_parity_devices == 2)
	{
		schedule_cache = jerasure_generate_schedule_cache(meta_info->no_of_data_devices, meta_info->no_of_parity_devices, meta_info->wordsize, bitmatrix, 1);    /*Convert bitmatrix into a schedule of coding operations*/
	}
	dumb_schedule = jerasure_dumb_bitmatrix_to_schedule(meta_info->no_of_data_devices, meta_info->no_of_parity_devices, meta_info->wordsize, bitmatrix);
	smart_schedule = jerasure_smart_bitmatrix_to_schedule(meta_info->no_of_data_devices, meta_info->no_of_parity_devices, meta_info->wordsize, bitmatrix);   /*Convert bitmatrix into a schedule of coding operations*/
#ifdef DEBUG
	debug("Schedule and Schedule cache created\n");
#endif
	if(!crashRecover)
	{
		for(i = 0; i <= meta_info->no_of_parity_devices; i++)
		{
			meta_info->erasures[i] = -1;												/*Initialize the erasures data structures*/
			meta_lfs->erasures[i] = meta_info->erasures[i];
		}
	}
#ifdef DEBUG
	debug("Erasures assigned\n");
#endif

}


void initialize_meta(char * shelf, long long unsigned pad)													/*Initialize the meta data shelf from the anchor passed from the application*/
{
	char * eyecatcher;
	char * temp;
	eyecatcher = "@!!This is a eyecatcher for the scratchpad. Do not overwrite.!!@";


	meta_lfs = (meta *)shelf;

	if(meta_lfs == NULL)
	{
		error("Couldn't allocate memory for meta lfs\n");
	}
#ifdef DEBUG
	debug("Meta shelf mapped\n");
#endif
	meta_info = (meta *)malloc(sizeof(meta));												/*Allocate the meta information pointer in DRAM*/
	if(!crashRecover)
	{
		meta_info->end = false;
		meta_lfs->end = false;

		meta_info->pad = pad;
		meta_lfs->pad = pad;

		temp = (char *)((char *)meta_lfs + (sizeof(meta) + pad));
		memcpy(temp, eyecatcher, (sizeof(char)*64));
#ifdef DEBUG
		debug("Meta shelf eyecatcher in place\n");
#endif
		meta_info->scratchpad_offset = sizeof(meta);
		meta_lfs->scratchpad_offset = sizeof(meta);
	}
	else
	{
		memcpy(meta_info, meta_lfs, sizeof(meta));
#ifdef DEBUG
		debug("Meta shelf copied\n");
#endif
	}
}

char ** initialize_raid6_parameters(int argc, char*argv[], char ** shelves, int all_devices)                                                   /*Initialize the RAID-6 parameters*/
{
	int i;
	long long unsigned pad;
	int crashflag;

	if(argc != 10)
	{
		error("USAGE: r6_emu <memory_allocate> <no. of data devices> <word size for the code> <no. of coding devices> <no. of words in block> <prefetch before> <prefetch after> <scratch_pad mem> <crash_recover?>\n");
	}

	if(sscanf(argv[8], "%llu", &pad) == 0 || (pad < 0))  	                                    					      /*Number of coding(parity) devices*/
	{
		printf("%d", argv[8]);
		error("USAGE: The scratchpad should be a positive non-zero integer \n");
	}

	if(sscanf(argv[9], "%d", &crashflag) == 0 || (crashflag < 0 || crashflag > 1))		                                             /*Number of coding(parity) devices*/
	{
		error("USAGE: The crashflag should be a 0 or 1\n");
	}

	if(crashflag == 1)														     /*Recover the TransHPC framework from the crash*/
	{
		crash_recover(pad, shelves, all_devices);
	}
	else
	{
		initialize_meta(shelves[all_devices - 1], pad);

		if(sscanf(argv[1], "%llu", &meta_info->memory_allocate) == 0 || (meta_info->memory_allocate <= 0))                            /*No. of devices*/
		{
			error("USAGE: The memory to be allocated has to be a positive integer greater than zero\n");
		}
		meta_lfs->memory_allocate = meta_info->memory_allocate;

		if(sscanf(argv[2], "%d", &meta_info->no_of_data_devices) == 0 || (meta_info->no_of_data_devices <= 0))                       /*No. of devices*/
		{
			error("USAGE: The no. of devices have to be a positive integer greater than zero\n");
		}
		meta_lfs->no_of_data_devices = meta_info->no_of_data_devices;

		if(sscanf(argv[3], "%llu", &meta_info->wordsize) == 0 || (meta_info->wordsize < 0) || (meta_info->wordsize < meta_info->no_of_data_devices)) /*Word size for the code*/
		{
			error("USAGE: The word size should be a positive non-zero prime integer greater than no. of devices\n");
		}
		meta_lfs->wordsize = meta_info->wordsize;

		if(sscanf(argv[4], "%d", &meta_info->no_of_parity_devices) == 0 || (meta_info->no_of_parity_devices < 0))                    /*Number of coding(parity) devices*/
		{
			error("USAGE: The number of coding devices should be a positive non-zero integer\n");
		}
		meta_lfs->no_of_parity_devices = meta_info->no_of_parity_devices;

		if(sscanf(argv[5], "%d", &meta_info->no_of_words_in_block) == 0 || (meta_info->no_of_words_in_block < 0))                    /*Number of coding(parity) devices*/
		{
			error("USAGE: The number of words in a block should be a positive non-zero integer\n");
		}

		if(sscanf(argv[6], "%d", &meta_info->prefetch_before) == 0 || (meta_info->prefetch_before < 0))                              /*Number of coding(parity) devices*/
		{
			error("USAGE: The prefetch before should be a positive non-zero integer\n");
		}

		if(sscanf(argv[7], "%d", &meta_info->prefetch_after) == 0 || (meta_info->prefetch_after < 0))                                /*Number of coding(parity) devices*/
		{
			error("USAGE: The prefetch after should be a positive non-zero integer\n");
		}


		if(meta_info->prefetch_after != meta_info->prefetch_before)
		{
			error("USAGE: The prefetch after should be equal to prefetch before\n");
		}

		initialize_jerasure();

		meta_lfs->no_of_words_in_block = meta_info->no_of_words_in_block;
		meta_lfs->prefetch_before = meta_info->prefetch_before;
		meta_lfs->prefetch_after = meta_info->prefetch_after;

		meta_info->total_devices = meta_info->no_of_data_devices + meta_info->no_of_parity_devices;                     	     /*Total number of devices*/
		meta_lfs->total_devices = meta_info->total_devices;

		for (i = 2; i*i <= meta_info->wordsize; i++)		                                                                     /*Check whether meta_info->wordsize is prime*/
		{
			if(meta_info->wordsize % i == 0)
			{
				error("Word size is not prime\n");
			}
		}
		meta_info->wordsize_long_aligned = sizeof(long)*meta_info->wordsize;							     /*Long aligned word size*/
		meta_lfs->wordsize_long_aligned = meta_info->wordsize_long_aligned;

		meta_info->devicewords = (unsigned long)((meta_info->memory_allocate)/(meta_info->wordsize_long_aligned*meta_info->total_devices)); /*Number of words on each device*/
		if(meta_info->devicewords == 0 || (meta_info->no_of_words_in_block > meta_info->devicewords))
		{
			error("Memory allocate should be bigger\n");
		}
		meta_lfs->devicewords = meta_info->devicewords;
		meta_info->devicesize = meta_info->wordsize_long_aligned*meta_info->devicewords;                                      	     /*Size of each device*/
		meta_lfs->devicesize = meta_info->devicesize;

		meta_info->blocksize = meta_info->wordsize_long_aligned*meta_info->no_of_words_in_block;				     /*Size of each data block*/
		meta_lfs->blocksize = meta_info->blocksize;

		meta_info->stripesize = meta_info->blocksize*meta_info->total_devices;							     /*Size of each data stripe*/
		meta_lfs->stripesize = meta_info->stripesize;

		meta_info->datastripesize = meta_info->blocksize*meta_info->no_of_data_devices;						     /*size of the data blocks in the stripe*/
		meta_lfs->datastripesize = meta_info->datastripesize;

		meta_info->no_of_stripes = (int)(meta_info->devicesize/meta_info->blocksize);						     /*Total number of stripes*/
		meta_lfs->no_of_stripes = meta_info->no_of_stripes;
#ifdef DEBUG
		debug("Size globals assigned\n");
#endif

		initialize_data_shelves(shelves);


		initialize_data_stripes();


		create_data_region();

		meta_info->counter = 0;
		meta_lfs->counter = meta_info->counter;

		set_free_band_gap(0);

		meta_info->free_band_gap_id = 0;
		meta_lfs->free_band_gap_id = meta_info->free_band_gap_id;

		crashRecover = false;

#ifdef DEBUG
		debug("All parameters initialized\n");
#endif
	}
	return data_region;
}

size_t get_data_region_size()														/*Give the size of the entire data region*/
{
	return meta_info->data_region_size;
}

size_t get_stripesize()															/*Give the size of each block*/
{
	return meta_info->stripesize;
}

int stripe_locator(int stripeid)													/*Locate the stripe based on the ID and the step number*/
{
	if(meta_info->counter%2 == 0)
	{
		return stripeid + meta_info->prefetch_before + meta_info->prefetch_after + 1;
	}
	else
	{
		return stripeid - meta_info->prefetch_before - meta_info->prefetch_after - 1;
	}
	return -1;
}

char ** map_stripe(int stripeid)			                                                                                /*Map the data stripe specified by the stripeid index*/
{
	int i, j;
	bool flag = false;
	bool recover = false;
	stripeid = stripe_locator(stripeid);
#ifdef DEBUG
	debug("Stripe located\n");
#endif
	if(stripeid == -1)
	{
		error("Could not locate stripe\n");
	}

	for(i = 0; i < meta_info->total_devices; i++)
	{
		for(j = 0; j < meta_info->no_of_parity_devices; j++)
		{
			if(i == meta_info->erasures[j])											/*Recover any erasures*/
			{
				flag = true;
				if(!recover)
				{
					recover = true;
				}
			}
		}
		if(flag || (data_stripe[stripeid].empty && meta_info->counter == 0))
		{
			memset(temp_buf[i], 0, meta_info->blocksize);
			flag = false;
		}
		else
		{
			memcpy(temp_buf[i], data_stripe[stripeid].base[i], meta_info->blocksize);
		}
	}
#ifdef DEBUG
	debug("Copied the data into DRAM\n");
#endif
	if(recover && !data_stripe[stripeid].empty)
	{
		recover_device(temp_buf);
#ifdef DEBUG
		debug("recovered data from device\n");
#endif
	}

	for(i = 0; i < meta_info->no_of_data_devices; i++)
	{
		memcpy(data_stripe[stripeid].mapped_base[i], temp_buf[i], meta_info->blocksize);
	}
#ifdef DEBUG
	debug("stripe assigned and mapped\n");
#endif

	if(data_stripe[stripeid].empty)
	{
		data_stripe[stripeid].empty = false;
	}

	return data_stripe[stripeid].mapped_base;

}

void update_stripe(int stripeid)													/*Updates the stripe indexed by stripeid based on the movement of the band gap */
{
	int i, j;
	bool flag = false;

	stripeid = stripe_locator(stripeid);

#ifdef DEBUG
	debug("Stripe located\n");
#endif

	for(i = 0; i < meta_info->no_of_data_devices; i++)
	{
		memcpy(temp_buf[i], data_stripe[stripeid].mapped_base[i], meta_info->blocksize);
	}
	calculate_parity(temp_buf);													/*Calculate the parity of the updated stripe*/

#ifdef DEBUG
	debug("parity calculated for the src data\n");
#endif
	for(i = 0; i < meta_info->total_devices; i++)
	{
		for(j = 0; j < meta_info->no_of_parity_devices; j++)
		{
			if(i == meta_info->erasures[j])
			{
				flag = true;
			}
		}
		if(!flag)
		{
			memcpy(free_band_gap[0]->base[i], temp_buf[i], meta_info->blocksize);
			free_band_gap[0]->empty = false;
		}
		flag = false;
	}
#ifdef DEBUG
	debug("Stripe updated\n");
#endif
}

void run(void (*proc1)(void*, char**, meta*, int, char ***, char ***), void* args1, void (*proc2)(void*, meta *), void* args2)		/*Execute the 2 procedures passed to run and move the free band gap*/
{
	int i, j;
	int stripe_order;
	char **cur_stripe;
	char ***before;
	char ***after;

	before = NULL;
	after = NULL;

	clock_t begin, end;


	if(meta_info->prefetch_before > 0)
	{
		before = (char ***)malloc(sizeof(char **)*meta_info->prefetch_before);
	}
	if(meta_info->prefetch_after > 0)
	{
		after = (char ***)malloc(sizeof(char **)*meta_info->prefetch_after);
	}

	for(i = 0 ; i < meta_info->prefetch_before; i++)
	{
		before[i] = (char **)malloc(sizeof(char *)*meta_info->no_of_data_devices);
	}
	for(i = 0 ; i < meta_info->prefetch_after; i++)
	{
		after[i] = (char **)malloc(sizeof(char *)*meta_info->no_of_data_devices);
	}

#ifdef DEBUG
	debug("prefetch pointers assigned\n");
#endif

	meta_info->end = false;
	meta_lfs->end = meta_info->end;


	if(proc1 == NULL)
	{
		error("No subgrid function passed\n");
	}

	while(!meta_info->end)																/*Run the loop till indicated to stop*/
	{
		if(!crashRecover)
		{
			i = 0;
		}
		else
		{
			if(meta_info->counter%2 == 0)
			{
				i = meta_info->free_band_gap_id;
			}
			else
			{
				i = meta_info->no_of_stripes - 1 - meta_info->free_band_gap_id;
			}
		}
#ifdef DEBUG
		debug("stripe order set and starting bandgap loop\n");
#endif

		begin = clock();
		while((i < (meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 1)))					/*Move the free band gap*/
		{
			if(meta_info->counter%2 == 0)
			{
				stripe_order = i;
			}
			else
			{
				stripe_order = meta_info->no_of_stripes - i - 1;
			}
			if(i == 0)
			{
#ifdef DEBUG
				debug("First stripe\n");
#endif
				cur_stripe = map_stripe(stripe_order);
				if(meta_info->counter%2 != 0 && meta_info->prefetch_before > 0)
				{
					for(j = 1; j <= meta_info->prefetch_before; j++)
					{
						before[j -1] = map_stripe(stripe_order - j);
					}
				}
				else if(meta_info->counter%2 == 0 && meta_info->prefetch_after > 0)
				{
					for(j = 1; j <= meta_info->prefetch_after; j++)
					{
						after[j - 1] = map_stripe(stripe_order + j);
					}
				}
			}
			else if(meta_info->counter%2 == 0 && i < (meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 2))
			{
#ifdef DEBUG
				debug("Mid stripes in even step\n");
#endif
				for(j = meta_info->prefetch_before - 1; j > 0; j--)
				{
					if(j < i)
					{
						if(!crashRecover)
						{
							before[j] = before[j-1];
						}
						else
						{
							before[j] = map_stripe(stripe_order - 1 - j);
						}
					}
				}
				before[0] = map_stripe(stripe_order - 1);
				if(!crashRecover)
				{
					cur_stripe = after[0];
				}
				else
				{
					cur_stripe = map_stripe(stripe_order);
				}
				for(j = 0; j < meta_info->prefetch_after - 1; j++)
				{
					if(!crashRecover)
					{
						after[j] = after[j+1];
					}
					else
					{
						after[j] = map_stripe(stripe_order + j + 1);
					}
				}
				after[j] = map_stripe(stripe_order + j + 1);
			}
			else if(meta_info->counter%2 != 0 && i < (meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 2))
			{
#ifdef DEBUG
				debug("Mid stripes in odd step\n");
#endif
				for(j = meta_info->prefetch_after - 1; j > 0; j++)
				{
					if(j < i)
					{
						if(!crashRecover)
						{
							after[j] = after[j-1];
						}
						else
						{
							after[j] = map_stripe(stripe_order + 1 + j);
						}
					}
				}
				after[0] = map_stripe(stripe_order + 1);
				if(!crashRecover)
				{
					cur_stripe = before[0];
				}
				else
				{
					cur_stripe = map_stripe(stripe_order);
				}
				for(j = 0; j > meta_info->prefetch_before - 1; j++)
				{
					if(!crashRecover)
					{
						before[j] = before[j-1];
					}
					else
					{
						before[j] = map_stripe(stripe_order - 1 - j);
					}
				}
				before[j] = map_stripe(stripe_order - j - 1);

			}
			else if(meta_info->counter%2 == 0 && i < (meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 1))
			{
#ifdef DEBUG
				debug("Final stripe in even step\n");
#endif
				for(j = meta_info->prefetch_before - 1; j > 0; j--)
				{
					if(j < i)
					{
						if(!crashRecover)
						{
							before[j] = before[j-1];
						}
						else
						{
							before[j] = map_stripe(stripe_order - 1 - j);
						}
					}
				}
				before[0] = map_stripe(stripe_order - 1);
				if(!crashRecover)
				{
					cur_stripe = after[0];
				}
				else
				{
					cur_stripe = map_stripe(stripe_order);
				}
				for(j = 0; j < meta_info->prefetch_after - 1; j++)
				{
					if(!crashRecover)
					{
						after[j] = after[j+1];
					}
					else
					{
						after[j] = map_stripe(stripe_order + 1 + j);
					}
				}
				after[j] = NULL;
			}
			else if(meta_info->counter%2 != 0 && i < (meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 1))
			{
#ifdef DEBUG
				debug("Final stripe in odd step\n");
#endif
				for(j = meta_info->prefetch_after - 1; j > 0; j--)
				{
					if(j < i)
					{
						if(!crashRecover)
						{
							after[j] = after[j-1];
						}
						else
						{
							after[j] = map_stripe(stripe_order + 1 + j);
						}
					}
				}
				after[0] = map_stripe(stripe_order + 1);
				if(!crashRecover)
				{
					cur_stripe = before[0];
				}
				else
				{
					cur_stripe = map_stripe(stripe_order);
				}
				for(j = 0; j < meta_info->prefetch_before - 1; j++)
				{
					if(!crashRecover)
					{
						before[j] = before[j+1];
					}
					else
					{
						before[j] = map_stripe(stripe_order - 1 - j);
					}
				}
				before[j] = NULL;

			}
			proc1(args1, cur_stripe, meta_lfs, stripe_order, before, after);								/*Call the subgrid function*/
#ifdef DEBUG
			debug("Process called\n");
#endif
			update_stripe(stripe_order);													/*Update the stripe and move the freeband gap*/
			if(meta_info->counter%2 == 0 && meta_info->free_band_gap_id != (meta_info->no_of_stripes - meta_info->prefetch_before - meta_info->prefetch_after - 1))
			{
				set_free_band_gap(meta_info->free_band_gap_id + 1);
				meta_info->free_band_gap_id++;
				meta_lfs->free_band_gap_id++;
			}
			else if(meta_info->counter%2 != 0 && meta_info->free_band_gap_id != (meta_info->prefetch_before + meta_info->prefetch_after ))
			{
				set_free_band_gap(meta_info->free_band_gap_id - 1);
				meta_info->free_band_gap_id--;
				meta_lfs->free_band_gap_id--;
			}
			if(crashRecover)
			{
				crashRecover = false;
			}
			i++;
#ifdef DEBUG
			debug("Band Gap moved\n");
#endif
		}
		end = clock();
		printf("The time for this step is %f\n", (calculate_time(begin,end) - tl));
		printf("The time for each stripe in this step is %f\n", (calculate_time(begin,end) - tl)/(meta_info->no_of_stripes - meta_info->prefetch_after - meta_info->prefetch_before - 1));

		create_data_region();

		begin = clock();
		if(proc2 != NULL)															/*Call the control function*/
		{
			proc2(args2, meta_lfs);
		}
		end = clock();
		printf("The time for this end-of-step is %f\n", calculate_time(begin,end));

		char * temp;
		temp = (char *)((char*)meta_lfs + sizeof(meta) + meta_info->pad);

		if(strcmp(temp, "@!!This is a eyecatcher for the scratchpad. Do not overwrite.!!@") != 0)
		{
			error("Scratchpad boundary overwritten");
		}
		temp = NULL;
#ifdef DEBUG
		debug("Second process called\n");
#endif
		meta_info->counter++;
		meta_lfs->counter++;
		if(meta_info->counter%2 == 0)														/*Setup the next step*/
		{
			set_free_band_gap(0);
			meta_info->free_band_gap_id = 0;
			meta_lfs->free_band_gap_id = 0;
		}
		else
		{
			set_free_band_gap(meta_info->no_of_stripes - 1);
			meta_info->free_band_gap_id = meta_info->no_of_stripes - 1;
			meta_lfs->free_band_gap_id = meta_info->no_of_stripes - 1;
		}
		meta_info->end = meta_lfs->end;
#ifdef DEBUG
		debug("End of iterative loop\n");
#endif
		if(crashRecover)
		{
			crashRecover = false;
		}
	}

	for(i = 0 ; i < meta_info->prefetch_before; i++)
	{
		before[i] = NULL;
		free(before[i]);
	}
	for(i = 0 ; i < meta_info->prefetch_after; i++)
	{
		after[i] = NULL;
		free(after[i]);
	}

	free(before);
	free(after);
#ifdef DEBUG
	debug("End of run\n");
#endif
}

void crash_recover(long long unsigned pad, char ** shelves, int all_devices)												/*Recover the TransHPC from a crash*/
{
	crashRecover = true;

#ifdef DEBUG
	debug("Recover flag set\n");
#endif
	initialize_meta(shelves[all_devices - 1], pad);
#ifdef DEBUG
	debug("Recovered meta data\n");
#endif
	initialize_jerasure();
#ifdef DEBUG
	debug("Recovered jerasure\n");
#endif
	initialize_data_shelves(shelves);
#ifdef DEBUG
	debug("Recovered data shelves\n");
#endif
	initialize_data_stripes();
#ifdef DEBUG
	debug("Recovered data stripes\n");
#endif
	create_data_region();
#ifdef DEBUG
	debug("Recovered data region\n");
#endif
	set_free_band_gap(meta_info->free_band_gap_id);
#ifdef DEBUG
	debug("Recovered the state of program\n");
#endif
}
