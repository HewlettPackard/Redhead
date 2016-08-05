#include <math.h>
#include "r6_utils.h"

int failure;
                                                      
void print_region(long long unsigned ** filler, long long unsigned *** b_filler, long long unsigned *** a_filler,  meta * meta_info)                          
{                                                     
        int i, j;                                                   
	printf("The region is\n");  
	                                           
        for(i = 0; i < meta_info->no_of_data_devices; i++)
	{    
		for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++) 
		{                                                                        
               		printf("%llu ", (long long unsigned)filler[i][j]);                          
		}
		printf("\n");      	                                      
	}
}                                                     

void fill_region(long long unsigned ** filler, long long unsigned *** b_filler, long long unsigned *** a_filler,  meta * meta_info)
{                                               
        int i, j;                                  

	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {       
        	for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)        
        	{    
			filler[i][j] = (long long unsigned)( i*((int)(meta_info->blocksize)/sizeof(long long unsigned))+ j);
        	}
	}                                       
}                                               

void add_self(long long unsigned ** filler, long long unsigned *** b_filler, long long unsigned *** a_filler,  meta * meta_info)                        
{
                                                                                                                                          
        int i, j;                                                                                                                             
        
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
	        for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)                                                                                                   
        	{                                                                                                                                  
        		filler[i][j] = filler[i][j] + filler[i][j];                                                                             
        	}                                                                                                                                  
	}
}                                                                                                                                          

void mult_self(long long unsigned ** filler, long long unsigned *** b_filler, long long unsigned *** a_filler,  meta * meta_info)
{                                                                                                               
        int i, j;                                                                                                  
        
 	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
        	for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)                                                                        
        	{                                                                                                       
        	        filler[i][j] = filler[i][j] * filler[i][j];                                                              
        	}
	}                                                                                                       
}                                                                                                               

                                                                                                                      
void root_self(long long unsigned ** filler, long long unsigned *** b_filler, long long unsigned *** a_filler,  meta * meta_info)     
{                                                                                                                     
        int i, j;                                                                                                        
        
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                 for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)                                                          
	        {                                                                                                             
        	        filler[i][j] = (long long unsigned)sqrt((long double)filler[i][j]);                                                                    
        	}
	}                                                                                                             
}                                                                                                                     


                                                                                                                      
void half_self(long long unsigned ** filler, long long unsigned *** b_filler, long long unsigned *** a_filler,  meta * meta_info)     
{                                                                                                                     
        int i, j;                                                                                                        
        
 	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)                                                          
	        {                                                                                                             
        	        filler[i][j] = (long long unsigned)filler[i][j]/2;                                                                    
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
                	        b_filler[i][j] = (long long unsigned*)(*before)[i];
                	}
		}
	}
	
        for(i = 0; i < meta_info->prefetch_before; i++)
        {
		for(j =0; j < meta_info->no_of_data_devices; j++)
                {
 	        	if((*after) != NULL)
        	        {
                	        a_filler[i][j] = (long long unsigned*)(*after)[i];
                	}
		}
         }


	if(meta_info->counter == 0)
	{
		printf("Fill\t");
		fill_region(filler, b_filler, a_filler, meta_info);
	}
	else if(meta_info->counter == 1)
	{
		printf("Nothing\t");
		do_nothing(filler, b_filler, a_filler, meta_info);
	}
	else if(meta_info->counter == 2)
	{
		printf("Print\t");
		print_region(filler, b_filler, a_filler, meta_info);
	}
	else if(meta_info->counter == 3)
	{
		printf("Add\t");
		add_self(filler, b_filler, a_filler, meta_info);
	}
	else if(meta_info->counter == 4)
        {
		printf("Multiply\t");
		mult_self(filler, b_filler, a_filler, meta_info);
        }
        else if(meta_info->counter == 5)
	{
		printf("Root\t");
		root_self(filler, b_filler, a_filler, meta_info);
	}
	else if(meta_info->counter == 6)
        {
		printf("Half\t");
		half_self(filler, b_filler, a_filler, meta_info);	
        }
        else if(meta_info->counter == 7)
	{
		printf("Nothing\t");
		do_nothing(filler, b_filler, a_filler, meta_info);
	}
	else if(meta_info->counter == 8)
	{
		printf("Print\t");
		print_region(filler, b_filler, a_filler, meta_info);
	}
	fflush(NULL);

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

void retrieve_values(void * args, char ** cur_stripe, meta * meta_info)
{
	int i;
	long long unsigned ** filler;
	long long unsigned * scratchpad;
	scratchpad = (long long unsigned *)get_scratchpad();
	filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
	}
	
	printf("The halo values are:\n");
	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
		scratchpad[2*i] = filler[i][0];
		scratchpad[2*i + 1] = filler[i][(int)(meta_info->blocksize)/sizeof(long long unsigned) - 1]; 
		printf("%llu %llu ", scratchpad[2*i], scratchpad[2*i +1]);
	}
	printf("\n");
	if(meta_info->counter == 8)
	{
		meta_info->end = true;
	}
	scratchpad = NULL;
	free(filler);
} 


int main(int argc, char*argv[])
{
	char **mem;
	int crasher;
	clock_t begin, end;

	begin = clock();

	sscanf(argv[9], "%d", &crasher);                                             /*Number of coding(parity) devices*/
	
	if(crasher == 0)
        {
		mem = initialize_raid6_parameters(argc, argv);
		failure = 0;
        }
	else if(crasher == 1)
	{
		int pad;
		sscanf(argv[8], "%d", &pad);
		crash_recover(pad);
		failure = 2;
	}
		
	end = clock();
	print_configs();

	printf("The time for initializing the RAID-6 parameters is %f seconds.\n", calculate_time(begin, end));

	run(&iterate_compute, NULL, &retrieve_values, NULL);
	mem = NULL;
	destroy_raid6_parameters();
	return 0;

}