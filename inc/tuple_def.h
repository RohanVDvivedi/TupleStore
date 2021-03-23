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

typedef enum element_type element_type;
enum element_type
{
	UINT	= 0,
	INT 	= 1,
	FLOAT	= 2,
	STRING  = 3,
	BLOB 	= 4
	// data of STRING or BLOB type can be of fixed_size or be VARIABLE_SIZED

	// a VARIABLE_SIZED STRING or BLOB data type must be preceded with a UINT type of sizes (1,2 or 4)
	// such that the preceded data denotes its size in bytes
};

extern char type_as_string[][8];

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
	element_type type;

	// size in bytes that may be occupied by the element
	// size may be equal to VARIABLE_SIZED only for STRING and BLOB datatypes
	uint32_t size;

	// byte offset in tuple for the given element definition
	// valid only if the tuple is not VARIABLE_SIZED
	uint32_t byte_offset;
};

// check if a given size is valid for an element's datatype
int is_size_allowed(element_type ele_type, uint32_t size);

// initialize an element's definition using its type and size
// it may fail if the size parameters is not valid for a given data type
int init_element_def(element_def* element_d, element_type ele_type, uint32_t size);

// compare 2 elements, given their element definition
// this function must be called only on fixed sized elements i.e. ele_d->size != VARIABLE_SIZED
// if the elements are variable sized, then we do not have enough information to compare them appropriately
int compare_fixed_sized_elements(element e1, element e2, const element_def* ele_d);

typedef struct tuple_def tuple_def;
struct tuple_def
{
	// total size of tuple in bytes
	// size = VARIABLE_SIZED, if atleast one of the element_defs is VARIABLE_SIZED
	uint32_t size;

	// total elements in the tuple
	uint16_t element_count;

	// definition of all of the elements
	element_def element_defs[];
};

#define size_of_tuple_def(element_count) (sizeof(tuple_def) + ((element_count) * sizeof(element_def)))

// to initialize an empty tuple definition
void init_tuple_def(tuple_def* tuple_d);

// insert the key or values, insert keys in their decreasing order of importance
// mark the tuple_mark_key_complete, once all the keys are inserted
int insert_element_def(tuple_def* tuple_d, element_type ele_type, uint32_t element_size);

// after inserting all the elements call this function
void finalize_tuple_def(tuple_def* tuple_d);

// no function can be called on a tuple definition in a valid way if the tuple is empty
// the functions we are refering to are in the "tuple.h" header file
int is_empty_tuple_def(const tuple_def* tuple_d);

// returns 1, if the element at the given index will be responsible
// for specifing the size of a VARIABLE_SIZED tuple element at (index + 1).
int is_size_specifying_element(const tuple_def* tuple_d, uint16_t index);

// this is the minimum size of any tuple that can be defined by the given tuple definition
// the minimum sized tuple is the one where all variable size elements (as per the tuple definition) are 0 sized
uint32_t get_minimum_tuple_size(const tuple_def* tpl_d);

// to print a final tuple definition
void print_tuple_def(const tuple_def* tuple_d);

#endif

// before any operation initializing a tuple definiton for that tuple is very important
/*
**	tuple_def* tuple_d = acclocate( sizeof(tuple_def) + NO_OF_ELEMENTS * sizeof(element_def) );
**
**	init_tuple_def(tuple_d);
**
**	insert_element_def(tuple_d, ele_type_0, ele_size_0);
**	insert_element_def(tuple_d, ele_type_1, ele_size_1);
**	insert_element_def(tuple_d, ele_type_2, ele_size_2);
**	.
**	. ... NO_OF_ELEMENTS number of successfull inserts 
**
**	finalize_tuple_def(tuple_d);
**
**	if(is_empty_tuple_def(tuple_def))
**		exit(-1);	// panic for the error
**
**	// you may call print_tuple_def or any of other functions from "tuple.h" header file
**
*/