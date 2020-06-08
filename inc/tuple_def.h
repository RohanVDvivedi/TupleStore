#ifndef TUPLE_DEF_H
#define TUPLE_DEF_H

#include<stdint.h>

#define CHAR_STRING_max_length 128

typedef enum type type;
enum type
{
	CHAR_STRING 	= 0,
	SIGNED_INT 		= 1,
	UNSIGNED_INT	= 2,
	FLOATING_NUM	= 3
};

// checks if datatype size is allowed
int is_size_allowed(type type, uint16_t size);

int compare_signed_unsigned(int64_t a, uint64_t b);
int compare_signed_signed(int64_t a, int64_t b);
int compare_unsigned_unsigned(uint64_t a, uint64_t b);
int compare_float_float(double a, double b);

typedef struct element_def element_def;
struct element_def
{
	// byte offset in tuple for the given element definition
	uint16_t byte_offset;

	// size in bytes that is occupied by the element
	uint16_t size_in_bytes;

	// type stored in the cell
	type type;
};

typedef union element element;
union element
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

typedef struct tuple_def tuple_def;
struct tuple_def
{
	uint16_t size_in_bytes;

	uint16_t element_count;

	element_def element_defs[];
};

void init_tuple_def(tuple_def* tuple_d);
int insert_col_def(tuple_def* tuple_d, type element_type, uint16_t element_size_in_bytes);

#endif