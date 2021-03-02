#ifndef TUPLE_H
#define TUPLE_H

#include<tuple_def.h>

uint64_t get_column_size(tuple_def* tpl_d, uint64_t column_no, const void* tupl);

uint64_t get_column_offset(tuple_def* tpl_d, uint64_t column_no, const void* tupl);

element seek_to_column(tuple_def* tpl_d, uint64_t column_no, const void* tupl);

void copy_to_tuple(tuple_def* tpl_d, uint64_t column_no, void* tupl, const void* value);

void copy_from_tuple(tuple_def* tpl_d, uint64_t column_no, const void* tupl, void* value);

int compare_tuple(void* tup1, void* tup2, tuple_def* tpl_d);

// returns number of characters written to the string
int sprint_tuple(char* str, void* tup, tuple_def* tpl_d);

// returns number of bytes read from the string
int sscan_tuple(char* str, void* tup, tuple_def* tpl_d);

#endif