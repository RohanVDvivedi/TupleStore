#include<tuple_def.h>

int is_size_allowed(type element_type, uint16_t size)
{
	switch(element_type)
	{
		case CHAR_STRING :
			return (size <= CHAR_STRING_max_length);
		case SIGNED_INT :
		case UNSIGNED_INT :
			return (size == 1) || (size == 2) || (size == 4) || (size == 8);
		case FLOATING_NUM :
			return (size == 4) || (size == 8);
		default :
			return 0;
	}
}

int compare_signed_unsigned(int64_t a, uint64_t b)
{
	if(a < 0)
		return -1;
	else
	{
		uint64_t ua = a;
		if(ua > b)
			return 1;
		else if(ua < b)
			return -1;
	}
	return 0;
}

int compare_signed_signed(int64_t a, int64_t b)
{
	if(a > b)
		return 1;
	else if(a < b)
		return -1;
	return 0;
}

int compare_unsigned_unsigned(uint64_t a, uint64_t b)
{
	if(a > b)
		return 1;
	else if(a < b)
		return -1;
	return 0;
}

int compare_float_float(double a, double b)
{
	if(a > b)
		return 1;
	else if(a < b)
		return -1;
	return 0;
}

static int init_col_def(element_def* element_d, type element_type, uint16_t element_size_in_bytes)
{
	if(is_size_allowed(element_type, element_size_in_bytes))
	{
		element_d->type = element_type;
		element_d->size_in_bytes = element_size_in_bytes;
		return 1;
	}
	return 0;
}

void init_row_def(tuple_def* tuple_d)
{
	tuple_d->size_in_bytes = 0;
	tuple_d->element_count = 0;
}

int insert_col_def(tuple_def* tuple_d, type element_type, uint16_t element_size_in_bytes)
{
	element_def* new_element_def = &(tuple_d->element_defs[tuple_d->element_count]);
	if(init_col_def(new_element_def, element_type, element_size_in_bytes))
	{
		new_element_def->byte_offset = tuple_d->size_in_bytes;
		tuple_d->size_in_bytes += new_element_def->size_in_bytes;
		tuple_d->element_count += 1;
		return 1;
	}
	return 0;
}