/*
 * Version 1.2
 *
 * Mesut Kuscu - mesut.kuscu@hpe.com
 *
 *
 */

//TODO: Set SIGBUS Handler to catch SIGBUS and to re-initialize this pointer.

#ifndef PORTABLE_POINTER_H
#define PORTABLE_POINTER_H

#include <string>
#include <iosfwd>
#include "ptm_type_traits.h"

using namespace std;

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

template <class PPType>
class Portable_pointer
{
public:
    typedef PPType*                                               pointer;
    typedef typename ptm_type_traits::add_reference<PPType>::type reference;
    typedef uint                                                  id_type;

protected:
    pointer address = NULL;
    id_type id = 0; //The functionality which makes a pointer portable needs to have an ID!

    pointer add_offset_to_pointer (pointer addr, size_t offset_inBytes){
        unsigned char* temp = (reinterpret_cast<unsigned char*>(addr) + offset_inBytes);
        return (pointer)temp;
    }
    pointer dec_offset_from_pointer (pointer addr, size_t offset_inBytes){
        unsigned char* temp = (reinterpret_cast<unsigned char*>(addr) - offset_inBytes);
        return (pointer)temp;
    }

public:
    Portable_pointer(){
        this->address = NULL;
    }
    Portable_pointer(pointer baseAddr, size_t off_t, id_type ID) {
        this->address = add_offset_to_pointer(baseAddr, off_t);
        this->id = ID;
    }
    Portable_pointer(pointer absAddr, id_type ID){
        this->address = absAddr;
        this->id = ID;
    }
    Portable_pointer(pointer absAddr){
        this->address = absAddr;
    }
    bool is_valid() {
        return (bool)this->address;
    }
    //Get
    pointer get() const {
        if(!(this->address == NULL)){
            return this->address;
        }
        else{
            handle_error("portable pointer is not initialized. (Re-initialized pointer by using ID and Catalog ?).\n");
        }
    }

    pointer get_addrAtElement(size_t index){
        return add_offset_to_pointer(this->get(), sizeof(PPType) * index);
    }

    pointer beg() const{
        return this->get();
    }
    id_type get_id() const {
        return this->id;
    }

    void set_addr(pointer new_baseAddr){
        this->address = new_baseAddr;
    }
    void add_offset(size_t offset){
        this->address = this->add_offset_to_pointer(this->address, offset);
    }
    void dec_offset(size_t offset){
        this->address = this->dec_offset_from_pointer(this->address, offset);
    }

    //Operators
    // -> TODO fixme
    /*reference operator->() const{
        return this->get();
    }*/

    // * (Dereferencing operator)
    reference operator* () const {
        return this->get()[0]; //well, this works!
    }

    // [ ] (Indexing operator)
    reference operator[](size_t index) const{
        return this->get()[index];
    }

    // ++
    Portable_pointer operator++ (int) {
        this->add_offset(sizeof(PPType));
        return *this;
    }

    // --
    Portable_pointer operator-- (int) {
        this->dec_offset(sizeof(PPType));
        return *this;
    }

    // ++ ptm
    Portable_pointer &operator++(void){
        this->add_offset(sizeof(PPType));
        return *this;
    }

    // -- ptm
    Portable_pointer &operator--(void){
        this->dec_offset(sizeof(PPType));
        return *this;
    }

    // +=
    Portable_pointer operator += (size_t offset){
        this->add_offset(offset * sizeof(PPType));
        return *this;
    }

    // -=
    Portable_pointer &operator -= (size_t offset){
        this->dec_offset(offset * sizeof(PPType));
        return *this;
    }

    friend bool operator== (const Portable_pointer &p1, const Portable_pointer &p2){
        return p1.get() == p2.get();
    }
    friend bool operator!= (const Portable_pointer &p1, const Portable_pointer &p2){
        return p1.get() != p2.get();
    }
    friend bool operator< (const Portable_pointer &p1, const Portable_pointer &p2){
        return p1.get() < p2.get();
    }
    friend bool operator> (const Portable_pointer &p1, const Portable_pointer &p2){
        return p1.get() > p2.get();
    }
    friend bool operator<= (const Portable_pointer &p1, const Portable_pointer &p2){
        return p1.get() <= p2.get();
    }
    friend bool operator>= (const Portable_pointer &p1, const Portable_pointer &p2){
        return p1.get() >= p2.get();
    }

    Portable_pointer &operator = (Portable_pointer<PPType> Portable_pointer_extern){
        this->address = Portable_pointer_extern.get();
        this->id = Portable_pointer_extern.get_id();
        return *this;
    }
};


#endif //Portable_pointer_H
