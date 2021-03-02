#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include<stdio.h>
#include<stdint.h>

/*
**
** INT_X    => signed integer of X bytes
** UINT_X   => unsigned integer of X bytes
** FLOAT_X  => floating point number of X bytes
** STR      => string that ends with '\0'
**
*/

#define DATA_TYPES_COUNT 11

typedef enum data_type data_type;
enum data_type
{
	INT_1       = 0,
	INT_2       = 1,
	INT_4       = 2,
	INT_8       = 3,

	UINT_1      = 4,
	UINT_2      = 5,
	UINT_4      = 6,
	UINT_8      = 7,

	FLOAT_4     = 8,
	FLOAT_8     = 9,

	STR         = 10

	// composite types
};

#endif