#ifndef DATATYPE_H
#define DATATYPE_H

#include<stdint.h>

#define CHAR_STRING_max_length 128

typedef enum datatype datatype;
enum datatype
{
	CHAR_STRING 	= 0,
	SIGNED_INT 		= 1,
	UNSIGNED_INT	= 2,
	FLOATING_NUM	= 3
};

// checks if datatype size is allowed
int is_size_allowed(datatype type, unsigned int size);

int compare_signed_unsigned(int64_t a, uint64_t b);

int compare_signed_signed(int64_t a, int64_t b);

int compare_unsigned_unsigned(uint64_t a, uint64_t b);

int compare_float_float(double a, double b);

#endif