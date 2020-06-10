#include<tuple_def.h>

int is_size_allowed(type element_type, byte_size size)
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

static int init_col_def(element_def* element_d, type type, byte_size size, endian endian)
{
	if(is_size_allowed(type, size))
	{
		element_d->type = type;
		element_d->size = size;
		element_d->endian = endian;
		return 1;
	}
	return 0;
}

void init_row_def(tuple_def* tuple_d)
{
	tuple_d->key_size = 0;
	tuple_d->key_element_count = 0;
	tuple_d->size = 0;
	tuple_d->element_count = 0;
}

int insert_col_def(tuple_def* tuple_d, type element_type, uint16_t element_size)
{
	element_def* new_element_def = &(tuple_d->element_defs[tuple_d->element_count]);

	// all elements default to CPU_ENDIAN (may be little or big) but all keys are little endian
	if(init_col_def(new_element_def, element_type, element_size, CPU_ENDIAN))
	{
		new_element_def->offset = tuple_d->size;
		tuple_d->size += new_element_def->size;
		tuple_d->element_count += 1;
		return 1;
	}
	return 0;
}

void tuple_mark_key_complete(tuple_def* tuple_d)
{
	for(int i = tuple_d->key_element_count; i < tuple_d->element_count; i++)
	{
		// all keys are BIG endian, irrespective of cpu endianness
		tuple_d->element_defs[i].endian = BIG;
	}
}