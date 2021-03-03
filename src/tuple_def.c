#include<tuple_def.h>

#include<string.h>

char type_as_string[][6] = {
								"UINT",
								"INT",
								"FLOAT",
								"BLOB"
							};

int is_size_allowed(type element_type, uint64_t size)
{
	switch(element_type)
	{
		case UINT :
		case INT :
			return (size == 1) || (size == 2) || (size == 4) || (size == 8);
		case FLOAT :
			return (size == 4) || (size == 8);
		case STRING :
		case BLOB :	// STRING and BLOB can be of size VARIABLE_SIZED
			return 1;
		default :
			return 0;
	}
}

int init_element_def(element_def* element_d, type type, uint64_t size)
{
	if(!is_size_allowed(type, size))
		return 0;

	element_d->type = type;
	element_d->size = size;
	element_d->byte_offset = 0;
	return 1;	
}

#define compare(a,b)	( ((a)>(b)) ? 1 : (((a)<(b)) ? (-1) : 0 ) )
int compare_fixed_sized_elements(element e1, element e2, const element_def* ele_d)
{
	if(ele_d->size == VARIABLE_SIZED)
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
	}
	return 0;
}

void init_tuple_def(tuple_def* tuple_d)
{
	tuple_d->size = 0;
	tuple_d->element_count = 0;
}

int insert_element_def(tuple_def* tuple_d, type element_type, uint64_t element_size)
{
	// for a variable sized element
	// the size must be given by a preceding element of UINT data type
	if(element_size == VARIABLE_SIZED && tuple_d->element_defs[tuple_d->element_count - 1].type != UINT)
		return 0;

	// if an element is not of appropriate size it can not be initialized
	element_def* new_element_def = tuple_d->element_defs + tuple_d->element_count;
	if(!init_element_def(new_element_def, element_type, element_size))
		return 0;

	new_element_def->byte_offset = tuple_d->size;
	
	// update the tuple size
	if(element_size == VARIABLE_SIZED)
		tuple_d->size = 0;	// if a VARIABLE_SIZED element is encountered, then restart counting
	else
		tuple_d->size += new_element_def->size;

	// increment element count in the tuple
	tuple_d->element_count += 1;
	return 1;
}

void finalize_tuple_def(tuple_def* tuple_d)
{
	for(int i = 0; i < tuple_d->element_count; i++)
	{
		if(tuple_d->element_defs[i].size == VARIABLE_SIZED)
		{
			tuple_d->size = VARIABLE_SIZED;
			return;
		}
	}
}

static void print_element_def(const element_def* element_d)
{
	printf("\t\t\t type : %s\n", type_as_string[element_d->type]);
	if(element_d->size == VARIABLE_SIZED)
		printf("\t\t\t size : VARIABLE_SIZED (0)\n");
	else
		printf("\t\t\t size : %lu\n", element_d->size);
	printf("\t\t\t byte_offset : %lu\n", element_d->byte_offset);
}

void print_tuple_def(const tuple_def* tuple_d)
{
	printf("Tuple definition : \n");
	if(tuple_d->size == VARIABLE_SIZED)
		printf("\t tuple_size : VARIABLE_SIZED (0)\n");
	else
		printf("\t tuple_size : %lu\n", tuple_d->size);
	printf("\t elements : (%lu)\n", tuple_d->element_count);
	for(uint64_t i = 0; i < tuple_d->element_count; i++)
	{
		printf("\t\t Column : %lu\n", i);
		print_element_def((tuple_d->element_defs) + i);
	}
}