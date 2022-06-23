#include<int_accesses.h>

#include<string.h>
#include<limits.h>

#define uint(X) uint ## X ## _t

#define READ_UINT(X)						\
{											\
	if(data_size == 0)						\
		return 0;							\
	if(data_size > sizeof(uint(X)))			\
		data_size = sizeof(uint(X));		\
	uint(X) x = 0;							\
	const uint8_t* data8 = data;			\
	for(uint32_t i = 0; i < data_size; i++)	\
	{										\
		uint(X) temp = data8[i];				\
		x |= (temp << (i * CHAR_BIT));		\
	}										\
	return x;								\
}

uint8_t read_uint8(const void* data, uint32_t data_size)	READ_UINT(8)
uint16_t read_uint16(const void* data, uint32_t data_size)	READ_UINT(16)
uint32_t read_uint32(const void* data, uint32_t data_size)	READ_UINT(32)
uint64_t read_uint64(const void* data, uint32_t data_size)	READ_UINT(64)

int8_t read_int8(const void* data, uint32_t data_size);
int16_t read_int16(const void* data, uint32_t data_size);
int32_t read_int32(const void* data, uint32_t data_size);
int64_t read_int64(const void* data, uint32_t data_size);

void write_uint8(void* data, uint32_t data_size, uint8_t x);
void write_uint16(void* data, uint32_t data_size, uint16_t x);
void write_uint32(void* data, uint32_t data_size, uint32_t x);
void write_uint64(void* data, uint32_t data_size, uint64_t x);

void write_int8(void* data, uint32_t data_size, int8_t x);
void write_int16(void* data, uint32_t data_size, int16_t x);
void write_int32(void* data, uint32_t data_size, int32_t x);
void write_int64(void* data, uint32_t data_size, int64_t x);

float read_float(const void* data)
{
	float x;
	memcpy(&x, data, sizeof(float));
	return x;
}

double read_double(const void* data)
{
	double x;
	memcpy(&x, data, sizeof(double));
	return x;
}

void write_float(void* data, float x)
{
	memcpy(data, &x, sizeof(float));
}

void write_double(void* data, double x)
{
	memcpy(data, &x, sizeof(double));
}
