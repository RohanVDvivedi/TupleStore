#include<tuple_def.h>

#include<string.h>
#include<stdlib.h>

#include<bitmap.h>

#include<page_layout_util.h>
#include<numeral_element_types.h>
#include<string_or_blob_element_types_util.h>

#include<serial_int.h>
#include<large_uint.h>

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
			return (1 <= size) && (size <= 8);
		case LARGE_UINT :
			return (1 <= size) && (size <= LARGE_UINT_MAX_BYTES);
		case FLOAT :
			return (size == sizeof(float)) || (size == sizeof(double)) || (size == sizeof(long double));
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

static int init_element_def(element_def* element_d, const char* name, element_type ele_type, uint32_t size_OR_prefix_size, int is_non_NULLable, const user_value* default_value)
{
	// name larger than 63 bytes
	if(strnlen(name, 64) == 64)
		return 0;

	// a non_NULLable can not have a NULL default value
	if(is_non_NULLable && is_user_value_NULL(default_value))
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

	element_d->is_non_NULLable = is_non_NULLable;

	// add name to this element definition
	strncpy(element_d->name, name, 63);
	element_d->name[63] = '\0';

	if(is_user_value_NULL(default_value))
		element_d->default_value = *NULL_USER_VALUE;

	// data and data_size in user_value is only used for string and blob data types
	// so only they need allocation
	else if(is_string_type_element_def(element_d) || is_blob_type_element_def(element_d))
		element_d->default_value = clone_user_value_with_data(default_value);
	else
		element_d->default_value = (*default_value);

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

int is_NULLable_element_def(const element_def* element_d)
{
	return !(element_d->is_non_NULLable);
}

int has_bit_in_is_NULL_bitmap(const element_def* element_d)
{
	return is_fixed_sized_element_def(element_d) && is_NULLable_element_def(element_d);
}

#define needs_bit_in_is_NULL_bitmap has_bit_in_is_NULL_bitmap

uint32_t get_element_size(const void* e, const element_def* ele_d)
{
	if(is_numeral_type_element_def(ele_d))
		return ele_d->size;
	else if(is_variable_sized_string_OR_blob_element_def(ele_d))
		return get_element_size_for_string_OR_blob_element(e, ele_d);
	return 0;
}

uint32_t get_element_size_from_user_value(const user_value* uval, const element_def* ele_d)
{
	if(is_numeral_type_element_def(ele_d))
		return ele_d->size;
	else if(is_variable_sized_string_OR_blob_element_def(ele_d))
		return get_element_size_from_user_value_for_string_OR_blob_element(uval, ele_d);
	return 0;
}

int can_compare_element_defs(const element_def* ele_d_1, const element_def* ele_d_2)
{
	if(is_numeral_type_element_def(ele_d_1) && is_numeral_type_element_def(ele_d_2))
		return can_compare_numeral_type_element_def(ele_d_1, ele_d_2);
	else if(is_string_type_element_def(ele_d_1) && is_string_type_element_def(ele_d_2))
		return 1;
	else if(is_blob_type_element_def(ele_d_1) && is_blob_type_element_def(ele_d_2))
		return 1;
	return 0;
}

int can_set_from_element_defs(const element_def* ele_d_1, const element_def* ele_d_2)
{
	// ideal scenario, if you can compare elements, then you can assign one to the another
	return can_compare_element_defs(ele_d_1, ele_d_2);
}

int compare_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2)
{
	if(is_numeral_type_element_def(ele_d_1) && is_numeral_type_element_def(ele_d_2))
		return compare_numeral_type_elements(e1, ele_d_1, e2, ele_d_2);
	else if(is_string_type_element_def(ele_d_1) && is_string_type_element_def(ele_d_2))
		return compare_string_type_elements(e1, ele_d_1, e2, ele_d_2);
	else if(is_blob_type_element_def(ele_d_1) && is_blob_type_element_def(ele_d_2))
		return compare_blob_type_elements(e1, ele_d_1, e2, ele_d_2);

	// a return value of -2 implies in comparable types
	return -2;
}

uint32_t hash_element(const void* e, const element_def* ele_d, uint32_t (*hash_func)(const void* data, uint32_t size))
{
	if(is_numeral_type_element_def(ele_d))
		return hash_numeral_type_element(e, ele_d, hash_func);
	else if(is_string_OR_blob_type_element_def(ele_d))
		return hash_string_OR_blob_type_element(e, ele_d, hash_func);

	return 0;
}

