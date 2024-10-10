#ifndef TUPLE_DEF_H
#define TUPLE_DEF_H

#include<data_type_info.h>
#include<user_value.h>

typedef struct tuple_size_def tuple_size_def;
struct tuple_size_def
{
	int is_variable_sized;

	union
	{
		uint32_t size; // -> to be used directly if the is_variable_sized = 0

		// ------ attributes below this point only used if is_variable_sized = 1

		uint32_t min_size; // -> suggests minimum size of the tuple for a variable sized tuple
	};

	uint32_t max_size; // to read size OR element_count from prefix using read_value_from_page(, max_size)

	// below two fields are used only when is_variable_sized = 1
	int has_size_in_prefix; // -> if set directly read size from the prefix and we are done

	// ------ attributes below this point only used if is_variable_sized = 1 and has_size_in_prefix = 0

	int has_element_count_in_prefix; // -> need to do more work, to know the size if has_size_inprefix = 0, but has_element_count_in_prefix = 1

	// below attributes are only required when is_variable_sized = 1, has_size_in_prefix = 0, and has_element_count_in_prefix = 1
	// this also implies that the element_count is variable, but the individual element is fixed size (may or may not be nullable)
	// this is the case with variable size string, variable sized blob, and variable element count arrays of fixed sized elements
	int does_containee_need_is_valid_bit_in_prefix;
	int is_containee_bit_field; // -> can either be a bitfield or not
	union
	{
		uint32_t containee_size;
		uint32_t containee_bit_field_size;
	}; // -> if is_containee_bit_field gives containee_bit_field_size in bits, else the actual size of fixed sized containee in the containee_size attribute
};
// if packed this struct is no more than 1 + 32 + 32 + 1 + 1 + 1 + 1 + 32 = 101 bits = 13 bytes

typedef struct tuple_def tuple_def;
struct tuple_def
{
	// summary of the type_info for the tuple, that allows you to know the size of the tuple
	// this is mostly require by the page_layout to know the size of the tuple
	tuple_size_def size_def;

	// can be any type_info except a BIT_FIELD type
	data_type_info* type_info;
};

// initialize a tuple_size_def using the data_type_info
// it will also finalize the data_type_info
// fails with 0 only if dti->type == BIT_FIELD
int initialize_tuple_size_def(tuple_size_def* tuple_size_d, data_type_info* dti);

uint32_t get_tuple_size_using_tuple_size_def(const tuple_size_def* tpl_sz_d, const void* tupl);

int is_variable_sized_tuple_size_def(const tuple_size_def* tuple_size_d);

uint32_t get_minimum_tuple_size_using_tuple_size_def(const tuple_size_def* tuple_size_d);

uint32_t get_maximum_tuple_size_using_tuple_size_def(const tuple_size_def* tuple_size_d);

// this function is not used, it merely duplicated the logic in `uint32_t initialize_minimal_data_for_type_info(const data_type_info* dti, void* data);`
// not publicly exposed, it is here to just show that tuple_size_def is enough to minimally initialize the tuple
// uint32_t initialize_minimal_tuple_for_tuple_size_info(const tuple_size_def* tpl_sz_d, void* tupl);

// serialize deserialize function for tuple_size_def
// for serialization it is assumed that data has atleast 13 bytes to hold the serialized form of tuple_size_d
uint32_t serialize_tuple_size_def(const tuple_size_def* tuple_size_d, void* data);
int deserialize_tuple_size_def(tuple_size_def* tuple_size_d, void* data, uint32_t size);

void print_tuple_size_def(const tuple_size_def* tuple_size_d);

// initialize a tuple_def using the data_type_info
// it will also finalize the data_type_info
// fails with 0 only if dti->type == BIT_FIELD
int initialize_tuple_def(tuple_def* tuple_d, data_type_info* dti);

uint32_t get_tuple_size(const tuple_def* tpl_d, const void* tupl);

int is_variable_sized_tuple_def(const tuple_def* tuple_d);

int is_fixed_sized_tuple_def(const tuple_def* tuple_d);

uint32_t get_minimum_tuple_size(const tuple_def* tuple_d);

uint32_t get_maximum_tuple_size(const tuple_def* tuple_d);

void print_tuple_def(const tuple_def* tuple_d);

#endif