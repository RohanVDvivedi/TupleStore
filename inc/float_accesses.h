#ifndef FLOAT_ACCESSES_H
#define FLOAT_ACCESSES_H

#include<stdint.h>

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

static inline float deserialize_float(const void* data);
static inline double deserialize_double(const void* data);
static inline long double deserialize_long_double(const void* data);

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

static inline void serialize_float(void* data, float x);
static inline void serialize_double(void* data, double x);
static inline void serialize_long_double(void* data, long double x);

/*
	get MIN and MAX for float and double
*/

static inline float get_FLOAT_MIN();
static inline double get_DOUBLE_MIN();
static inline long double get_LONG_DOUBLE_MIN();

static inline float get_FLOAT_MAX();
static inline double get_DOUBLE_MAX();
static inline long double get_LONG_DOUBLE_MAX();

// static inline implementation

#include<cutlery_stds.h>

static inline float deserialize_float(const void* data)
{
	float x;
	memory_move(&x, data, sizeof(float));
	return x;
}

static inline double deserialize_double(const void* data)
{
	double x;
	memory_move(&x, data, sizeof(double));
	return x;
}

static inline long double deserialize_long_double(const void* data)
{
	long double x;
	memory_move(&x, data, sizeof(long double));
	return x;
}

static inline void serialize_float(void* data, float x)
{
	memory_move(data, &x, sizeof(float));
}

static inline void serialize_double(void* data, double x)
{
	memory_move(data, &x, sizeof(double));
}

static inline void serialize_long_double(void* data, long double x)
{
	memory_move(data, &x, sizeof(long double));
}

static inline float get_FLOAT_MIN()
{
	return -1.0f/0.0f;
}

static inline double get_DOUBLE_MIN()
{
	return -1.0/0.0;
}

static inline long double get_LONG_DOUBLE_MIN()
{
	return -1.0l/0.0l;
}

static inline float get_FLOAT_MAX()
{
	return 1.0f/0.0f;
}

static inline double get_DOUBLE_MAX()
{
	return 1.0/0.0;
}

static inline long double get_LONG_DOUBLE_MAX()
{
	return 1.0l/0.0l;
}

#endif