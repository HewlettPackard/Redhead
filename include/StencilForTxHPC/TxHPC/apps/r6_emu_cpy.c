#include <math.h>
#include "r6_utils.h"

long long unsigned * halo;

void print_address(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)
{
	int i;
	long long unsigned ** filler;
        long long unsigned ** b_filler;
        long long unsigned ** a_filler;
        filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        b_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        a_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
		printf("Curr addr: %p\nFill addr: %p\n", cur_stripe[i], filler[i]);
        }
}

                                                      
void print_region(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)                          
{                                                     
        int i, j;                                                   
	long long unsigned ** filler;                                            
	long long unsigned ** b_filler;                                          
	long long unsigned ** a_filler;                                          
        filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);                                                 
	b_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
	a_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);

	for(i = 0; i < meta_info->no_of_data_devices; i++)
	{
		filler[i] = (long long unsigned*)cur_stripe[i];
		if((*before) != NULL)
		{
			b_filler[i] = (long long unsigned*)(*before)[i];
		}
		if((*after) != NULL)
		{
			a_filler[i] = (long long unsigned*)(*after)[i];
		}
	}
        for(i = 0; i < meta_info->no_of_data_devices; i++)
	{    
		printf("Stripe id: %d Step no: %d\n", stripe_id, meta_info->counter);                                             
		for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++) 
		{                                                                        
               		printf("%llu ", (long long unsigned)filler[i][j]);                          
		}
		printf("\n");      	                                      
	}
}                                                     

void fill_region(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)
{                                               
        int i, j;                                  
        long long unsigned ** filler;
	long long unsigned ** b_filler;
	long long unsigned ** a_filler;
	filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        b_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        a_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
		if((*before) != NULL)
		{
                	b_filler[i] = (long long unsigned*)(*before)[i];
			
		}
		if((*after)!= NULL)
		{
                	a_filler[i] = (long long unsigned*)(*after)[i];
		}
        }

/*	if((*before) != NULL)
	{        
		for(i = 0; i < meta_info->no_of_data_devices; i++)
		{
			printf("Before: %d Stripe id: %d\n", i, stripe_id);
			fflush(NULL);
			for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)
			{
				printf("%llu ", (long long unsigned)b_filler[i][j]);
				fflush(NULL);
			
			}
			printf("\n");
			fflush(NULL);
		}
	}
*/
	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {       
        	for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)        
        	{      
			if(stripe_id == 0 && i == 0 && j == 0) 
			{                               
                		filler[i][j] = (long long unsigned)j;           
			}
			else if(stripe_id != 0 && i == 0 && j == 0 )
			{
				filler[i][j] = b_filler[meta_info->no_of_data_devices - 1][((int)((meta_info->blocksize)/sizeof(long long unsigned))) - 1] + 1;
			}
			else if(i != 0 && j == 0)
			{
				filler[i][j] = filler[i-1][((int)((meta_info->blocksize)/sizeof(long long unsigned))) - 1] + 1;
			}
			else
			{
				filler[i][j] = filler[i][j-1] + 1;
			}
		
		
	//	filler[i][j] = (long long unsigned)j;
        	}
		//printf("\n");
	}                                       
}                                               

void add_self(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)                        
{
                                                                                                                                          
        int i, j;                                                                                                                             
        long long unsigned ** filler;                                                                                                                      
        long long unsigned ** b_filler;                                                                                                                    
        long long unsigned ** a_filler;                                                                                                                    
	filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        b_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        a_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
		if((*before) != NULL)
		{
                	b_filler[i] = (long long unsigned*)(*before)[i];
		}
		if((*after) != NULL)
		{
                	a_filler[i] = (long long unsigned*)(*after)[i];
		}
        }
        
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
	        for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)                                                                                                   
        	{                                                                                                                                  
        		filler[i][j] = filler[i][j] + filler[i][j];                                                                             
        	}                                                                                                                                  
	}
}                                                                                                                                          

