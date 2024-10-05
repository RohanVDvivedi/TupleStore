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

extern user_value const * const NULL_USER_VALUE;
extern user_value const * const OUT_OF_BOUNDS_USER_VALUE; // OUT_OF_BOUNDS_USER_VALUE must have its is_NULL bit set, it is logically NULL also

extern user_value const * const ZERO_USER_VALUE;
extern user_value const * const EMPTY_USER_VALUE; // same value as ZERO_USER_VALUE, but generally used for conatiners like STRING, BLOB, TUPLE and ARRAY; tuple_value and array_value here are NULLs but they are considered as if this pointer points to their most minimally initialized value
// EMPTY_USER_VALUE can only be used for setting TUPLE and ARRAY types to their empty initialized forms using set_user_value_* and can_set_user_value_* functions in data_type_info.h and set_element_in_tuple and can_set_element_in_tuple functions in tuple.h
// you must never pass them to hash and compare functions below
// ZERO_USER_VALUE and EMPTY_USER_VALUE must not be passed to any function except the ones that are used for setting the attribute or initializing the data

int is_user_value_NULL(const user_value* uval);

int is_user_value_OUT_OF_BOUNDS(const user_value* uval);

#include<data_type_info.h>

// only a valid function calls for container_type_info -> STRING, BLOB, TUPLE and ARRAY
uint32_t get_element_count_for_user_value(const user_value* uval, const data_type_info* dti);
const user_value get_containee_for_user_value(const user_value* uval, const data_type_info* dti, uint32_t index);

int can_compare_user_value(const data_type_info* dti1, const data_type_info* dti2);

int compare_user_value(const user_value* uval1, const data_type_info* dti1, const user_value* uval2, const data_type_info* dti2);

uint64_t hash_user_value(const user_value* uval, const data_type_info* dti, uint64_t (*hash_func)(const void* data, uint32_t data_size));

void print_user_value(const user_value* uval, const data_type_info* dti);

#endif