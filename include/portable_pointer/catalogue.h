//
// Mesut Kuscu - mesut.kuscu@hp.com
//
#ifndef HEADERS_CATALOGUE_H
#define HEADERS_CATALOGUE_H

#include <fcntl.h>
#include <unistd.h>

#include "meta_entry.h"
#include "portable_pointer.h"
#include "config/settings.h"

#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cmath>


/*
 * //TODO Backlog:
        //gets an entry of metadata depending on the pointer
        Meta_entry getEntry(pointer catalogueAddr){
        }
        //Gets the path of a file which represents the MMAPED region
        char getPath[50] (pointer catalogueAddr){
        }
       //Gets the baseAddr of a Meta_entry
       pointer getBaseAddr(pointer catalogueAddr);

       //Gets the offset
       size_t getOffset(pointer catalogueAddr);

       //COHERENCE TEMP SOL
       //Makes the pointer dirty so someone is accessingit
       void makeDirty(pointer);

       //Makes the pointer clean
       void clean();

       //Checks if someone updated the pointer or not
       bool is_dirty(pointer catalogeAddr);
*/


template <class CATType>
class cat {
    typedef Meta_entry<CATType> *pointer;

protected:
    Portable_pointer<Meta_entry<CATType>> catPointer;
    int id = 0;

public:
    //STATIC FUNCTIONS - Memory-Mapping Procedures
    static void *nvram_mmap(const char *fileName, size_t file_length, const char* mmap_dir) {
        //  const char* regionname = path;
        //  off_t length;
        //  length = nvram_region_length(path); //if file is there already then take the length of it
        //  if (length < 0)

        char *filePath;
        //appending to LFS dir to filename
        if (asprintf(&filePath, "%s/%s", mmap_dir, fileName) == -1) {
            handle_error("Unable to concatenate file name to MMAP dir");
        }
//        std::cout << "MMAPing: " << filePath << ".length: " << file_length << ".\n";
        // Create file
        int fd = open(filePath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0) {
            handle_error("open of file failed");
        }
        //currentFD = fd;
        // Grow the size
        int res = ftruncate(fd, file_length);
        if (res < 0) {
            handle_error("ftruncate failed - Valid size entered?");
        }
        // Mmap file
        //TODO Set Flags accordingly to the needs of the programm, maybe in setting.h?
        void *mappedMemory = mmap(0, file_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (MAP_FAILED == mappedMemory) {
            int errsv = errno;
            std::cout << "errno: " << errsv << ".\n";
            handle_error("mmap() of file failed");
        }
        // Close file still holding the memory-mapped procedure
        if (close(fd) == -1) {
            handle_error("Close failed");
        }
        return mappedMemory;
    }

    static void unmap(void* mmapAddr, size_t totalSz) {
        // Unmap
        if (munmap(mmapAddr, totalSz) == -1) {
            int errsv = errno;
            std::cout << "errno: " << errsv << ".\n";
            handle_error("Unmap failed");
        }
    }

    static Portable_pointer<CATType> mmap_returnPP(const char *path, ulong file_length) {
        void *t = cat::nvram_mmap(path, file_length);
        Portable_pointer<CATType> temp((CATType *) t, 0); //0 is not an ID to a metapointer
        return temp;
    }

    //MMaps a set of shelves
    static CATType** createShelvesAndMMAP(int amountOfPointer, size_t shelvesize, const char* mmap_address)
    {
        size_t sizeOfPointerArray = sizeof(int*) * amountOfPointer;
        CATType** ptrArray = (CATType**)malloc(sizeOfPointerArray);
        std::string shelveID = SHELVENAME;
        std::string temp = "";
        for (int j = 0; j < amountOfPointer; ++j) {
            temp = shelveID + std::to_string(j);
            ptrArray[j]  = (CATType*)cat<CATType>::nvram_mmap(temp.c_str(), shelvesize, mmap_address);
        }
        return &ptrArray[0];
    }
    //Writes a '0' in the first bit of every shelve
    static void checkMemory(CATType* memoryPtr[11], int amountOfPointer){
        for (int j = 0; j < amountOfPointer; ++j) {
            *(memoryPtr[j]) = 0;
        }
        for (int j = 0; j < amountOfPointer; ++j) {
            if(*(memoryPtr[j]) == 0){
            }else{
                handle_error("Memory is not accessible correctly. Could not read the right value. Check Fail.");
            }
        }
    }
    //Unmaps an array of shelves
    static void unmapShelves(int amountOfPointer, size_t shelvesize, const char* mmap_address){
        for (int j = 0; j < amountOfPointer; ++j) {
            cat<char>::unmap((void*)mmap_address[amountOfPointer], shelvesize);
        }
    }

    //NON-STATIC FUNCTIONS:
    //Initializing the catalog with a memory mapped file
    cat() {
        this->init(PATH_CAT); //Path of catalogue in settings file
    }

    void init(const char *catpath) {
        void *p = cat::nvram_mmap(catpath, SIZE_CAT);
        this->catPointer.set_addr((pointer) p);
    }

    //Unmap catalogue file
    void Finilize(){
        void* catPath = this->catPointer.get();
        cat::unmap(catPath, SIZE_CAT);
    }

    //Save Data
    //Create Meta Pointer - Write into Catalogue - Return Either the ID or the PP.

    // Sets an entry in the Catalogue
    // Returns the address to that entry
    // So the pointer can be saved instead of the ID!

    Portable_pointer<CATType> addEntry_returnPP(const char* filePath, size_t offset, off_t filelength) {
        //Crate Meta_entry
        Meta_entry<char> tempME(filePath, offset, filelength);
        //Save MetaEntry in Catalogue
        id++;
        this->catPointer[id] = tempME; //Save object in catalogue
        void *tempaddress = cat<CATType>::nvram_mmap(tempME.get_filePath(), tempME.get_fileLength());
        Portable_pointer<CATType> tempPP((CATType *) tempaddress, id);
        return tempPP;
    }

/*    Portable_pointer<CATType> addEntry_returnPP(Meta_entry<CATType> exPointer) {
        id++;
        this->catPointer[id] = exPointer;
        void *tempaddress = cat<CATType>::nvram_mmap(exPointer.get_filePath(), exPointer.get_fileLength());
        Portable_pointer<CATType> temp2((CATType *) tempaddress, id);
        return temp2;
    }*/


    //Load Data
    //Load Meta information from ID or MetaPointer. Use it to create a PP and return it.

/*    //MMAPS an ID and returns
    void* mmapWithID(int ID){
        this->checkID(ID);
        return nvram_mmap(this->catPointer[ID].get_filePath(), this->catPointer[ID].get_fileLength());
    }*/

    //Retuns created Portable Pointer object made of a Meta Pointer by providing the ID
    Portable_pointer<CATType> getPortablePointerEntryWithID(uint ID) {
//        checkID(ID);
//        void *p = cat::nvram_mmap(this->catPointer[ID].get_filePath(), this->catPointer[ID].get_fileLength());
        Meta_entry<CATType> tempME = getMetaEntryWithID(ID);
        void *p = cat::nvram_mmap(tempME.get_filePath(), tempME.get_fileLength());
        Portable_pointer<CATType> tempPP((CATType *) p, ID);
        tempPP.add_offset(tempME.get_offset()); //Add offset of the ME.
        return tempPP;
    }

    //For initializing purposes
    Meta_entry<CATType> getMetaEntryWithID(uint ID) {
        checkID(ID);
        Meta_entry<CATType> tempMP(this->catPointer[ID].get_filePath(), this->catPointer[ID].get_offset(),
                                     this->catPointer[ID].get_fileLength());
        return tempMP;
    }
    //Entered ID needs to be in range.
    void checkID(int ID) {
        if (ID > this->id) {
            handle_error("ERROR: ID is over the range! Indexing elements are starting at 0.");
        }
    }

    //Delete Data
    void DeleteEntry(uint ID) {
        id--;
    }

};
#endif //HEADERS_CATALOGUE_H


    /*
    static off_t checkLength(off_t length){
        if(length < 1){
            return SIZE_DEFAULT;
        }
        else{
            return length;
        }
    }






    }*/







