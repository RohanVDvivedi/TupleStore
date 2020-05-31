#ifndef TUPLE_DEFINITION_H
#define TUPLE_DEFINITION_H

#include<cell_definition.h>

typedef struct tuple_definition tuple_definition;
struct tuple_definition
{
	// uncompressed size of each row having this tuple definition
	// size_in_bytes is equal to the total size of all the data_definitions in the tuple
	unsigned int size_in_bytes;

	// the definition of each cell in the tuple can be distinctive
	cell_definition data_definitions[];
};

#endif