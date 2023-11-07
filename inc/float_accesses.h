#ifndef FLOAT_ACCESSES_H
#define FLOAT_ACCESSES_H

#include<stdint.h>

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

float deserialize_float(const void* data);
double deserialize_double(const void* data);
long double deserialize_long_double(const void* data);

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

void serialize_float(void* data, float x);
void serialize_double(void* data, double x);
void serialize_long_double(void* data, long double x);

/*
	get MIN and MAX for float and double
*/

float get_FLOAT_MIN();
double get_DOUBLE_MIN();
long double get_LONG_DOUBLE_MIN();

float get_FLOAT_MAX();
double get_DOUBLE_MAX();
long double get_LONG_DOUBLE_MAX();

#endif