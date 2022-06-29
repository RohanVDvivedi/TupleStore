#ifndef USER_VALUE_H
#define USER_VALUE_H

// user passes the parameters for assignement using this union

typedef union user_value user_value;
union user_value
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

#endif