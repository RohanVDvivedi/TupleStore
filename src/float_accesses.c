#include<float_accesses.h>

#include<cutlery_stds.h>

float deserialize_float(const void* data)
{
	float x;
	memory_move(&x, data, sizeof(float));
	return x;
}

double deserialize_double(const void* data)
{
	double x;
	memory_move(&x, data, sizeof(double));
	return x;
}

void serialize_float(void* data, float x)
{
	memory_move(data, &x, sizeof(float));
}

void serialize_double(void* data, double x)
{
	memory_move(data, &x, sizeof(double));
}

float get_FLOAT_MIN()
{
	return -1.0f/0.0f;
}

double get_DOUBLE_MIN()
{
	return -1.0/0.0;
}

float get_FLOAT_MAX()
{
	return 1.0f/0.0f;
}

double get_DOUBLE_MAX()
{
	return 1.0/0.0;
}
