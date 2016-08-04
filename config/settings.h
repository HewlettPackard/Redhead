//
// Created by kuscum on 6/14/16.
//

//This file contains the general settings for the mmap()
//and the MPI start. It is good to have all statical settings in one place.
//If both features are not used,
//this file does not need to be adjusted.


#ifndef PORTABLE_POINTER_SETTINGS_H
#define PORTABLE_POINTER_SETTINGS_H

#include <string>
#include <iostream>

//Path for the MMAP
//#define MMAP_DIR "/lfs"

//Catalogue - File name (Shelve ID)
#define PATH_CAT "HATdemoCatalogue"
//Catalogue size
#define SIZE_CAT 1024

#define SHELVENAME "TxHPC_"
#define SHELVESIZE 1073741824 //1 Gibibyte
#define MMAP_DIR_LFS "/lfs"
#define MMAP_DIR_DEVSHM "/dev/shm"



#endif //PORTABLE_POINTER_SETTINGS_H
