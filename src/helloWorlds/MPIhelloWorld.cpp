//
// Created by kuscum on 6/21/16.
//

/*
 Mesut Kuscu - mesut.kuscu@hp.com

 This is a hello world example, which uses MPI to connect to multiple
 machines and to use portable pointers.

 The following happens in this application:
    1. Call 2 processors on 4 nodes (the "my_hostfile" needs to be set up correctly)
    2. Create a shelve in /lfs
    3. Write to it: "Hello World"
    4. Read from it: "Hello World"
*/

#include "mpi.h"
#include "stdio.h"
#include <iostream>
#include <include/portable_pointer.h>
#include <include/catalogue.h>
#include <set>

#define SHELVENAME "HATdemoFile"
#define SHELVESIZE 1024

using namespace std;
void createShelveAndWrite(int processID)
{
    //Concatenate shelve name with processID, so each shelve name is unique
    std::string shelveName(SHELVENAME);
    shelveName+=std::to_string(processID);

    //Create or get Shelve with Size.
    char* memoryAddr = (char*)cat<char>::nvram_mmap(shelveName.c_str(), SHELVESIZE);
    Portable_pointer<char> ptr1(memoryAddr, processID);

    //Write into Shelve.
    std::string temp = "Hello HAT!";
    temp.copy(&ptr1[0],10,0);

    //unmap
    cat<char>::unmap((void*)memoryAddr, SHELVESIZE); //File is already closed after the mmap.
}

//This code can be called from another process which is completely independent.
void evaluateShelve(int processID){
    //Concatenate shelve name.
    std::string shelveName(SHELVENAME);
    shelveName+= std::to_string(processID);

    //Get Shelve with Size.
    char* memoryAddr = (char*)(cat<char>::nvram_mmap(shelveName.c_str(), SHELVESIZE));
    Portable_pointer<char> ptr1(memoryAddr, processID);

    //Should be "Hello HAT!" - So we wait, till this is the case.
    std::string temp1 = "";
    std::string goalToRead = "Hello HAT!";
    for (int i = 0; i < 10; ++i) {
        temp1 =+ ptr1[i];
    }

    //Wait till the data is read correctly
    do{}while(goalToRead.compare(temp1) == 0);
    cat<char>::unmap((void*)memoryAddr, SHELVESIZE); //File is already closed after the mmap.
}

void printFinishedStatement(char processor_name[MPI_MAX_PROCESSOR_NAME], int my_rank){
    printf("Hello HAT! from Node %s from Processor with rank %d"
                   ". Successfully Written and Read."
                   "\n",
           processor_name, my_rank);
}

int main(int argc, char *argv[])
{
    MPI_Init( &argc, &argv );
    // How many processes are there?
    int num_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    // What is my rank?
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    // What is my name?
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    //Run Test
    createShelveAndWrite(my_rank);
    evaluateShelve(my_rank);
    //Print Finish Statement
    printFinishedStatement(processor_name, my_rank);

    MPI_Finalize();
    return 0;
}
