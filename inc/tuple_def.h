#ifndef TUPLE_DEF_H
#define TUPLE_DEF_H

#include<stdio.h>
#include<stdint.h>

typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

typedef int8_t   i1;
typedef int16_t  i2;
typedef int32_t  i4;
typedef int64_t  i8;

typedef float    f4;
typedef double   f8;

typedef enum type type;
enum type
{
	UINT	= 0,
	INT 	= 1,
	FLOAT	= 2,
	STRING  = 3,
	BLOB 	= 4
	// a BLOB type may be preceded with a UINT type
	// such that the preceded UINT type represents its size in bytes
};

extern char type_as_string[][6];

typedef union element element;
union element
{
	u1* 	UINT_1;
	u2* 	UINT_2;
	u4* 	UINT_4;
	u8* 	UINT_8;

	i1*		INT_1;
	i2* 	INT_2;
	i4* 	INT_4;
	i8* 	INT_8;

	f4* 	FLOAT_4;
	f8* 	FLOAT_8;

	char*	STRING;

	void* 	BLOB;
};

#define VARIABLE_SIZED 0

typedef struct element_def element_def;
struct element_def
{
	// datatype stored in the element
	type type;

	// size in bytes that may be occupied by the element
	// size may be equal to VARIABLE_SIZED only for STRING and BLOB datatypes
	uint64_t size;

	// byte offset in tuple for the given element definition
	// valid only if the tuple is not VARIABLE_SIZED
	uint64_t byte_offset;
};

typedef struct tuple_def tuple_def;
struct tuple_def
{
	// total size of tuple in bytes
	// size = VARIABLE_SIZED, if atleast one of the element_defs is VARIABLE_SIZED
	uint64_t size;

	// total elements in the tuple
	uint64_t element_count;

	// definition of all of the elements
	element_def element_defs[];
};

// to initialize a tuple definition
void init_tuple_def(tuple_def* tuple_d);

// insert the key or values, insert keys in their decreasing order of importance
// mark the tuple_mark_key_complete, once all the keys are inserted
int insert_element_def(tuple_def* tuple_d, type element_type, uint64_t element_size);

// after inserting all the elements call this function
void finalize_tuple_def(tuple_def* tuple_d);

// to print a final tuple definition
void print_tuple_def(tuple_def* tuple_d);

#endif