static void deinit_element_def(element_def* ele_d)
{
	// data and data_size in user_value is only used for string and blob data types
	if((is_string_type_element_def(ele_d) || is_blob_type_element_def(ele_d)) && ele_d->default_value.data != NULL)
		free((void*)(ele_d->default_value.data));
	memset(ele_d, 0, sizeof(element_def));
}

declarations_value_arraylist(element_defs_list, element_def)
#define EXPANSION_FACTOR 1.5
function_definitions_value_arraylist(element_defs_list, element_def)

static int init_tuple_def(tuple_def* tuple_d, const char* name, uint32_t max_tuple_size)
{
	// name larger than 63 bytes
	if(strnlen(name, 64) == 64)
		return 0;

	// copy name
	strncpy(tuple_d->name, name, 63);
	tuple_d->name[63] = '\0';

	tuple_d->size_def.is_variable_sized = 0;
	tuple_d->size_def.size_of_byte_offsets = get_value_size_on_page(max_tuple_size);
	tuple_d->size_def.size = 0; // this also sets min_size to 0
	tuple_d->size_def.max_size = max_tuple_size;
	tuple_d->byte_offset_to_is_null_bitmap = 0;
	tuple_d->is_NULL_bitmap_size_in_bits = 0;

	return 1;
}

tuple_def* get_new_tuple_def(const char* name, uint32_t element_capacity, uint32_t max_tuple_size)
{
	tuple_def* tuple_d = malloc(sizeof(tuple_def));
	if(tuple_d == NULL)
		return NULL;
	if(!initialize_element_defs_list(&(tuple_d->element_defs), element_capacity))
	{
		free(tuple_d);
		return NULL;
	}
	if(!init_tuple_def(tuple_d, name, max_tuple_size))
	{
		deinitialize_element_defs_list(&(tuple_d->element_defs));
		free(tuple_d);
		return NULL;
	}
	return tuple_d;
}

tuple_def* clone_tuple_def(const tuple_def* tuple_d)
{
	tuple_def* clone_tuple_d = get_new_tuple_def(tuple_d->name, get_element_def_count_tuple_def(tuple_d), get_maximum_tuple_size(tuple_d));
	for(uint32_t i = 0; i < get_element_def_count_tuple_def(tuple_d); i++)
		insert_copy_of_element_def(clone_tuple_d, NULL, tuple_d, i);
	return clone_tuple_d;
}

int insert_element_def(tuple_def* tuple_d, const char* name, element_type ele_type, uint32_t element_size_OR_prefix_size, int is_non_NULLable, const user_value* default_value)
{
	// if an element definition by the name already exists then we fail an insertion
	if(get_element_def_id_by_name(tuple_d, name) != ELEMENT_DEF_NOT_FOUND)
		return 0;

	// for a varibale sized element type, it has a prefix that suggests the bytes it will occupy
	// the size of this prefix must not be greater than size_of_byte_offsets in the tuple, since this would waste space on the tuple
	if(is_variable_sized_element_type(ele_type) && tuple_d->size_def.size_of_byte_offsets < element_size_OR_prefix_size)
		return 0;

	// attempt initializing the element def
	element_def new_element_def;
	if(!init_element_def(&new_element_def, name, ele_type, element_size_OR_prefix_size, is_non_NULLable, default_value))
		return 0;

	// if the elment_defs is full, then expand it
	if(is_full_element_defs_list(&(tuple_d->element_defs)))
		expand_element_defs_list(&(tuple_d->element_defs));

	// now push it to back the element_defs
	if(!push_back_to_element_defs_list(&(tuple_d->element_defs), &new_element_def))
	{
		deinit_element_def(&new_element_def);
		return 0;
	}

	return 1;
}

