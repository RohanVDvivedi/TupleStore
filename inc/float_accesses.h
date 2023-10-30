#ifndef INT_ACCESSES_H
#define INT_ACCESSES_H

#include<stdint.h>

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

float deserialize_float(const void* data);
double deserialize_double(const void* data);

/*
	below functions allow you to read and write floats and doubles independent of alignement issues
	they store data in the processor order only
*/

void serialize_float(void* data, float x);
void serialize_double(void* data, double x);

/*
	get MIN and MAX for float and double
*/

float get_FLOAT_MIN();
double get_DOUBLE_MIN();

float get_FLOAT_MAX();
double get_DOUBLE_MAX();

#endif