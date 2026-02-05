#ifndef DATUM_H
#define DATUM_H

#include<stdint.h>

#include<serint/large_uints.h>
#include<serint/large_ints.h>

// user passes the parameters for assignement using this union

typedef struct datum datum;
struct datum
{
	// if the datum is NULL this bit must be set
	int is_NULL;

	// else one of the attributes of this union must be set, based on the given element_def
	union
	{
		uint64_t bit_field_value;
		uint64_t uint_value;
		int64_t int_value;
		float float_value;
		double double_value;
		uint256 large_uint_value;
		int256 large_int_value;
		struct
		{
			const void* string_value;
			uint32_t string_size;
		};
		struct
		{
			const void* binary_value;
			uint32_t binary_size;
		};
		struct
		{
			const void* string_or_binary_value;
			uint32_t string_or_binary_size;
		};
		// above 3 structs to access string and binary values must be identical, and can be used interchangeably
		const void* tuple_value;
		const void* array_value;
	};
};

extern datum const * const NULL_DATUM;

extern datum const * const ZERO_DATUM;
extern datum const * const EMPTY_DATUM; // same value as ZERO_DATUM, but generally used for conatiners like STRING, BINARY, TUPLE and ARRAY; tuple_value and array_value here are NULLs but they are considered as if this pointer points to their most minimally initialized value
// EMPTY_DATUM can only be used for setting TUPLE and ARRAY types to their empty initialized forms using set_datum_* and can_set_datum_* functions in data_type_info.h and set_element_in_tuple and can_set_element_in_tuple functions in tuple.h
// you must never pass them to hash and compare functions below
// ZERO_DATUM and EMPTY_DATUM must not be passed to any function except the ones that are used for setting the attribute or initializing the data

static inline int is_datum_NULL(const datum* uval)
{
	return (uval == NULL) || (uval->is_NULL);
}

#include<tuplestore/data_type_info.h>

// only a valid function calls for container_type_info -> STRING, BINARY, TUPLE and ARRAY
uint32_t get_element_count_for_datum(const datum* uval, const data_type_info* dti);
int get_containee_for_datum(datum* uval_c, const datum* uval, const data_type_info* dti, uint32_t index);

int can_compare_datum(const data_type_info* dti1, const data_type_info* dti2);

int compare_datum(const datum* uval1, const data_type_info* dti1, const datum* uval2, const data_type_info* dti2);

// this function is added to skip the checks that ensure that the datums can be compared, so that such checks can be discarded
int compare_datum2(const datum* uval1, const datum* uval2, const data_type_info* dti);

#include<tuplestore/tuple_hasher.h>
uint64_t hash_datum(const datum* uval, const data_type_info* dti, tuple_hasher* th);

void print_datum(const datum* uval, const data_type_info* dti);

#endif