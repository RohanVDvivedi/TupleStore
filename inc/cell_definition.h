#ifndef CELL_DEFINITION_H
#define CELL_DEFINITION_H

#include<datatype.h>

typedef struct cell_definition cell_definition;
struct cell_definition
{
	// offset in tuple
	unsigned int byte_offset_in_tuple;

	// size in bytes that is occupied by the data cell with this cell definition
	unsigned int size_in_bytes;

	// datatype stored in the cell
	datatype type;
};

uint64_t get_unsigned_int_value(void* position, cell_definition* cell_def);

int64_t get_signed_int_value(void* position, cell_definition* cell_def);

double get_floating_point_value(void* position, cell_definition* cell_def);

#endif