#include<tuple_def.h>

#include<string.h>

char type_as_string[][8] = {
								"UINT",
								"INT",
								"FLOAT",
								"STRING",
								"BLOB"
							};

int is_size_allowed(element_type ele_type, uint32_t size)
{
	switch(ele_type)
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

int init_element_def(element_def* element_d, element_type ele_type, uint32_t size)
{
	if(!is_size_allowed(ele_type, size))
		return 0;

	element_d->type = ele_type;
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

int insert_element_def(tuple_def* tuple_d, element_type ele_type, uint32_t element_size)
{
	// for a variable sized element, the size is given by 
	// a preceding element of UINT data type of size (1,2, or 4)
	if((element_size == VARIABLE_SIZED) && (tuple_d->element_count < 1 || !is_size_specifying_element(tuple_d, tuple_d->element_count - 1)))
		return 0;

	// if an element is not of appropriate size it can not be initialized
	element_def* new_element_def = tuple_d->element_defs + tuple_d->element_count;
	if(!init_element_def(new_element_def, ele_type, element_size))
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
		if(is_variable_sized_element(tuple_d, i))
		{
			tuple_d->size = VARIABLE_SIZED;
			return;
		}
	}
}

int is_empty_tuple_def(const tuple_def* tuple_d)
{
	// an empty tuple definition is of no use to us
	// an empty tuple definition can not represent any set of tuples
	return tuple_d->element_count == 0;
}

int is_variable_sized_element(const tuple_def* tuple_d, uint16_t index)
{
	// index out of bounds
	if(index >= tuple_d->element_count)
		return 0;

	return tuple_d->element_defs[index].size == VARIABLE_SIZED;
}

int is_size_specifying_element(const tuple_def* tuple_d, uint16_t index)
{
	// there must be atleast 2 elements, so that one element specifies the size of the other
	if(tuple_d->element_count < 2)
		return 0;

	// index out of bounds
	if(index >= tuple_d->element_count)
		return 0;

	// returns 1, if the next element is a variable sized element
	// and the index-ed element is of type UINT, with size lesser than 8
	return (tuple_d->element_defs[index].type == UINT)
		&& (tuple_d->element_defs[index].size <= 4)
		&& is_variable_sized_element(tuple_d, index + 1);
}

uint32_t get_minimum_tuple_size(const tuple_def* tpl_d)
{
	// if the tuple is not variable sized
	// i.e. if the tuple is fixed sized, then return its size
	if(tpl_d->size != VARIABLE_SIZED)
		return tpl_d->size;

	// for a VARIABLE_SIZED tuple definition
	// consider all VARIABLE_SIZED elements to be of size 0
	uint32_t minimum_size = 0;
	for(int i = 0; i < tpl_d->element_count; i++)
		minimum_size += (is_variable_sized_element(tpl_d, i) ? 0 : tpl_d->element_defs[i].size);
	return minimum_size;
}

static void print_element_def(const element_def* element_d)
{
	printf("\t\t\t type : %s\n", type_as_string[element_d->type]);
	if(element_d->size == VARIABLE_SIZED)
		printf("\t\t\t size : VARIABLE_SIZED (0)\n");
	else
		printf("\t\t\t size : %u\n", element_d->size);
	printf("\t\t\t byte_offset : %u\n", element_d->byte_offset);
}

void print_tuple_def(const tuple_def* tuple_d)
{
	printf("Tuple definition : \n");
	if(tuple_d->size == VARIABLE_SIZED)
		printf("\t tuple_size : VARIABLE_SIZED (0)\n");
	else
		printf("\t tuple_size : %u\n", tuple_d->size);
	printf("\t elements : (%u)\n", tuple_d->element_count);
	for(uint16_t i = 0; i < tuple_d->element_count; i++)
	{
		printf("\t\t Column : %u\n", i);
		print_element_def((tuple_d->element_defs) + i);
	}
}