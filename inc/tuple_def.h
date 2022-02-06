#ifndef TUPLE_DEF_H
#define TUPLE_DEF_H

#include<stdio.h>
#include<stdint.h>

#include<var_sized.h>

typedef enum element_type element_type;
enum element_type
{
	UINT	   = 0,
	INT 	   = 1,
	FLOAT	   = 2,
	STRING     = 3,
	BLOB       = 4,
	// above data types are fixed type

	VAR_STRING = 5,
	VAR_BLOB   = 6
	// data of VAR_STRING or VAR_BLOB are of variable size
};

extern char type_as_string[][16];

typedef union element element;
union element
{
	uint8_t*        UINT_1;
	uint16_t*       UINT_2;
	uint32_t*       UINT_4;
	uint64_t*       UINT_8;

	int8_t*         INT_1;
	int16_t*        INT_2;
	int32_t*        INT_4;
	int64_t*        INT_8;

	float*          FLOAT_4;
	double*         FLOAT_8;

	char*           STRING;

	void*           BLOB;

	var_string8*    VAR_STRING_1;
	var_string16*   VAR_STRING_2;
	var_string32*   VAR_STRING_4;

	var_blob8*      VAR_BLOB_1;
	var_blob16*     VAR_BLOB_2;
	var_blob32*     VAR_BLOB_4;
};

#define VARIABLE_SIZED 0

typedef struct element_def element_def;
struct element_def
{
	// datatype stored in the element
	element_type type;

	union
	{
		// used for type == UINT, INT, FLOAT, STRING and BLOB
		// size in bytes that may be occupied by the element
		uint32_t size;

		// used for type = VAR_STRING and VAR_BLOB
		// this is the number of bytes used in prefix to specify the actual sizes of VAR_* datatypes
		uint32_t size_specifier_prefix_size;
	};

	// byte offset in tuple for the given element definition
	// valid only if the tuple is not VARIABLE_SIZED
	uint32_t byte_offset;
};

// initialize an element's definition using its type and size
// it may fail if the size parameters is not valid for a given data type
int init_element_def(element_def* element_d, element_type ele_type, uint32_t size_OR_prefix_size);

// returns true if an element is of a variable sized
int is_variable_sized_element_def(const element_def* element_d);

// returns true if an element is of a fixed sized
int is_fixed_sized_element_def(const element_def* element_d);

// returns size of element
uint32_t get_element_size(element e, const element_def* ele_d);

// compare 2 elements, given their element definition
int compare_elements(element e1, element e2, const element_def* ele_d);

// hash element, given their element definition
uint32_t hash_element(element e, const element_def* ele_d, uint32_t (*hash_func)(const void* data, uint32_t size));

typedef struct tuple_def tuple_def;
struct tuple_def
{
	// total size of tuple in bytes
	// size = VARIABLE_SIZED, if atleast one of the element_defs is VARIABLE_SIZED
	uint32_t size;

	// total elements in the tuple
	uint32_t element_count;

	// definition of all of the elements
	element_def element_defs[];
};

#define size_of_tuple_def(element_count) (sizeof(tuple_def) + ((element_count) * sizeof(element_def)))

// to initialize an empty tuple definition
void init_tuple_def(tuple_def* tuple_d);

// insert the key or values, insert keys in their decreasing order of importance
// mark the tuple_mark_key_complete, once all the keys are inserted
int insert_element_def(tuple_def* tuple_d, element_type ele_type, uint32_t element_size_OR_prefix_size);

// after inserting all the elements call this function
void finalize_tuple_def(tuple_def* tuple_d);

// returns 1, if the tuple_d does not contain any elements
int is_empty_tuple_def(const tuple_def* tuple_d);

// returns true if the tuple definition is of a fixed sized
// this includes a tuple definition with 0 element_count
int is_fixed_sized_tuple_def(const tuple_def* tuple_d);

// returns true if the tuple definition is of a variable sized
int is_variable_sized_tuple_def(const tuple_def* tuple_d);

// this is the minimum size of any tuple that can be defined by the given tuple definition
// the minimum sized tuple is the one where all variable size elements (as per the tuple definition) are 0 sized
uint32_t get_minimum_tuple_size(const tuple_def* tuple_d);

// to print a final tuple definition
void print_tuple_def(const tuple_def* tuple_d);

#endif

// before any operation initializing a tuple definiton for that tuple is very important
/*
**	tuple_def* tuple_d = allocate( size_of_tuple_def(NO_OF_ELEMENTS) );
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
**
**	// you may call print_tuple_def or any of other functions from "tuple.h" header file
**
*/