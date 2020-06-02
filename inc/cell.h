#ifndef CELL_H
#define CELL_H

#include<datatype.h>

typedef union cell cell;
union cell
{
	void* 	  GENERIC;
	
	char*     CHAR_STRING;

	int8_t*   SIGNED_INT_1;
	int16_t*  SIGNED_INT_2;
	int32_t*  SIGNED_INT_4;
	int64_t*  SIGNED_INT_8;

	uint8_t*  UNSIGNED_INT_1;
	uint16_t* UNSIGNED_INT_2;
	uint32_t* UNSIGNED_INT_4;
	uint64_t* UNSIGNED_INT_8;

	float*    FLOATING_NUM_4;
	double*   FLOATING_NUM_8;
};

#endif