int insert_copy_of_element_def(tuple_def* tuple_d, const char* name, const tuple_def* tuple_d_copy_from, uint32_t element_def_id)
{
	// element_def_id is out of bounds
	if(element_def_id >= get_element_count_element_defs_list(&(tuple_d_copy_from->element_defs)))
		return 0;

	const element_def* def = get_element_def_by_id(tuple_d_copy_from, element_def_id);

	if(name == NULL)
		name = def->name;

	if(is_fixed_sized_element_def(def))
		return insert_element_def(tuple_d, name, def->type, def->size, def->is_non_NULLable, &(def->default_value));
	else
		return insert_element_def(tuple_d, name, def->type, def->size_specifier_prefix_size, def->is_non_NULLable, &(def->default_value));
}

// returns true if the operation would overflow OR the result would cross max_limit
// else it will perform addition
static inline int check_overflow_and_add(uint32_t* a, uint32_t b, uint32_t max_limit)
{
	// check overflow
	if(will_unsigned_sum_overflow(uint32_t, (*a), b))
		return 1;

	// check max_limit check
	uint32_t result = (*a) + b;
	if(result > max_limit)
		return 1;

	// successful addition
	(*a) = result;
	return 0;
}

int finalize_tuple_def(tuple_def* tuple_d)
{
	tuple_d->size_def.is_variable_sized = 0;
	tuple_d->is_NULL_bitmap_size_in_bits = 0;

	// figure out 2 things in this loop
	// whether the tuple_def is_variable_sized
	// and the number of the bits required in is_NULL_bitmap
	for(uint32_t i = 0; i < get_element_def_count_tuple_def(tuple_d); i++)
	{
		element_def* def = (element_def*) get_element_def_by_id(tuple_d, i);

		// if the element_def is variable_sized, then mark the tuple as variable sized
		if(is_variable_sized_element_def(def))
			tuple_d->size_def.is_variable_sized = 1;

		// give this element a bit in is_NULL_bitmap only if it needs it
		if(needs_bit_in_is_NULL_bitmap(def))
			def->is_NULL_bitmap_bit_offset = tuple_d->is_NULL_bitmap_size_in_bits++;
	}

	tuple_d->size_def.size = 0;
	tuple_d->byte_offset_to_is_null_bitmap = 0;

	// allocate space for storing tuple_size for variable sized tuple_def
	if(tuple_d->size_def.is_variable_sized)
	{
		if(check_overflow_and_add(&(tuple_d->size_def.min_size), tuple_d->size_def.size_of_byte_offsets, tuple_d->size_def.max_size))
			return 0;
		tuple_d->byte_offset_to_is_null_bitmap = tuple_d->size_def.min_size;
	}

	tuple_d->size_def.size += bitmap_size_in_bytes(tuple_d->is_NULL_bitmap_size_in_bits);

	// now we compute the offsets (and the offsets to their byte_offsets) for all the element_defs
	for(uint32_t i = 0; i < get_element_def_count_tuple_def(tuple_d); i++)
	{
		element_def* def = (element_def*) get_element_def_by_id(tuple_d, i);

		if(is_variable_sized_element_def(def))
		{
			def->byte_offset_to_byte_offset = tuple_d->size_def.min_size;
			if(check_overflow_and_add(&(tuple_d->size_def.min_size), tuple_d->size_def.size_of_byte_offsets, tuple_d->size_def.max_size))
				return 0;
		}
		else
		{
			def->byte_offset = tuple_d->size_def.size;
			if(check_overflow_and_add(&(tuple_d->size_def.size), def->size, tuple_d->size_def.max_size))
				return 0;
		}
	}

	if(tuple_d->size_def.min_size > tuple_d->size_def.max_size)
		return 0;

	return 1;
}

int is_empty_tuple_def(const tuple_def* tuple_d)
{
	return is_empty_element_defs_list(&(tuple_d->element_defs));
}

uint32_t get_element_def_count_tuple_def(const tuple_def* tuple_d)
{
	return get_element_count_element_defs_list(&(tuple_d->element_defs));
}

int is_fixed_sized_tuple_size_def(const tuple_size_def* tuple_size_d)
{
	return tuple_size_d->is_variable_sized == 0;
}

int is_fixed_sized_tuple_def(const tuple_def* tuple_d)
{
	return is_fixed_sized_tuple_size_def(&(tuple_d->size_def));
}

int is_variable_sized_tuple_size_def(const tuple_size_def* tuple_size_d)
{
	return !is_fixed_sized_tuple_size_def(tuple_size_d);
}

int is_variable_sized_tuple_def(const tuple_def* tuple_d)
{
	return is_variable_sized_tuple_size_def(&(tuple_d->size_def));
}

