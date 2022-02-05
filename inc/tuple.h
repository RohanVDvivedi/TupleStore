#ifndef TUPLE_H
#define TUPLE_H

#include<tuple_def.h>

// also the index must be an unsigned integral value between [0, tpl_d->element_count),
// for the functions that are called on elements of a tuple for given a tuple definition.
// this is the mandatory condition, since, the functions do not check this condition to be true, repetitively.
// and the functions assume that this condition holds true while you call that particular element level function.


// element level functions
uint32_t get_element_size_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl);

uint32_t get_element_offset(const tuple_def* tpl_d, uint32_t index, const void* tupl);

element get_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl);


// tuple level functions
uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl);

void* get_end_of_tuple(const tuple_def* tpl_d, const void* tupl);


// copy element to and from tuple functions
void copy_element_to_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const void* value);

void copy_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl, void* value);


// compare functions for tuple and for an element inside the tuple
int compare_elements_within_tuple(const void* tup1, const void* tup2, const tuple_def* tpl_d, uint32_t index);

int compare_tuples(const void* tup1, const void* tup2, const tuple_def* tpl_d);


// hash function for tuple and for an element inside the tuple at the specified index
uint32_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, uint32_t index, uint32_t (*hash_func)(const void* data, uint32_t size));

uint32_t hash_tuple(const void* tup, const tuple_def* tpl_d, uint32_t (*hash_func)(const void* data, uint32_t size));


// print and scan functions for tuple
// returns number of characters written to the string
int sprint_tuple(char* str, const void* tup, const tuple_def* tpl_d);

// returns number of bytes read from the string
int sscan_tuple(const char* str, void* tup, const tuple_def* tpl_d);

#endif