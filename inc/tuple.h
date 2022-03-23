#ifndef TUPLE_H
#define TUPLE_H

#include<tuple_def.h>

// the index must be an unsigned integral value between [0, tpl_d->element_count), for the functions that are called on elements of a tuple for given a tuple definition.
// this is the mandatory condition, since, the functions do not check this condition to be true, repetitively.
// and the functions assume that this condition holds true while you call that particular element level function.

// initialized the tuple with all NULL attributes and its minimum size
void init_tuple(const tuple_def* tpl_d, void* tupl);

// element level functions
uint32_t get_element_size_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl);

uint32_t get_element_offset_within_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl);

element get_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl);


// tuple level functions
uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl);

void* get_end_of_tuple(const tuple_def* tpl_d, const void* tupl);


// check if the ith element in the tuple is NULL
int is_NULL_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl);

// copy element to and from tuple functions
void set_element_in_tuple(const tuple_def* tpl_d, uint32_t index, void* tupl, const void* value, uint32_t var_blob_size);

void copy_element_from_tuple(const tuple_def* tpl_d, uint32_t index, const void* tupl, void* value);


// compare functions for tuple and for an element inside the tuple
int compare_elements_within_tuple(const void* tup1, const void* tup2, const tuple_def* tpl_d, uint32_t index);

// if element_ids == NULL, then this functions compares the first element_count number of elements from both the tuples
// else it compares the elements in the same order as provided in element_ids (here element_count denotes its size)
int compare_tuples(const void* tup1, const void* tup2, const tuple_def* tpl_d, uint32_t element_count, uint32_t* element_ids);


// hash function for tuple and for an element inside the tuple at the specified index
uint32_t hash_element_within_tuple(const void* tup, const tuple_def* tpl_d, uint32_t index, uint32_t (*hash_func)(const void* data, uint32_t size));

// if element_ids == NULL, then this functions hashes the first element_count number of elements from the tuple
// else it hashes the elements in the same order as provided in element_ids (here element_count denotes its size)
uint32_t hash_tuple(const void* tup, const tuple_def* tpl_d, uint32_t (*hash_func)(const void* data, uint32_t size), uint32_t element_count, uint32_t* element_ids);


// print and scan functions for tuple
// returns number of characters written to the string
int sprint_tuple(char* str, const void* tup, const tuple_def* tpl_d);

#endif