void mult_self(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)
{                                                                                                               
        int i, j;                                                                                                  
        long long unsigned ** filler;                                                                                           
        long long unsigned ** b_filler;                                                                                         
        long long unsigned ** a_filler;                                                                                         
	filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        b_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        a_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
		if((*before) != NULL)
		{
                	b_filler[i] = (long long unsigned*)(*before)[i];
		}
		if((*after) != NULL)
		{
                	a_filler[i] = (long long unsigned*)(*after)[i];
		}
        }
        
 	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
        	for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)                                                                        
        	{                                                                                                       
        	        filler[i][j] = filler[i][j] * filler[i][j];                                                              
        	}
	}                                                                                                       
}                                                                                                               

                                                                                                                      
void root_self(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)     
{                                                                                                                     
        int i, j;                                                                                                        
        long long unsigned ** filler;                                                                                                 
        long long unsigned ** b_filler;                                                                                               
        long long unsigned ** a_filler;                                                                                               
	filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        b_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        a_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
		if((*before) != NULL)
		{
                	b_filler[i] = (long long unsigned*)(*before)[i];
		}
		if((*after) != NULL)
		{
                	a_filler[i] = (long long unsigned*)(*after)[i];
		}
        }
        
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                 for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)                                                          
	        {                                                                                                             
        	        filler[i][j] = (long long unsigned)sqrt((long double)filler[i][j]);                                                                    
        	}
	}                                                                                                             
}                                                                                                                     


                                                                                                                      
void half_self(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)     
{                                                                                                                     
        int i, j;                                                                                                        
        long long unsigned ** filler;                                                                                                 
        long long unsigned ** b_filler;                                                                                               
        long long unsigned ** a_filler;                                                                                               
	filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        b_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        a_filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
        for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
		if((*before) != NULL)
		{
                	b_filler[i] = (long long unsigned*)(*before)[i];
		}
		if((*after) != NULL)
		{
                	a_filler[i] = (long long unsigned*)(*after)[i];
		}
        }
        
 	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                for(j = 0; j < ((int)(meta_info->blocksize)/sizeof(long long unsigned)); j++)                                                          
	        {                                                                                                             
        	        filler[i][j] = (long long unsigned)filler[i][j]/2;                                                                    
        	}
	}                                                                                                             
}                                                                                                                     

void do_nothing(void * args, char ** cur_stripe, meta * meta_info, int stripe_id, char*** before, char*** after)
{
	return;
}

void retrieve_values(void * args, char ** cur_stripe, meta * meta_info)
{
	int i;
	long long unsigned ** filler;
	filler = (long long unsigned **)malloc(sizeof(long long unsigned*)*meta_info->no_of_data_devices);
	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
                filler[i] = (long long unsigned*)cur_stripe[i];
	}
	
	printf("The halo values are:\n");
	for(i = 0; i < meta_info->no_of_data_devices; i++)
        {
		halo[2*i] = filler[i][0];
		halo[2*i + 1] = filler[i][(int)(meta_info->blocksize)/sizeof(long long unsigned) - 1]; 
		printf("%llu %llu ", halo[2*i], halo[2*i +1]);
	}
	printf("\n");
} 


int main(int argc, char*argv[])
{
	char **mem;
	int eraser;
	meta * meta_info;


	int i;	
	time_t t;
	clock_t begin, end;
        srand((unsigned)time(&t));

	begin = clock();
	mem = initialize_raid6_parameters(argc, argv);
	end = clock();


	meta_info = get_meta_info();
	halo = (long long unsigned *)malloc(sizeof(long long unsigned)*meta_info->no_of_data_devices*2);	
	print_configs();

	printf("The time for initializing the RAID-6 parameters is %f seconds.\n", calculate_time(begin, end));

	begin = clock();
	run(&fill_region, NULL, &retrieve_values, NULL);
	end = clock();
	printf("The time for filling region before erasures is %f seconds.\n", calculate_time(begin, end));
	
	begin = clock();
	run(&do_nothing, NULL, NULL, NULL);
	end = clock();
	printf("The time for a free band gap traversal is %f seconds.\n", calculate_time(begin, end));

	//printf("The data region is:\n");
	//run(&print_region, NULL, 1, 1);
	
	printf("Adding\n");
	begin = clock();
	run(&add_self, NULL, &retrieve_values, NULL);
	end = clock();
	printf("The time for addition to it self before erasures is %f seconds.\n", calculate_time(begin, end));
	while(1)                         
	{                                      
       		eraser = (rand()%30);          
        	if(eraser < meta_info->total_devices)     
        	{                              
                	erase(eraser);         
                	erased_device(eraser);
			break; 
        	}                              
	}

	printf("First erasure: Shelf %d\n", eraser);

	printf("Multiplying\n");
	begin = clock();
	run(&mult_self, NULL, &retrieve_values, NULL);
	end = clock();
	printf("The time for multiplication after one erasure is %f seconds.\n", calculate_time(begin, end));

	printf("Root\n");
	begin = clock();
	run(&root_self, NULL, &retrieve_values, NULL);
	end = clock();
	printf("The time for sqaure-root after one erasure is %f seconds.\n", calculate_time(begin, end));
        
                              
	while(1)                               
	{                                     
        	eraser = (rand()%30);         
        	if(eraser < meta_info->total_devices)    
        	{                             
        	        erase(eraser);        
                	erased_device(eraser);
                	break;                
        	}	                             
	}
         
	printf("First erasure: Shelf %d\n", eraser);

	printf("Divide\n");                            
	begin = clock();
	run(&half_self, NULL, &retrieve_values, NULL);
	end = clock();
	printf("The time for subtraction after two erasures is %f seconds.\n", calculate_time(begin, end));
	
//	run(&do_nothing, NULL, 1, 1);	

	//printf("The data region is :\n");
	//run(&print_region, NULL, 1, 1);

	return 0;

}
