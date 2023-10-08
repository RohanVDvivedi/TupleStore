#ifndef INT_ACCESSES_H
#define INT_ACCESSES_H

#include<stdint.h>
#include<inttypes.h>

#include<cutlery_math.h>

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
	uintX_t get_UINTX_MIN(uint32_t data_size) and
	uintX_t get_UINTX_MAX(uint32_t data_size)
	returns minimum and maximum number that can be represented in data_size number of bytes for an unsigned intteger
*/

uint8_t get_UINT8_MIN(uint32_t data_size);
uint16_t get_UINT16_MIN(uint32_t data_size);
uint32_t get_UINT32_MIN(uint32_t data_size);
uint64_t get_UINT64_MIN(uint32_t data_size);

uint8_t get_UINT8_MAX(uint32_t data_size);
uint16_t get_UINT16_MAX(uint32_t data_size);
uint32_t get_UINT32_MAX(uint32_t data_size);
uint64_t get_UINT64_MAX(uint32_t data_size);

/*
	uintX_t get_INTX_MIN(uint32_t data_size) and
	uintX_t get_INTX_MAX(uint32_t data_size)
	returns minimum and maximum number that can be represented in data_size number of bytes for a signed integer
*/

int8_t get_INT8_MIN(uint32_t data_size);
int16_t get_INT16_MIN(uint32_t data_size);
int32_t get_INT32_MIN(uint32_t data_size);
int64_t get_INT64_MIN(uint32_t data_size);

int8_t get_INT8_MAX(uint32_t data_size);
int16_t get_INT16_MAX(uint32_t data_size);
int32_t get_INT32_MAX(uint32_t data_size);
int64_t get_INT64_MAX(uint32_t data_size);

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

/*
	get MIN and MAX for float and double
*/

float get_FLOAT_MIN();
double get_DOUBLE_MIN();

float get_FLOAT_MAX();
double get_DOUBLE_MAX();

#endif