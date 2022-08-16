#ifndef USER_VALUE_H
#define USER_VALUE_H

#include<stdint.h>

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

user_value get_MAX_user_value(const element_def* ele_d);

user_value get_MIN_user_value(const element_def* ele_d);

// forward declaration for element_def struct
typedef struct element_def element_def;
struct element_def;

void print_user_value(const user_value* uval, const element_def* ele_d);

#endif