#ifndef TUPLE_DEF_H
#define TUPLE_DEF_H

#include<data_type_info.h>
#include<user_value.h>

typedef struct tuple_size_def tuple_size_def;
struct tuple_size_def
{
	// set to 1 for a variable sized tuple_def
	// The tuple is variable sized, if atleast one of the element_defs is variable sized
	int is_variable_sized;

	// size of offsets (to variable sized elements) and tuple_size (stored as the prefix of the tuple for variable sized tuples) stored in the variable length tuples
	uint32_t size_of_byte_offsets;

	// min total size of tuple in bytes
	// i.e. size of tuple when all variable sized elements are NULL
	// this will be same as tuple's size for a fixed sized tuple
	union
	{
		// defined for variable sized tuple
		uint32_t min_size;

		// defined for fixed sized tuple
		uint32_t size;
	};

	// max_size of the tuple
	uint32_t max_size;
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