#include<tuple_def.h>

#include<string.h>

char type_as_string[][16] = {
								"UINT",
								"INT",
								"FLOAT",
								"STRING",
								"BLOB",
								"VAR_STRING",
								"VAR_BLOB"
							};

static int is_variable_sized_element_type(element_type ele_type)
{
	return ele_type == VAR_STRING || ele_type == VAR_BLOB;
}

static int is_size_allowed_for_fixed_sized_type(element_type ele_type, uint32_t size)
{
	switch(ele_type)
	{
		case UINT :
		case INT :
			return (size == 1) || (size == 2) || (size == 4) || (size == 8);
		case FLOAT :
			return (size == 4) || (size == 8);
		case STRING :
		case BLOB :	// STRING and BLOB can be of any size
			return 1;
		default :
			return 0;
	}
}

static int is_prefix_size_allowed_for_variable_sized_type(element_type ele_type, uint32_t prefix_size)
{
	switch(ele_type)
	{
		case VAR_STRING :
		case VAR_BLOB :
			return (prefix_size == 1) || (prefix_size == 2) || (prefix_size == 4);
		default :
			return 0;
	}
}

int init_element_def(element_def* element_d, element_type ele_type, uint32_t size_OR_prefix_size)
{
	if(is_variable_sized_element_type(ele_type))
	{
		if(!is_prefix_size_allowed_for_variable_sized_type(ele_type, size_OR_prefix_size))
			return 0;

		element_d->type = ele_type;
		element_d->size_specifier_prefix_size = size_OR_prefix_size;
		element_d->byte_offset = 0;
	}
	else
	{
		if(!is_size_allowed_for_fixed_sized_type(ele_type, size_OR_prefix_size))
			return 0;

		element_d->type = ele_type;
		element_d->size = size_OR_prefix_size;
		element_d->byte_offset = 0;
	}
	return 1;
}

int is_variable_sized_element_def(const element_def* element_d)
{
	return is_variable_sized_element_type(element_d->type);
}

int is_fixed_sized_element_def(const element_def* element_d)
{
	return !is_variable_sized_element_def(element_d);
}

uint32_t get_element_size(element e, const element_def* ele_d)
{
	if(is_fixed_sized_element_def(ele_d))
		return ele_d->size;
	else
	{
		switch(ele_d->size_specifier_prefix_size)
		{
			case 1 :
				return 1 + e.VAR_STRING_1->size;
			case 2 :
				return 2 + e.VAR_STRING_2->size;
			case 4 :
				return 4 + e.VAR_STRING_4->size;
			default :
				return 0;
		}
	}
}

#define compare(a,b)	( ((a)>(b)) ? 1 : (((a)<(b)) ? (-1) : 0 ) )
int compare_elements(element e1, element e2, const element_def* ele_d)
{
	// return if an element is not fixed sized
	if(!is_fixed_sized_element_def(ele_d))
		return 0;

	switch(ele_d->type)
	{
		case UINT :
		{
			switch(ele_d->size)
			{
				case 1 :
					return compare(*e1.UINT_1, *e2.UINT_1);
				case 2 :
					return compare(*e1.UINT_2, *e2.UINT_2);
				case 4 :
					return compare(*e1.UINT_4, *e2.UINT_4);
				case 8 :
					return compare(*e1.UINT_8, *e2.UINT_8);
			}
		}
		case INT :
		{
			switch(ele_d->size)
			{
				case 1 :
					return compare(*e1.INT_1, *e2.INT_1);
				case 2 :
					return compare(*e1.INT_2, *e2.INT_2);
				case 4 :
					return compare(*e1.INT_4, *e2.INT_4);
				case 8 :
					return compare(*e1.INT_8, *e2.INT_8);
			}
		}
		case FLOAT :
		{
			switch(ele_d->size)
			{
				case 4 :
					return compare(*e1.FLOAT_4, *e2.FLOAT_4);
				case 8 :
					return compare(*e1.FLOAT_8, *e2.FLOAT_8);
			}
		}
		case STRING :
			return strncmp(e1.STRING, e2.STRING, ele_d->size);
		case BLOB :
			return memcmp(e1.STRING, e2.STRING, ele_d->size);
		case VAR_STRING :
		{
			uint32_t size1;
			uint32_t size2;

			uint32_t min_size;

			int compare = 0;

			switch(ele_d->size_specifier_prefix_size)
			{
				case 1 :
				{
					size1 = strnlen(e1.VAR_STRING_1->string, e1.VAR_STRING_1->size);
					size2 = strnlen(e2.VAR_STRING_1->string, e2.VAR_STRING_1->size);
					min_size = (size1 < size2) ? size1 : size2;
					compare = strncmp(e1.VAR_STRING_1->string, e2.VAR_STRING_1->string, min_size);
					break;
				}
				case 2 :
				{
					size1 = strnlen(e1.VAR_STRING_2->string, e1.VAR_STRING_2->size);
					size2 = strnlen(e2.VAR_STRING_2->string, e2.VAR_STRING_2->size);
					min_size = (size1 < size2) ? size1 : size2;
					compare = strncmp(e1.VAR_STRING_2->string, e2.VAR_STRING_2->string, min_size);
					break;
				}
				case 4 :
				{
					size1 = strnlen(e1.VAR_STRING_4->string, e1.VAR_STRING_4->size);
					size2 = strnlen(e2.VAR_STRING_4->string, e2.VAR_STRING_4->size);
					min_size = (size1 < size2) ? size1 : size2;
					compare = strncmp(e1.VAR_STRING_4->string, e2.VAR_STRING_4->string, min_size);
					break;
				}
			}

			if(compare > 0)
				compare = 1;
			else if(compare < 0)
				compare = -1;
			else if((compare == 0) && (size1 != size2))
			{
				// in dictionary ordering if 1 string is a prefix of the other
				// then the larger string comes latter in the order
				if(size1 > size2)
					compare = -1;
				else if(size1 > size2)
					compare = 1;
			}

			return compare;
		}
		case VAR_BLOB :
		{
			uint32_t size1;
			uint32_t size2;

			uint32_t min_size = (size1 < size2) ? size1 : size2;

			int compare = 0;

			switch(ele_d->size_specifier_prefix_size)
			{
				case 1 :
				{
					size1 = e1.VAR_BLOB_1->size;
					size2 = e2.VAR_BLOB_1->size;
					min_size = (size1 < size2) ? size1 : size2;
					compare = memcmp(e1.VAR_BLOB_1->blob, e2.VAR_BLOB_1->blob, min_size);
					break;
				}
				case 2 :
				{
					size1 = e1.VAR_BLOB_2->size;
					size2 = e2.VAR_BLOB_2->size;
					min_size = (size1 < size2) ? size1 : size2;
					compare = memcmp(e1.VAR_BLOB_2->blob, e2.VAR_BLOB_2->blob, min_size);
					break;
				}
				case 4 :
				{
					size1 = e1.VAR_BLOB_4->size;
					size2 = e2.VAR_BLOB_4->size;
					min_size = (size1 < size2) ? size1 : size2;
					compare = memcmp(e1.VAR_BLOB_4->blob, e2.VAR_BLOB_4->blob, min_size);
					break;
				}
			}

			if(compare > 0)
				compare = 1;
			else if(compare < 0)
				compare = -1;
			else if((compare == 0) && (size1 != size2))
			{
				// in dictionary ordering if 1 string is a prefix of the other
				// then the larger string comes latter in the order
				if(size1 > size2)
					compare = -1;
				else if(size1 > size2)
					compare = 1;
			}

			return compare;
		}
	}
	return 0;
}

