#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include<stdio.h>
#include<stdint.h>

/*
**
** UINT_X   => unsigned integer of X bytes
** INT_X    => signed integer of X bytes
** FLOAT_X  => floating point number of X bytes
** SIZE_X	=> they denote size in bytes of datatype field that succeed them(the SIZE_X fields)
** STR      => string that ends with '\0', with maximum capacity given by the preceding SIZE_X
** BLOB     => binary data, with maximum size given by the preceding SIZE_X field
**
*/

#define DATA_TYPES_COUNT 11

typedef enum data_type data_type;
enum data_type
{
	// unsigned numbers
	UINT_1  = 0,
	UINT_2  = 1,
	UINT_4  = 2,
	UINT_8  = 3,

	// signed numbers
	INT_1   = 4,
	INT_2   = 5,
	INT_4   = 6,
	INT_8   = 7,

	// floats
	FLOAT_4 = 8,
	FLOAT_8 = 9,

	// sizeof() numbers [technically same as UINT_X]
	// they precede composite types
	// they are used to depict sizes of (composite) datatypes succeeding them
	SIZE_1  = 10,
	SIZE_2  = 11,
	SIZE_4  = 12,
	SIZE_8  = 13,

	// composite types
	STR     = 14,
	BLOB    = 15
	// 
	// SIZE_X represents size of a succeeding data field
	// using X number of bytes as an unsigned int
	// SIZE_X is analogous to UINT_X
	//
	// for example usage
	// 		[SIZE_2, STR] OR [SIZE_4, BLOB]
	// here the maximum capacity of the string/blob is stored in its preceding SIZE_2/SIZE_4 field
	//
};

#endif