uint32_t get_minimum_tuple_size_using_tuple_size_def(const tuple_size_def* tuple_size_d)
{
	if(is_fixed_sized_tuple_size_def(tuple_size_d))
		return tuple_size_d->size;
	else
		return tuple_size_d->min_size;
}

uint32_t get_minimum_tuple_size(const tuple_def* tuple_d)
{
	return get_minimum_tuple_size_using_tuple_size_def(&(tuple_d->size_def));
}

uint32_t get_maximum_tuple_size_using_tuple_size_def(const tuple_size_def* tuple_size_d)
{
	return tuple_size_d->max_size;
}

uint32_t get_maximum_tuple_size(const tuple_def* tuple_d)
{
	return get_maximum_tuple_size_using_tuple_size_def(&(tuple_d->size_def));
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
	for(uint32_t i = 0; i < get_element_def_count_tuple_def(tuple_d); i++)
	{
		const element_def* def = get_element_def_by_id(tuple_d, i);
		if(0 == strcmp(def->name, name))
			return i;
	}

	return ELEMENT_DEF_NOT_FOUND;
}

const element_def* get_element_def_by_id(const tuple_def* tuple_d, uint32_t index)
{
	return get_from_front_of_element_defs_list(&(tuple_d->element_defs), index);
}

void delete_tuple_def(tuple_def* tuple_d)
{
	while(!is_empty_element_defs_list(&(tuple_d->element_defs)))
	{
		element_def* def = (element_def*) get_front_of_element_defs_list(&(tuple_d->element_defs));
		deinit_element_def(def);
		pop_front_from_element_defs_list(&(tuple_d->element_defs));
	}
	deinitialize_element_defs_list(&(tuple_d->element_defs));
	free(tuple_d);
}

static void print_element_def(const element_def* element_d)
{
	printf("\t\t\t \"%s\" of type : %s\n", element_d->name, type_as_string[element_d->type]);
	if(is_variable_sized_element_def(element_d))
	{
		printf("\t\t\t size_specifier_prefix_size : %"PRIu32"\n", element_d->size_specifier_prefix_size);
		printf("\t\t\t byte_offset_to_byte_offset : %"PRIu32"\n", element_d->byte_offset_to_byte_offset);
	}
	else
	{
		printf("\t\t\t size : %"PRIu32"\n", element_d->size);
		printf("\t\t\t byte_offset : %"PRIu32"\n", element_d->byte_offset);
	}
	printf("\t\t\t NON_NULL : %d\n", element_d->is_non_NULLable);
	if(has_bit_in_is_NULL_bitmap(element_d))
		printf("\t\t\t is_NULL_bitmap_bit_offset : %"PRIu32"\n", element_d->is_NULL_bitmap_bit_offset);
	printf("\t\t\t default : ");
	print_user_value(&(element_d->default_value), element_d);
	printf("\n");
}

void print_tuple_def(const tuple_def* tuple_d)
{
	printf("Tuple definition for \"%s\" : \n", tuple_d->name);
	printf("is_variable_sized : %d\n", tuple_d->size_def.is_variable_sized);
	if(is_variable_sized_tuple_def(tuple_d))
		printf("\t min_size : %"PRIu32"\n", get_minimum_tuple_size(tuple_d));
	else
		printf("\t size : %"PRIu32"\n", tuple_d->size_def.size);
	printf("\t byte_offset_to_is_null_bitmap : %"PRIu32"\n", tuple_d->byte_offset_to_is_null_bitmap);
	if(is_variable_sized_tuple_def(tuple_d))
		printf("\t size_of_byte_offsets : %"PRIu32"\n", tuple_d->size_def.size_of_byte_offsets);
	printf("\t is_NULL_bitmap_size_in_bits : %"PRIu32"\n", tuple_d->is_NULL_bitmap_size_in_bits);
	printf("\t element_count : %"PRIu32"\n", get_element_def_count_tuple_def(tuple_d));
	for(uint32_t i = 0; i < get_element_def_count_tuple_def(tuple_d); i++)
	{
		printf("\t\t Column : %"PRIu32"\n", i);
		const element_def* def = get_element_def_by_id(tuple_d, i);
		print_element_def(def);
	}
}