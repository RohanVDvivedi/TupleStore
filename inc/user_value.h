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

#include<data_type_info.h>

int compare_user_value(const user_value* uval1, const data_type_info* dti1, const user_value* uval2, const data_type_info* dti2);

uint64_t hash_user_value(const user_value* uval, const data_type_info* dti, uint64_t (*hash_func)(const void* data, uint32_t data_size));

void print_user_value(const user_value* uval, const data_type_info* dti);

#endif