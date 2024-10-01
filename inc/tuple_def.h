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
		uint32_t min_size // -> suggests minimum size of the tuple for a variable sized tuple
	};

	// below two fields are used only when is_variable_sized = 1
	int has_size_in_prefix; // -> if set directly read size from the prefix and we are done
	int has_element_count_in_pefix; // -> need to do more work, to know the size if has_size_inprefix = 0, but has_element_count_in_prefix = 1

	uint32_t max_size; // to read size OR element_count from prefix using read_value_from_page(, max_size)

	// below attributes are only required when is_variable_sized = 1, has_size_in_prefix = 0, and has_element_count_in_pefix = 1
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

typedef struct tuple_def tuple_def;
struct tuple_def
{
	// summary of the type_info for the tuple, that allows you to know the size of the tuple
	// this is mostly require by the page_layout to know the size of the tuple
	tuple_size_def size_def;

	// can be any type_info except a BIT_FIELD type
	data_type_info* type_info;
};



#endif