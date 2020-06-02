#include<tuple.h>

cell get_cell(row_def* tpl_d, tuple tpl, unsigned int column_no)
{
	cell c;
	c.GENERIC = (tpl + (tpl_d->col_definitions[column_no].byte_offset));
	return c;
}

void set_cell(row_def* tpl_d, tuple tpl, unsigned int column_no, void* value)
{
	void* vp = (tpl + (tpl_d->col_definitions[column_no].byte_offset));
	memcpy(vp, value, tpl_d->col_definitions[column_no].size_in_bytes);
}