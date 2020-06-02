#include<col_def.h>

int init_col_def(col_def* col_d, datatype col_type, unsigned int size_in_bytes)
{
	if(is_size_allowed(col_type, size_in_bytes))
	{
		col_d->type = col_type;
		col_d->size_in_bytes = size_in_bytes;
		return 1;
	}
	return 0;
}