/*
 * Version 1.0
 *
 * Mesut Kuscu - mesut.kuscu@hp.com
 *
 *
 */

#ifndef Meta_entry_H
#define Meta_entry_H

#include <string>
#include <iosfwd>
#include "ptm_type_traits.h"

#define FILEPATH_SIZE 16 //Change it here

template <class MPType>
class Meta_entry
{
public:
    //typedef char              pathToMemory[FILEPATH_SIZE];
    //should the path be a different type? Or a template?
    //Unknown, no filesystem of The Machine
    //MMapping to a region can be done for example with the path of a file
    //which is either a char* or string
    typedef size_t              byte_type;


protected:
    char filepath[FILEPATH_SIZE];
    off_t filelength = 0;
    byte_type offset_in_byte = 0;


public:
    Meta_entry(){}

    Meta_entry(const char* filepathIn, byte_type offset, off_t filelength) {
        strncpy(this->filepath, filepathIn, 16);
        this->offset_in_byte = offset;
        this->filelength = filelength;
    }
    Meta_entry(const char* filepathIn, byte_type offset) {
        strncpy(this->filepath, filepathIn, 16);
        this->offset_in_byte = offset;
    }
    byte_type get_offset() const{
        return this->offset_in_byte;
    }
    const char* get_filePath(){
        return this->filepath;
        //char* buffer = new char[FILEPATH_SIZE];
        //return buffer;
    }
    const off_t get_fileLength(){
        return this->filelength;
    }
    void set_filePath(const char* filepathIn){
        strcpy(this->filepath, filepathIn);
    }
    void set_offset(byte_type offset){
        this->offset_in_byte = offset;
    }
    void set_fileLength(off_t newlength){
        this->filelength = newlength;
    }

    Meta_entry &operator = (Meta_entry pointerEx){
        this->filelength = pointerEx.get_fileLength();
        this->offset_in_byte = pointerEx.offset_in_byte;
        strcpy(this->filepath, pointerEx.get_filePath());
        return *this;
    }
};
#endif //Meta_entry_H
