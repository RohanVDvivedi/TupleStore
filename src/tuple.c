#include<tuple.h>

element get_cell(tuple_def* tpl_d, void* tupl, unsigned int column_no)
{
	element e;
	e.GENERIC = (tupl + (tpl_d->element_defs[column_no].offset));
	return e;
}

void set_cell(tuple_def* tpl_d, void* tupl, unsigned int column_no, void* value)
{
	void* vp = (tupl + (tpl_d->element_defs[column_no].offset));
	memcpy(vp, value, tpl_d->element_defs[column_no].size);
}