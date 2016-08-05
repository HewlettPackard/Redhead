//raid6/include/r6_utils.h
//Author: Onkar Patil

/*This header file lists and declares all the necessary
global variables, datatypes and functions required to 
implement TransHPC over persistent memory*/

#define error(...) printf("ERROR: %s\n" , #__VA_ARGS__); fflush(NULL); exit(-1);						/*Error assertion directive*/

#ifdef DEBUG
	#define debug(...) printf("DEBUG: %s line-%d file-%s\n", #__VA_ARGS__, __LINE__, __FILE__); fflush(NULL);		/*Debug assertion directive*/
#endif

/*Including the list of header files to enable all the 
libraries required for TransHPC. It includes the header 
files for the Jerasure 2.0 library as well*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include "jerasure.h"
#include "liberation.h"
#ifndef JERASURE	/*Using Reed-Solomon encoding instead of liberation encoding*/
#include "reed_sol.h"
#endif



/*Stripe is a structure for keeping track of all 
data stripes. It has a base pointer that points to 
all the data blocks in persistent memory, empty 
flag to mark it empty initially and not empty once it
is written and a mapped base pointer that points 
to all the data blocks mapped into DRAM*/

typedef struct Stripe
{
	char ** base;											//Base Pointer to the data blocks in persistent memory
	bool empty;											//Flag to indicate empty or not empty
	char * mapped_base;										//Pointer to the mapped stripe in DRAM
}stripe;

/*Meta is structure for storing all the information
required for working with the Jerasure2.0 library
and also the book keeping required for TransHPC.
At the end of the struct in LFS we provide a 
scratchpad which can be used for storing data from 
previous steps.*/


typedef struct Meta
{
	long long unsigned counter;									//Count of the number of steps taken by the Free Band Gap
	int free_band_gap_id;										//Current position of the top of free band gap
	int no_of_data_devices;										//Total number of data shelves
	long long unsigned wordsize;									//Prime number for the Liberation encoding unit word size
	long long unsigned wordsize_long_aligned;							//Word size multiplied by the size of long
	int no_of_words_in_block;									//Block size divided by the word size long alligned
	int no_of_parity_devices;									//Total number of parity devices
	int no_of_stripes;										//Total number of stripes of the entire data region
	int total_devices;										//Total number of shelves for data and parity
	long long unsigned devicewords;									//Total number of words in the entire data region
	long long unsigned stripesize;									//Size of each entire stripe
	char **data_shelf;										//Anchor to all the data shelves in persitent memory
	long long unsigned datastripesize;								//Size of the data part of the stripe
	long long unsigned devicesize;									//Size of the data shelves
	long long unsigned blocksize;									//Size of each data block
	long long unsigned memory_allocate;								//Total Memory allocated by the application for all data shelves
	long long unsigned data_region_size;								//Cumulative size of all data blocks
	int prefetch_before;										//Number of stripes to be prefetched preceeding the current stripe 
	int prefetch_after;										//Number of stripes to be prefetched proceeding the current stripe
	long long unsigned scratchpad_offset;								//The starting address of the scratchpad
	long long unsigned pad;										//The size of the scratchpad
	bool end;											//The flag to stop the Free Band Gap mover
	int erasures[3];										//Array to keep track of data shelf failures
	char gap_filler[40];										//Padding to make the Meta pointer cache line aligned
}meta;


/*These are the global pointers and flags that help 
easy data access across all the functions in TransHPC
These are reset after a crash to what is in the Metadata
else they are initialized on first use.*/

extern stripe *data_stripe;										//Anchor for all the stripes
extern stripe **free_band_gap;										//Pointer to the top of the free band gap
extern meta * meta_lfs;											//Anchor for the meta information shelf in persistent memory
extern meta * meta_info;										//Anchor for the meta information in DRAM

extern int *matrix;
extern int *bitmatrix;											//the bitmatrix created by Jerasure 2.0 for encoding data
extern int **dumb_schedule;										//the dumb schedule created by Jerasure 2.0
extern int **smart_schedule;										//the smart schedule created by Jerasure 2.0
extern int ***schedule_cache;										//the schedule cache created by Jerasure 2.0
extern bool crashRecover;										//the flag that indicates whether the application is recovering from a crash
extern char ** temp_buf;										//A temporary buffer to move data around
extern char **data;											//Pointers to all the data blocks in the current stripe to be passed to Jerasure2.0
extern char **coding;											//Pointers to all the parity blocks in the current stripe to be passed to Jerasure2.0
extern double tl;
/*User facing functions that help in the use of TransHPC*/

void initialize_raid6_parameters(int argc, char * argv[], char ** shelves, int all_devices);		/*Initialize the RAID-6 parameters*/
void run(void (*proc1)(void*, char*, meta *, int, char *, char **, char **), void* args1, void(*proc2)(void*, meta*), void * args2);	/*Execute the band gap mover and the functions passed to it*/
void destroy_raid6_parameters();									/*Free the memory allocated for the devices*/
meta* get_meta_info();											/*Get the meta info pointer*/


/*Internal Functions to implement the functionalities of
TransHPC*/

void print_configs();											/*Print RAID-6 configs*/
void print_stats();											/*Print the statistics for encoding and decoding*/
void erase(int shelf_id);										/*Erase a device*/
void erased_device(int shelf_id);									/*Keep track of the erased devices*/
double calculate_time(struct timespec begin, struct timespec end);					/*Calculate the CPU time between begin and end*/
void * get_scratchpad();										/*get the start anchor of the scratchpad*/
void calculate_parity(char ** ca_stripe);								/*Calculate the parity for the given data stripe*/
void recover_device(char ** re_stripe);									/*Recover a data device after erasures*/
void initialize_jerasure();										/*Initialize all the parameters and data structures required by Jerasure 2.0*/
void initialize_meta(char * shelf, long long unsigned pad);						/*Initialize the meta info unless it already exists after a crash*/
void initialize_data_shelves(char ** shelves);								/*Initialize the data shelves unless they already exist after a crash*/
void initialize_data_stripes();										/*Initialize the data stipes*/
void set_free_band_gap(int begin);									/*Set the Free Band Gap on a particular stripe*/
void create_data_region();										/*Resolve and access data blocks in the control function only*/
size_t get_data_region_size();										/*Give the size of the entire data region*/
size_t get_stripesize();										/*Give the size of each block*/
int stripe_locator(int stripeid);									/*Locate the stripe with respect to the free band gap*/
char * map_stripe(int stripeid);									/*Maps stripe to DRAM memory buffer*/
void update_stripe(int stripeid);									/*Update the stripe based on the free band gap movement*/
void crash_recover(long long unsigned pad, char ** shelves, int all_devices);				/*Recover the TxHPC from a crash*/	
