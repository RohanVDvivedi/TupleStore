#ifndef USER_VALUE_H
#define USER_VALUE_H

#include<stdint.h>

#include<large_uints.h>

// user passes the parameters for assignement using this union

typedef struct user_value user_value;
struct user_value
{
	// if the user_value is NULL this bit must be set
	int is_NULL:1;

	// else one of the attributes of this union must be set, based on the given element_def
	union
	{
		uint64_t uint_value;
		int64_t int_value;
		float float_value;
		double double_value;
		long double long_double_value;
		uint256 large_uint_value;
		uint64_t bit_field_value;
		struct
		{
			const void* data;
			uint32_t data_size;
		};
	};
};

// use default user value to make the set functions to set the value in tuple to a defautl value
extern user_value const * const DEFAULT_USER_VALUE;

extern user_value const * const NULL_USER_VALUE;
extern user_value const * const ZERO_USER_VALUE;
extern user_value const * const EMPTY_USER_VALUE; // user_value for empty BLOB, VAR_BLOB, VAR_STRING and STRING types

int is_user_value_NULL(const user_value* uval);

// you need to call free on the return_value.data, on the object created using this function
user_value clone_user_value_with_data(const user_value* uval);

// forward declaration for element_def struct
typedef struct element_def element_def;
struct element_def;

user_value get_MIN_user_value(const element_def* ele_d);

user_value get_MAX_user_value_for_numeral_element_def(const element_def* ele_d);

// returns max user value for string or blob type element,
// limits the returned size to max_length or the maximum size of the element
// on failure to allocate memory memory_allocation_error will be set
// returns a list of "\xff\xff..."
user_value get_MAX_user_value_for_string_OR_blob_element_def(const element_def* ele_d, uint32_t max_length, int* memory_allocation_error);

void print_user_value(const user_value* uval, const element_def* ele_d);

#endif