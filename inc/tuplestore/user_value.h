#ifndef USER_VALUE_H
#define USER_VALUE_H

#include<stdint.h>

#include<serint/large_uints.h>
#include<serint/large_ints.h>

// user passes the parameters for assignement using this union

typedef struct user_value user_value;
struct user_value
{
	// if the user_value is NULL this bit must be set
	int is_NULL;

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
		int256 large_int_value;
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
		struct
		{
			const void* string_or_blob_value;
			uint32_t string_or_blob_size;
		};
		// above 3 structs to access string and blob values must be identical, and can be used interchangeably
		const void* tuple_value;
		const void* array_value;
	};
};

extern user_value const * const NULL_USER_VALUE;

extern user_value const * const ZERO_USER_VALUE;
extern user_value const * const EMPTY_USER_VALUE; // same value as ZERO_USER_VALUE, but generally used for conatiners like STRING, BLOB, TUPLE and ARRAY; tuple_value and array_value here are NULLs but they are considered as if this pointer points to their most minimally initialized value
// EMPTY_USER_VALUE can only be used for setting TUPLE and ARRAY types to their empty initialized forms using set_user_value_* and can_set_user_value_* functions in data_type_info.h and set_element_in_tuple and can_set_element_in_tuple functions in tuple.h
// you must never pass them to hash and compare functions below
// ZERO_USER_VALUE and EMPTY_USER_VALUE must not be passed to any function except the ones that are used for setting the attribute or initializing the data

static inline int is_user_value_NULL(const user_value* uval)
{
	return (uval == NULL) || (uval->is_NULL);
}

#include<tuplestore/data_type_info.h>

// only a valid function calls for container_type_info -> STRING, BLOB, TUPLE and ARRAY
uint32_t get_element_count_for_user_value(const user_value* uval, const data_type_info* dti);
int get_containee_for_user_value(user_value* uval_c, const user_value* uval, const data_type_info* dti, uint32_t index);

int can_compare_user_value(const data_type_info* dti1, const data_type_info* dti2);

int compare_user_value(const user_value* uval1, const data_type_info* dti1, const user_value* uval2, const data_type_info* dti2);

// this function is added to skip the checks that ensure that the user_values can be compared, so that such checks can be discarded
int compare_user_value2(const user_value* uval1, const user_value* uval2, const data_type_info* dti);

#include<tuplestore/tuple_hasher.h>
uint64_t hash_user_value(const user_value* uval, const data_type_info* dti, tuple_hasher* th);

void print_user_value(const user_value* uval, const data_type_info* dti);

#endif