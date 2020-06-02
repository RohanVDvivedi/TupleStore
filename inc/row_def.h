#ifndef ROW_DEF_H
#define ROW_DEF_H

#include<col_def.h>

typedef struct row_def row_def;
struct row_def
{
	unsigned int size_in_bytes;

	unsigned int column_count;

	col_def col_definitions[];
};

void init_row_def(row_def* row_d);

int insert_col_def(row_def* row_d, datatype col_type, unsigned int col_size_in_bytes);

#endif