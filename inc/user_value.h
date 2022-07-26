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

extern const user_value NULL_USER_VALUE;
extern const user_value ZERO_USER_VALUE;

int is_user_value_NULL(const user_value* uval);

#endif