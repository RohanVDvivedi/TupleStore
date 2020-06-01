#ifndef CELL_H
#define CELL_H

#include<datatype.h>

typedef union cell cell;
union cell
{
	char     CHAR_STRING[128];

	int8_t   SIGNED_INT_1;
	int16_t  SIGNED_INT_2;
	int32_t  SIGNED_INT_4;
	int64_t  SIGNED_INT_8;

	uint8_t  UNSIGNED_INT_1;
	uint16_t UNSIGNED_INT_2;
	uint32_t UNSIGNED_INT_4;
	uint64_t UNSIGNED_INT_8;

	float    FLOATING_POINT_4;
	double   FLOATING_POINT_8;
};

#define read(cell_ptr, TYPE ,SIZE) ((cell*)cell_ptr)->TYPE_SIZE

#endif