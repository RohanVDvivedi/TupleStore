#ifndef TUPLE_H
#define TUPLE_H

#include<tuple_def.h>

// NONE OF THE FOLLOWING FUNCTIONS CAN BE CALLED ON AN EMPTY TUPLE
// i.e. please check true for (0 == is_empty_tuple_def(const tuple_def* tpl_d))
// prior to calling any of the following functions below

// element level functions
uint32_t get_element_size(const tuple_def* tpl_d, uint16_t index, const void* tupl);

uint32_t get_element_offset(const tuple_def* tpl_d, uint16_t index, const void* tupl);

element seek_to_element(const tuple_def* tpl_d, uint16_t index, const void* tupl);


// tuple level functions
uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl);

void* seek_to_end_of_tuple(const tuple_def* tpl_d, const void* tupl);


// copy element to and from tuple functions
void copy_element_to_tuple(const tuple_def* tpl_d, uint16_t index, void* tupl, const void* value);

void copy_element_from_tuple(const tuple_def* tpl_d, uint16_t index, const void* tupl, void* value);


// compare functions for tuple and for an element inside the tuple
int compare_elements(const void* tup1, const void* tup2, const tuple_def* tpl_d, uint16_t index);

int compare_tuples(const void* tup1, const void* tup2, const tuple_def* tpl_d);


// hash function for tuple and for an element inside the tuple at the specified index
uint32_t hash_element(const void* tup, const tuple_def* tpl_d, uint16_t index);

uint32_t hash_tuple(const void* tup, const tuple_def* tpl_d);


// print and scan functions for tuple
// returns number of characters written to the string
int sprint_tuple(char* str, void* tup, const tuple_def* tpl_d);

// returns number of bytes read from the string
int sscan_tuple(char* str, void* tup, const tuple_def* tpl_d);

#endif