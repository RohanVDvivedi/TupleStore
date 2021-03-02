#ifndef TUPLE_DEFINITION_H
#define TUPLE_DEFINITION_H

#include<stdint.h>
#include<data_type.h>

typedef struct tuple_definition tuple_definition;
struct tuple_definition
{
	// number of columns in this tuple
	uint64_t column_count;

	// this value is true if all the datatypes of the column are of fixed size
	int is_fixed_sized;
	// if the tuple is_fixed_sized == 1, then fixed_size is the size of the tuple as in datastore
	uint64_t fixed_size;

	// list of datatypes of each column in order of their occurence in the tuple
	data_type data_type_of_column[];
};

const tuple_definition* get_tuple_definition(uint64_t column_count, int is_fixed_sized, data_type column_types[]);

#endif