void init_tuple_def(tuple_def* tuple_d)
{
	tuple_d->size = 0;
	tuple_d->element_count = 0;
}

int insert_element_def(tuple_def* tuple_d, element_type ele_type, uint32_t element_size_OR_prefix_size)
{
	// if an element is not approved of appropriate size it can not be initialized
	element_def* new_element_def = tuple_d->element_defs + tuple_d->element_count;
	if(!init_element_def(new_element_def, ele_type, element_size_OR_prefix_size))
		return 0;

	tuple_d->element_count++;
	return 1;
}

void finalize_tuple_def(tuple_def* tuple_d)
{
	tuple_d->size = 0;
	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		if(is_variable_sized_element_def(tuple_d->element_defs + i))
		{
			tuple_d->size = VARIABLE_SIZED;
			break;
		}

		tuple_d->element_defs[i].byte_offset = tuple_d->size;
		tuple_d->size += tuple_d->element_defs[i].size;
	}
}

int is_empty_tuple_def(const tuple_def* tuple_d)
{
	return tuple_d->element_count == 0;
}

int is_fixed_sized_tuple_def(const tuple_def* tuple_d)
{
	return tuple_d->element_count == 0 || tuple_d->size != VARIABLE_SIZED;
}

int is_variable_sized_tuple_def(const tuple_def* tuple_d)
{
	return !is_fixed_sized_tuple_def(tuple_d);
}

uint32_t get_minimum_tuple_size(const tuple_def* tuple_d)
{
	// if the tuple is not variable sized
	// i.e. if the tuple is fixed sized, then return its size
	if(is_fixed_sized_tuple_def(tuple_d))
		return tuple_d->size;

	// for a VARIABLE_SIZED tuple definition
	// consider all VARIABLE_SIZED elements to be of size of their prefix
	uint32_t minimum_size = 0;
	for(int i = 0; i < tuple_d->element_count; i++)
	{
		if(is_variable_sized_element_def(tuple_d->element_defs + i))
			minimum_size += tuple_d->element_defs[i].size_specifier_prefix_size;
		else
			minimum_size += tuple_d->element_defs[i].size;
	}
	return minimum_size;
}

static void print_element_def(const element_def* element_d)
{
	printf("\t\t\t type : %s\n", type_as_string[element_d->type]);
	if(is_variable_sized_element_def(element_d))
		printf("\t\t\t size : VARIABLE_SIZED (%d + 0)\n", element_d->size_specifier_prefix_size);
	else
		printf("\t\t\t size : %u\n", element_d->size);
	printf("\t\t\t byte_offset : %u\n", element_d->byte_offset);
}

void print_tuple_def(const tuple_def* tuple_d)
{
	printf("Tuple definition : \n");
	if(is_variable_sized_tuple_def(tuple_d))
		printf("\t tuple_size : VARIABLE_SIZED (0)\n");
	else
		printf("\t tuple_size : %u\n", tuple_d->size);
	printf("\t elements : (%u)\n", tuple_d->element_count);
	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		printf("\t\t Column : %u\n", i);
		print_element_def((tuple_d->element_defs) + i);
	}
}