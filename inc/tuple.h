#ifndef TUPLE_H
#define TUPLE_H

#include<stdio.h>
#include<alloca.h>
#include<tuple_def.h>

#include<string.h>

element seek_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d);

void copy_to_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d, void* value);

void copy_from_cell(void* tupl, unsigned int column_no, tuple_def* tpl_d, void* value);

int compare_tuple(void* tup1, void* tup2, tuple_def* tpl_d);

void swap_tuples(void* tup1, void* tup2, tuple_def* tpl_d);

// returns number of characters written to the string
int sprint_tuple(char* str, void* tup, tuple_def* tpl_d);

void sscan_tuple(char* str, void* tup, tuple_def* tpl_d);

#endif