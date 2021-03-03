#ifndef TUPLE_H
#define TUPLE_H

#include<tuple_def.h>

uint64_t get_element_size(const tuple_def* tpl_d, uint64_t index, const void* tupl);

uint64_t get_element_offset(const tuple_def* tpl_d, uint64_t index, const void* tupl);

element seek_to_element(const tuple_def* tpl_d, uint64_t index, const void* tupl);

void copy_element_to_tuple(const tuple_def* tpl_d, uint64_t index, void* tupl, const void* value);

void copy_element_from_tuple(const tuple_def* tpl_d, uint64_t index, const void* tupl, void* value);

int compare_elements(void* tup1, void* tup2, const tuple_def* tpl_d, uint64_t index);

int compare_tuples(void* tup1, void* tup2, const tuple_def* tpl_d);

// returns number of characters written to the string
int sprint_tuple(char* str, void* tup, const tuple_def* tpl_d);

// returns number of bytes read from the string
int sscan_tuple(char* str, void* tup, const tuple_def* tpl_d);

#endif