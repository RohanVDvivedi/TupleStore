#ifndef FLOAT_ACCESSES_H
#define FLOAT_ACCESSES_H

#include<stdint.h>

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

static inline float deserialize_float(const void* data);
static inline double deserialize_double(const void* data);

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

static inline void serialize_float(void* data, float x);
static inline void serialize_double(void* data, double x);

/*
	get MIN and MAX for float and double
*/

static inline float get_FLOAT_MIN();
static inline double get_DOUBLE_MIN();

static inline float get_FLOAT_MAX();
static inline double get_DOUBLE_MAX();

// static inline implementation

#include<cutlery/cutlery_stds.h>

// assumptions are that float must be 4 bytes and double must be 8 bytes
fail_build_on(sizeof(float) != sizeof(uint32_t));
fail_build_on(sizeof(double) != sizeof(uint64_t));

static inline float deserialize_float(const void* data)
{
	// little-endian on-disk format
	uint32_t temp = deserialize_uint32(data, sizeof(float));

	float x;
	memory_move(&x, &temp, sizeof(float));
	return x;
}

static inline double deserialize_double(const void* data)
{
	// little-endian on-disk format
	uint64_t temp = deserialize_uint64(data, sizeof(double));

	double x;
	memory_move(&x, &temp, sizeof(double));
	return x;
}

static inline void serialize_float(void* data, float x)
{
	uint32_t temp;
	memory_move(&temp, &x, sizeof(float));

	// little-endian on-disk format
	serialize_uint32(data, sizeof(float), temp);
}

static inline void serialize_double(void* data, double x)
{
	uint64_t temp;
	memory_move(&temp, &x, sizeof(double));

	// little-endian on-disk format
	serialize_uint64(data, sizeof(double), temp);
}

static inline float get_FLOAT_MIN()
{
	return -INFINITY;
}

static inline double get_DOUBLE_MIN()
{
	return -INFINITY;
}

static inline float get_FLOAT_MAX()
{
	return INFINITY;
}

static inline double get_DOUBLE_MAX()
{
	return INFINITY;
}

#endif