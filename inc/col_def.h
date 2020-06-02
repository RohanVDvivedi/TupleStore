#ifndef COL_DEF_H
#define COL_DEF_H

#include<datatype.h>

typedef struct col_def col_def;
struct col_def
{
	// byte offset in tuple for the given cell definition
	unsigned int byte_offset;

	// size in bytes that is occupied by the data cell with this cell definition
	unsigned int size_in_bytes;

	// datatype stored in the cell
	datatype type;
};

int init_col_def(col_def* col_d, datatype col_type, unsigned int size_in_bytes);

#endif