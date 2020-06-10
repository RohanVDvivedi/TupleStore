#include<tuple_def.h>

char type_as_string[][16] = {
								"CHAR_STRING",
								"SIGNED_INT",
								"UNSIGNED_INT",
								"FLOATING_NUM"
							};

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

static int init_col_def(element_def* element_d, type type, byte_size size)
{
	if(is_size_allowed(type, size))
	{
		element_d->type = type;
		element_d->size = size;
		return 1;
	}
	return 0;
}

void init_tuple_def(tuple_def* tuple_d)
{
	tuple_d->key_size = 0;
	tuple_d->key_element_count = 0;
	tuple_d->size = 0;
	tuple_d->element_count = 0;
}

int insert_element_def(tuple_def* tuple_d, type element_type, uint16_t element_size)
{
	element_def* new_element_def = &(tuple_d->element_defs[tuple_d->element_count]);
	if(init_col_def(new_element_def, element_type, element_size))
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
		tuple_d->key_size = tuple_d->size;
		tuple_d->key_element_count = tuple_d->element_count;
	}
}

void print_tuple_def(tuple_def* tuple_d)
{
	printf("Tuple definition       : \n");
	printf("\t tuple_size in bytes : %u\n", tuple_d->size);
	printf("\t elements            : %u\n", tuple_d->element_count);
	printf("\t key_size in bytes   : %u\n", tuple_d->key_size);
	printf("\t key_elements        : %u\n", tuple_d->key_element_count);
	for(u2 i = 0; i < tuple_d->element_count; i++)
	{
		printf("\t\tColumn : %u\n", i);
		printf("\t\t\ttype              : %s\n", type_as_string[tuple_d->element_defs[i].type]);
		printf("\t\t\tsize  (bytes)     : %u\n", tuple_d->element_defs[i].size);
		printf("\t\t\toffset in tuple   : %u\n", tuple_d->element_defs[i].offset);
	}
}