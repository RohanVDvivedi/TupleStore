#ifndef DATA_BYTE_METHODS_H
#define DATA_BYTE_METHODS_H

typedef struct data_type_methods data_type_methods;
struct data_type_methods
{
	// dest = pointer in database page, where the src needs to be stored
	// c_data_type = pointer to the data type's structured representation in c code
	// this function returns 1 on success, and 0 on error
	// on success it writes get_size number of bytes to dest
	int write_to_data_store(void* dest, const void* c_data_type);

	// c_data_type = pointer to the data type's structured representation in c code
	// src = pointer in database page, where the given data type starts
	// this function returns 1 on success, and 0 on error
	// on success it reads get_size number of bytes from src
	int read_from_data_store(void* c_data_type, const void* src);

	// get size of field as represented in serialized form 
	unsigned int get_size(const void* data);

	// 
	int (*compare)(const void* data1, const void* data2);
};

#endif