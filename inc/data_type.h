#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include<stdio.h>
#include<stdint.h>

/*
**
** INT_X       => signed integer of X bytes
** UINT_X      => unsigned integer of X bytes
** FLOAT_X     => floating point number of X bytes
** CHAR_STRING => char string can be any number of bytes string
** BLOB        => a binary large object can be any length
**
*/

#define DATA_TYPES_COUNT 12

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

	/*
	** INT_L => an integer of any number of bytes
	** the size is also serialized with its data as {size, sign, data_bytes}
	*/

	FLOAT_4     = 8,
	FLOAT_8     = 9,

	CHAR_STRING = 10,
	// the first 4 bytes represent the capacity of CHAR_STRING inside the datastore page
	// this string terminates with '\0' (which is included in the string capacity)
	// CHAR_STRING => {capacity, string, '\0'}
	// capacity >= strlen(string) + 1

	BLOB        = 11
};

#endif