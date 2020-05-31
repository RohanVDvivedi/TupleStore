#ifndef CELL_DEFINITION_H
#define CELL_DEFINITION_H

#include<datatype.h>

typedef struct cell_definition cell_definition;
struct cell_definition
{
	// size in bytes that is occupied by the data cell with this cell definition
	unsigned int size_in_bytes;

	// datatype stored in the cell
	datatype type;
};

#endif