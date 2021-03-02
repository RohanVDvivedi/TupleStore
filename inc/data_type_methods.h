#ifndef DATA_BYTE_METHODS_H
#define DATA_BYTE_METHODS_H

#include<stdint.h>
#include<data_type.h>

typedef struct data_type_methods data_type_methods;
struct data_type_methods
{
	// dest = pointer inside database store, where the src needs to be stored
	// c_data_type = pointer to the data type's structured representation in c code
	// this function returns 1 on success, and 0 on error
	// on success it writes get_size number of bytes to dest
	int (*write_to_data_store)(void* dest, const void* c_data_type);

	// c_data_type = pointer to the data type's structured representation in c code
	// src = pointer inside database store, where the given data type starts
	// this function returns 1 on success, and 0 on error
	// on success it reads get_size number of bytes from src
	int (*read_from_data_store)(void* c_data_type, const void* src);

	// get size of field as represented in serialized form inside the database store
	// special cases for composite datatypes
	// NULL for BLOB datatype
	// strlen() for STR datatype
	uint64_t (*get_size)(const void* data);

	// data1 and data2 are both pointers inside database store, where given datatypes are stored
	// data1 and data2 both point to the same datatype
	// return (*data1)- (*data2) or strncmp for strings or memcmp for byte
	int (*compare)(const void* data1, const void* data2);

	// a hashmap of type specific methods 
	// add for numbers, strcat for strings, etc
	// hashmap <string(function_name) -> function_pointers>
};

extern const data_type_methods methods_for_data_type[DATA_TYPES_COUNT];

void init_methods_of_data_type();

#endif