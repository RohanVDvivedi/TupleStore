#ifndef INT_ACCESSES_H
#define INT_ACCESSES_H

#include<stdint.h>
#include<inttypes.h>

// utility macros to be used only for numeric types
#define compare(a,b)	( ((a)>(b)) ? 1 : (((a)<(b)) ? (-1) : 0 ) )
#define min(a,b)        (((a)<(b))?(a):(b))
#define max(a,b)        (((a)>(b))?(a):(b))

/*
	all the functions assume that data is stored at data in little endian format
	it returns data in the byte ordering of the processor

	the functions declared in this header file allow you to perform
	endianness and alignment independent accesses to data stored on the page

	this functions also allow you to have and operate with integers with varibale length byte widths
	all the way from 1 to 8 

	alignment is mostly a problem on systems that fault on an unaligned accesses,
	it is always better
*/

/*
	for a function operating on uintX_t or intX_t if data_size == 0
	then the operation is a NO-OP (NOP)
	if the data_size > X/8 then data_size is taken as X/8
	if data_size is between 1 and X/8 then only the specified bytes are read or written to form the number
*/

/*
	read_uintX reads min(X/8, data_size) bytes from data as an uintX_t
*/

uint8_t read_uint8(const void* data, uint32_t data_size);
uint16_t read_uint16(const void* data, uint32_t data_size);
uint32_t read_uint32(const void* data, uint32_t data_size);
uint64_t read_uint64(const void* data, uint32_t data_size);

/*
	read_intX reads min(X/8, data_size) bytes from data as an intX_t
*/

int8_t read_int8(const void* data, uint32_t data_size);
int16_t read_int16(const void* data, uint32_t data_size);
int32_t read_int32(const void* data, uint32_t data_size);
int64_t read_int64(const void* data, uint32_t data_size);

/*
	write_uintX writes min(X/8, data_size) bytes to data
*/

void write_uint8(void* data, uint32_t data_size, uint8_t x);
void write_uint16(void* data, uint32_t data_size, uint16_t x);
void write_uint32(void* data, uint32_t data_size, uint32_t x);
void write_uint64(void* data, uint32_t data_size, uint64_t x);

/*
	write_intX writes min(X/8, data_size) bytes to data
*/

void write_int8(void* data, uint32_t data_size, int8_t x);
void write_int16(void* data, uint32_t data_size, int16_t x);
void write_int32(void* data, uint32_t data_size, int32_t x);
void write_int64(void* data, uint32_t data_size, int64_t x);

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

float read_float(const void* data);
double read_double(const void* data);

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

void write_float(void* data, float x);
void write_double(void* data, double x);

#endif