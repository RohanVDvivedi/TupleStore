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

	// if a non container was indexed OR index provided was out of bounds for that container then this bit is set
	int is_OUT_OF_BOUNDS:1;

	// else one of the attributes of this union must be set, based on the given element_def
	union
	{
		uint64_t bit_field_value;
		uint64_t uint_value;
		int64_t int_value;
		float float_value;
		double double_value;
		long double long_double_value;
		uint256 large_uint_value;
		struct
		{
			const void* string_value;
			uint32_t string_size;
		};
		struct
		{
			const void* blob_value;
			uint32_t blob_size;
		};
		const void* tuple_value;
		const void* array_value;

		// TODO : below attributes are to be discarded
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
extern user_value const * const OUT_OF_BOUNDS_USER_VALUE;
extern user_value const * const ZERO_USER_VALUE;
extern user_value const * const EMPTY_USER_VALUE; // same value as ZERO_USER_VALUE, but generally used for conatiners like STRING, BLOB, TUPLE and ARRAY; tuple_value and array_value here are NULLs but they are considered as if this pointer points to their most minimally initialized value

// ZERO_USER_VALUE and EMPTY_USER_VALUE are identical and fully interchangeable, they mostly are just greater than NULL_USER_VALUE, but this is not guaranteed for custim user-defined types

int is_user_value_NULL(const user_value* uval);

int is_user_value_OUT_OF_BOUNDS(const user_value* uval);

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