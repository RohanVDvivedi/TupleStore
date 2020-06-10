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

typedef uint16_t byte_size;

#define CHAR_STRING_max_length 128

typedef enum type type;
enum type
{
	CHAR_STRING 	= 0,
	SIGNED_INT 		= 1,
	UNSIGNED_INT	= 2,
	FLOATING_NUM	= 3
};

extern char type_as_string[][16];

typedef struct element_def element_def;
struct element_def
{
	// byte offset in tuple for the given element definition
	byte_size offset;

	// size in bytes that is occupied by the element
	byte_size size;

	// type stored in the cell
	type type;
};

typedef union element element;
union element
{
	void* 	GENERIC;
	
	char* 	CHAR_STRING;

	i1*		SIGNED_INT_1;
	i2* 	SIGNED_INT_2;
	i4* 	SIGNED_INT_4;
	i8* 	SIGNED_INT_8;

	u1* 	UNSIGNED_INT_1;
	u2* 	UNSIGNED_INT_2;
	u4* 	UNSIGNED_INT_4;
	u8* 	UNSIGNED_INT_8;

	f4* 	FLOATING_NUM_4;
	f8* 	FLOATING_NUM_8;
};

typedef struct tuple_def tuple_def;
struct tuple_def
{
	// byte size of key in the tuple
	byte_size key_size;

	// number of elements in the key
	uint16_t key_element_count;

	// total size of tuple in bytes
	byte_size size;

	// total elements in the tuple
	uint16_t element_count;

	// definition of all of the elements, of which the first key_element_count form the key for the element
	element_def element_defs[];
};

// checks if datatype size is allowed
int is_size_allowed(type type, byte_size size);

// to initialize a tuple definition
void init_tuple_def(tuple_def* tuple_d);

// marks the tuple key complete, call this once you have inseted all the keys
void tuple_mark_key_complete(tuple_def* tuple_d);

// insert the key or values, insert keys in their decreasing order of importance
// mark the tuple_mark_key_complete, once all the keys are inserted
int insert_element_def(tuple_def* tuple_d, type element_type, byte_size element_size);

void print_tuple_def(tuple_def* tuple_d);

#endif