#ifndef TUPLE_H
#define TUPLE_H

#include<alloca.h>
#include<tuple_def.h>

#include<byteswap.h>
#include<endian.h>
#include<string.h>

element get_cell(tuple_def* tpl_d, void* tpl, unsigned int column_no);
void set_cell(tuple_def* tpl_d, void* tpl, unsigned int column_no, void* value);

#endif