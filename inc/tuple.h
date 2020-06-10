#ifndef TUPLE_H
#define TUPLE_H

#include<stdio.h>
#include<alloca.h>
#include<tuple_def.h>

#include<string.h>

inline element seek_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d);

inline void copy_to_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d, void* value);

inline void copy_from_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d, void* value);

inline int compare_tuple(void* tup1, void* tup2, tuple_def* tpl_d);

inline void swap_tuples(void* tup1, void* tup2, tuple_def* tpl_d);

// returns number of characters written to the string
int sprint_tuple(char* str, void* tup, tuple_def* tpl_d);

// returns number of bytes read from the string
int sscan_tuple(char* str, void* tup, tuple_def* tpl_d);

#endif