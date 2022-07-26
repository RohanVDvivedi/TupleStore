#ifndef TUPLE_DEF_H
#define TUPLE_DEF_H

#include<stdio.h>
#include<stdint.h>

#include<user_value.h>

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

typedef struct element_def element_def;
struct element_def
{
	// name of this attribute i.e. name of column
	char name[64];

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

	union
	{
		// used for type == UINT, INT, FLOAT, STRING and BLOB
		// defined as byte offset of the element in the tuple
		uint32_t byte_offset;

		// used for type = VAR_STRING and VAR_BLOB
		// defined as byte offset to the byte offset of this element in the tuple
		uint32_t byte_offset_to_byte_offset;
	};

	// set if the element is non NULL-able
	// i.e. if this bit is set, the element can not be a NULL
	int is_non_NULLable;

	// valid only for NULLable fixed_sized_element_def
	uint32_t is_NULL_bitmap_bit_offset;

	// if the element_def is non NULLable then this is the default vaue it must be set to
	// default_value of is_NON_NULLable element can not be (user_value){.is_NULL = 1}
	user_value default_value;
};

// initialize an element's definition using its type and size
// it may fail if the size parameters is not valid for a given data type
int init_element_def(element_def* element_d, const char* name, element_type ele_type, uint32_t size_OR_prefix_size, int is_non_NULLable, const user_value* default_value);

// returns true if an element is of a variable sized
int is_variable_sized_element_def(const element_def* element_d);

// returns true if an element is of a fixed sized
int is_fixed_sized_element_def(const element_def* element_d);

// returns 1, if the element_def can be set to a NULL
int is_NULLable_element_def(const element_def* element_d);

// returns 1, if this element_def will need/(or has) a bit in the is_NULL bitmap
int has_bit_in_is_NULL_bitmap(const element_def* element_d);

// returns size of element
uint32_t get_element_size(const void* e, const element_def* ele_d);

// returns true if the two different element defs can be compared against one another
int can_compare_element_defs(const element_def* ele_d_1, const element_def* ele_d_2);

// returns true if an element_defs are compatible to be set from each other
// ideally if you can compare them, then you must be allowed to set one from another
int can_set_from_element_defs(const element_def* ele_d_1, const element_def* ele_d_2);

// compare 2 elements, given their element definitions
// to use this function appropriately can_compare_element_defs on the corresponding element_defs must return true(1)
int compare_elements(const void* e1, const element_def* ele_d_1, const void* e2, const element_def* ele_d_2);

// hash element, given their element definition
uint32_t hash_element(const void* e, const element_def* ele_d, uint32_t (*hash_func)(const void* data, uint32_t size));

typedef struct tuple_def tuple_def;
struct tuple_def
{
	// name of the tuple type i.e. table/index name
	char name[64];

	// set to 1 for a variable sized tuple_def
	// The tuple is VARIABLE_SIZED, if atleast one of the element_defs is VARIABLE_SIZED
	int is_variable_sized;

	// min total size of tuple in bytes
	// i.e. size of tuple when all variable sized elements are NULL
	// this will be same as tuple's size for a fixed sized tuple
	union
	{
		// defined for variable sized tuple
		uint32_t min_size;

		// defined for fixed sized tuple
		uint32_t size;
	};

	// at this offset there is a bitmap of element_count number of bits
	// if the corresponding bit is set then the value at that location is NULL
	// for fixed length tuple this offset is 0
	// for variale length tuple this offset is equal to the bytes required to store the tuple size
	union
	{
		uint32_t byte_offset_to_is_null_bitmap;

		uint32_t size_of_byte_offsets;
	};

	// number of bits in is_NULL_bitmap
	// to reiterate only NULLable fixed_sized_elements need a bit in is_NULL bitmap
	uint32_t is_NULL_bitmap_size_in_bits;

	// total elements in the tuple
	uint32_t element_count;

	// definition of all of the elements
	element_def element_defs[];
};

#define size_of_tuple_def(element_count) (sizeof(tuple_def) + ((element_count) * sizeof(element_def)))

// to initialize an empty tuple definition
int init_tuple_def(tuple_def* tuple_d, const char* name);

// insert an element definition in this tuple definition
// returns 1 on success
int insert_element_def(tuple_def* tuple_d, const char* name, element_type ele_type, uint32_t element_size_OR_prefix_size, int is_non_NULLable, const user_value* default_value);

// insert an element definition in this tuple definition
// such that it copies its type and size from "element_def_id"-th element definiton of "tuple_d_copy_from"
// if the name parameter is NULL then we also copy the name from the corresponding element definition
// returns 1 on success
int insert_copy_of_element_def(tuple_def* tuple_d, const char* name, const tuple_def* tuple_d_copy_from, uint32_t element_def_id);

// after inserting all the elements call this function
// here the parameter max_tuple_size is not required for fixed length tuples
// it is used to calculate the size (in bytes) required for storing tuple_size and offsets to varable sized elements
void finalize_tuple_def(tuple_def* tuple_d, uint32_t max_tuple_size);

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

// gets index to an already existing element def in the given tuple def by its name
// if not found it returns ELEMENT_DEF_NOT_FOUND
#define ELEMENT_DEF_NOT_FOUND (~((uint32_t)0))
uint32_t get_element_def_id_by_name(const tuple_def* tuple_d, const char* name);

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