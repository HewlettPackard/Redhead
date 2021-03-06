//
// Created by kuscum on 6/14/16.
//

//This file contains the general settings for the mmap()
//and the MPI start. It is good to have all statical settings in one place.
//If both features are not used,
//this file does not need to be adjusted.
/*
    [HPE copyright notice]

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    As an exception, the copyright holders of this Library grant you permission to (i) compile an Application with the Library, and
    (ii) distribute the Application containing code generated by the Library and    
    added to the Application during this compilation process under terms of your choice, provided you also meet the terms and conditions of the Application license.

 */

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
#define SHELVESIZE 5.369e+9 //1 Gibibyte
#define MMAP_DIR_LFS "/lfs"
#define MMAP_DIR_DEVSHM "/dev/shm"



#endif //PORTABLE_POINTER_SETTINGS_H
