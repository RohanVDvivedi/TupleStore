#ifndef USER_VALUE_H
#define USER_VALUE_H

#include<stdint.h>

#include<large_uint.h>

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
		large_uint large_uint_value;
		uint64_t bit_field_vale;
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

int is_user_value_NULL(const user_value* uval);

user_value clone_user_value_with_data(const user_value* uval);

// forward declaration for element_def struct
typedef struct element_def element_def;
struct element_def;

user_value get_MIN_user_value(const element_def* ele_d);

user_value get_MAX_user_value_for_numeral_element_def(const element_def* ele_d);

void print_user_value(const user_value* uval, const element_def* ele_d);

#endif