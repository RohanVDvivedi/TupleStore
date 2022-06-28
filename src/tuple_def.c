#include<tuple_def.h>

#include<string.h>

#include<bitmap.h>

#include<int_accesses.h>
#include<page_layout_util.h>
#include<numeral_element_types.h>
#include<non_numeral_element_types.h>

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
			return (size != 1) && (size <= 8);
		case FLOAT :
			return (size == sizeof(float)) || (size == sizeof(double));
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
			return (prefix_size != 0) && (prefix_size <= 4);
		default :
			return 0;
	}
}

int init_element_def(element_def* element_d, const char* name, element_type ele_type, uint32_t size_OR_prefix_size)
{
	// name larger than 63 bytes
	if(strnlen(name, 64) == 64)
		return 0;

	if(is_variable_sized_element_type(ele_type))
	{
		if(!is_prefix_size_allowed_for_variable_sized_type(ele_type, size_OR_prefix_size))
			return 0;

		element_d->size_specifier_prefix_size = size_OR_prefix_size;

		// set the byte_offset_to_byte_offset to 0
		// we don't know it yet
		element_d->byte_offset_to_byte_offset = 0;
	}
	else
	{
		if(!is_size_allowed_for_fixed_sized_type(ele_type, size_OR_prefix_size))
			return 0;

		element_d->size = size_OR_prefix_size;

		// set the byte_offset to 0
		// we don't know it yet
		element_d->byte_offset = 0;
	}

	// set type to the ele_type as in parameter
	element_d->type = ele_type;

	// add name to this element definition
	strncpy(element_d->name, name, 63);
	element_d->name[63] = '\0';

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
	else if(is_variable_sized_non_numeral_element_def(ele_d))
		return get_element_size_for_variable_sized_non_numeral_element(e.BLOB, ele_d);
	return 0;
}

int can_compare_element_defs(const element_def* ele_d_1, const element_def* ele_d_2)
{
	if(is_numeral_type_element_def(ele_d_1) && is_numeral_type_element_def(ele_d_2))
		return 1;
	else if(is_string_type_element_def(ele_d_1) && is_string_type_element_def(ele_d_2))
		return 1;
	else if(is_blob_type_element_def(ele_d_1) && is_blob_type_element_def(ele_d_2))
		return 1;
	return 0;
}

int compare_elements(element e1, const element_def* ele_d_1, element e2, const element_def* ele_d_2)
{
	if(is_numeral_type_element_def(ele_d_1) && is_numeral_type_element_def(ele_d_2))
		return compare_numeral_type_elements(e1.BLOB, ele_d_1, e2.BLOB, ele_d_2);
	else if(is_string_type_element_def(ele_d_1) && is_string_type_element_def(ele_d_2))
		return compare_string_type_elements(e1.BLOB, ele_d_1, e2.BLOB, ele_d_2);
	else if(is_blob_type_element_def(ele_d_1) && is_blob_type_element_def(ele_d_2))
		return compare_blob_type_elements(e1.BLOB, ele_d_1, e2.BLOB, ele_d_2);;

	// a return value of -2 implies in comparable types
	return -2;
}

uint32_t hash_element(element e, const element_def* ele_d, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	// for a STRING or VAR_STRING type the size is the capacity, not the actual size, 
	// the string may be smaller than the size
	if(ele_d->type == STRING)
		return hash_func(e.STRING, get_string_length_for_string_type_element(e.BLOB, ele_d));
	else if(is_variable_sized_non_numeral_element_def(ele_d)) // this works because for VAR_STRING, data_size is same as string length
		return hash_func(get_data_for_variable_sized_non_numeral_element(e.BLOB, ele_d), get_data_size_for_variable_sized_non_numeral_element(e.BLOB, ele_d));
	else // else this is fixed sized element def (except for STRING)
		return hash_func(e.BLOB, get_element_size(e, ele_d));

	return 0;
}

int init_tuple_def(tuple_def* tuple_d, const char* name)
{
	// name larger than 63 bytes
	if(strnlen(name, 64) == 64)
		return 0;

	tuple_d->is_variable_sized = 0;
	tuple_d->size = 0;
	tuple_d->byte_offset_to_is_null_bitmap = 0;
	tuple_d->element_count = 0;

	// copy name
	strncpy(tuple_d->name, name, 63);
	tuple_d->name[63] = '\0';

	return 1;
}

int insert_element_def(tuple_def* tuple_d, const char* name, element_type ele_type, uint32_t element_size_OR_prefix_size)
{
	// if an element definition by the name already exists then we fail an insertion
	if(get_element_def_id_by_name(tuple_d, name) != ELEMENT_DEF_NOT_FOUND)
		return 0;

	// attempt initializing the ith element def
	element_def* new_element_def = tuple_d->element_defs + tuple_d->element_count;
	if(!init_element_def(new_element_def, name, ele_type, element_size_OR_prefix_size))
		return 0;

	tuple_d->element_count++;
	return 1;
}

