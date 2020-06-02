#include<row_def.h>

void init_row_def(row_def* row_d)
{
	row_d->size_in_bytes = 0;
	row_d->column_count = 0;
}

int insert_col_def(row_def* row_d, datatype col_type, unsigned int col_size_in_bytes)
{
	col_def* new_column = &(row_d->col_definitions[row_d->column_count]);
	if(init_col_def(new_column, col_type, col_size_in_bytes))
	{
		new_column->byte_offset = row_d->size_in_bytes;
		row_d->size_in_bytes += new_column->size_in_bytes;
		row_d->column_count += 1;
		return 1;
	}
	return 0;
}