int insert_copy_of_element_def(tuple_def* tuple_d, const char* name, const tuple_def* tuple_d_copy_from, uint32_t element_def_id)
{
	// element_def_id is out of bounds
	if(element_def_id >= tuple_d_copy_from->element_count)
		return 0;

	if(name == NULL)
		name = tuple_d_copy_from->element_defs[element_def_id].name;

	if(is_fixed_sized_element_def(tuple_d_copy_from->element_defs + element_def_id))
		return insert_element_def(tuple_d, name, tuple_d_copy_from->element_defs[element_def_id].type, tuple_d_copy_from->element_defs[element_def_id].size);
	else
		return insert_element_def(tuple_d, name, tuple_d_copy_from->element_defs[element_def_id].type, tuple_d_copy_from->element_defs[element_def_id].size_specifier_prefix_size);
}

void finalize_tuple_def(tuple_def* tuple_d, uint32_t max_tuple_size)
{
	// calcuate size required to store offsets on a page that is of size max_tuple_size
	uint32_t size_of_offsets = get_value_size_on_page(max_tuple_size);

	tuple_d->size = 0;
	tuple_d->is_variable_sized = 0;

	tuple_d->byte_offset_to_is_null_bitmap = 0;
	tuple_d->size += bitmap_size_in_bytes(tuple_d->element_count);

	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		if(is_variable_sized_element_def(tuple_d->element_defs + i))
		{
			tuple_d->is_variable_sized = 1;
			tuple_d->element_defs[i].byte_offset_to_byte_offset = tuple_d->min_size;
			tuple_d->min_size += size_of_offsets;
		}
		else
		{
			tuple_d->element_defs[i].byte_offset = tuple_d->size;
			tuple_d->size += tuple_d->element_defs[i].size;
		}
	}

	if(!tuple_d->is_variable_sized)
		return;

	// if the tuple_def is variable sized we move everything back by the size (in bytes) required to store the size of the tuple
	tuple_d->min_size += size_of_offsets;
	tuple_d->byte_offset_to_is_null_bitmap = size_of_offsets;
	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		if(is_variable_sized_element_def(tuple_d->element_defs + i))
			tuple_d->element_defs[i].byte_offset_to_byte_offset += size_of_offsets;
		else
			tuple_d->element_defs[i].byte_offset += size_of_offsets;
	}

}

int is_empty_tuple_def(const tuple_def* tuple_d)
{
	return tuple_d->element_count == 0;
}

int is_fixed_sized_tuple_def(const tuple_def* tuple_d)
{
	return tuple_d->is_variable_sized == 0;
}

int is_variable_sized_tuple_def(const tuple_def* tuple_d)
{
	return !is_fixed_sized_tuple_def(tuple_d);
}

uint32_t get_minimum_tuple_size(const tuple_def* tuple_d)
{
	if(is_fixed_sized_tuple_def(tuple_d))
		return tuple_d->size;
	else
		return tuple_d->min_size;
}

uint32_t get_element_def_id_by_name(const tuple_def* tuple_d, const char* name)
{
	// if the name size is more than or equal to 64 we quit
	uint32_t name_size = strnlen(name, 64);
	if(name_size == 64)
		return ELEMENT_DEF_NOT_FOUND;

	// we do a linear search here, it is not optimal
	// it should have an associative map, but I dont want to clutter the implementation of tuple def any more 
	// with any more complex data structures
	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		if(0 == strcmp(tuple_d->element_defs[i].name, name))
			return i;
	}

	return ELEMENT_DEF_NOT_FOUND;
}

static void print_element_def(const element_def* element_d)
{
	printf("\t\t\t \"%s\" of type : %s\n", element_d->name, type_as_string[element_d->type]);
	if(is_variable_sized_element_def(element_d))
	{
		printf("\t\t\t size_specifier_prefix_size : %u\n", element_d->size_specifier_prefix_size);
		printf("\t\t\t byte_offset_to_byte_offset : %u\n", element_d->byte_offset_to_byte_offset);
	}
	else
	{
		printf("\t\t\t size : %u\n", element_d->size);
		printf("\t\t\t byte_offset : %u\n", element_d->byte_offset);
	}
}

void print_tuple_def(const tuple_def* tuple_d)
{
	printf("Tuple definition for \"%s\" : \n", tuple_d->name);
	printf("is_variable_sized : %u\n", tuple_d->is_variable_sized);
	if(is_variable_sized_tuple_def(tuple_d))
		printf("\t min_size : %u\n", tuple_d->min_size);
	else
		printf("\t size : %u\n", tuple_d->size);
	printf("\t byte_offset_to_is_null_bitmap : %u\n", tuple_d->byte_offset_to_is_null_bitmap);
	printf("\t element_count : %u\n", tuple_d->element_count);
	for(uint32_t i = 0; i < tuple_d->element_count; i++)
	{
		printf("\t\t Column : %u\n", i);
		print_element_def((tuple_d->element_defs) + i);
	}
}