#include<int_accesses.h>

#include<cutlery_stds.h>
#include<limits.h>

#define uint(X) uint ## X ## _t
#define sint(X) int ## X ## _t

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
		uint(X) temp = data8[i];			\
		x |= (temp << (i * CHAR_BIT));		\
	}										\
	return x;								\
}

uint8_t read_uint8(const void* data, uint32_t data_size)	READ_UINT(8)
uint16_t read_uint16(const void* data, uint32_t data_size)	READ_UINT(16)
uint32_t read_uint32(const void* data, uint32_t data_size)	READ_UINT(32)
uint64_t read_uint64(const void* data, uint32_t data_size)	READ_UINT(64)

#define READ_INT(X)											\
{															\
	if(data_size == 0)										\
		return 0;											\
	if(data_size > sizeof(sint(X)))							\
		data_size = sizeof(sint(X));						\
	sint(X) x = 0;											\
	const uint8_t* data8 = data;							\
	for(uint32_t i = 0; i < data_size; i++)					\
	{														\
		uint(X) temp = data8[i];							\
		x |= (temp << (i * CHAR_BIT));						\
	}														\
	int is_neg = data8[data_size - 1] & ((uint8_t)0x80);	\
	if(!is_neg)												\
		return x;											\
	for(uint32_t i = data_size; i < sizeof(sint(X)); i++)	\
		x |= (UINT ## X ## _C(0xff) << (i * CHAR_BIT));		\
	return x;												\
}

int8_t read_int8(const void* data, uint32_t data_size)		READ_INT(8)
int16_t read_int16(const void* data, uint32_t data_size)	READ_INT(16)
int32_t read_int32(const void* data, uint32_t data_size)	READ_INT(32)
int64_t read_int64(const void* data, uint32_t data_size)	READ_INT(64)

#define WRITE_UINT(X)								\
{													\
	if(data_size == 0)								\
		return;										\
	if(data_size > sizeof(uint(X)))					\
		data_size = sizeof(uint(X));				\
	uint8_t* data8 = data;							\
	for(uint32_t i = 0; i < data_size; i++)			\
	{												\
		uint(X) temp = x & UINT ## X ## _C(0xff);	\
		data8[i] = (uint8_t)temp;					\
		x >>= 8;									\
	}												\
}

void write_uint8(void* data, uint32_t data_size, uint8_t x)		WRITE_UINT(8)
void write_uint16(void* data, uint32_t data_size, uint16_t x)	WRITE_UINT(16)
void write_uint32(void* data, uint32_t data_size, uint32_t x)	WRITE_UINT(32)
void write_uint64(void* data, uint32_t data_size, uint64_t x)	WRITE_UINT(64)

#define WRITE_INT(X)								\
{													\
	if(data_size == 0)								\
		return;										\
	if(data_size > sizeof(sint(X)))					\
		data_size = sizeof(sint(X));				\
	uint8_t* data8 = data;							\
	for(uint32_t i = 0; i < data_size; i++)			\
	{												\
		uint(X) temp = x & UINT ## X ## _C(0xff);	\
		data8[i] = (uint8_t)temp;					\
		x >>= 8;									\
	}												\
}

void write_int8(void* data, uint32_t data_size, int8_t x)		WRITE_INT(8)
void write_int16(void* data, uint32_t data_size, int16_t x)		WRITE_INT(16)
void write_int32(void* data, uint32_t data_size, int32_t x)		WRITE_INT(32)
void write_int64(void* data, uint32_t data_size, int64_t x)		WRITE_INT(64)

uint8_t get_UINT8_MIN(uint32_t data_size)		{return 0;}
uint16_t get_UINT16_MIN(uint32_t data_size)		{return 0;}
uint32_t get_UINT32_MIN(uint32_t data_size)		{return 0;}
uint64_t get_UINT64_MIN(uint32_t data_size)		{return 0;}

#define GET_UINT_MAX(X)											\
{																\
	if(data_size == 0)											\
		return 0;												\
	if(data_size > sizeof(uint(X)))								\
		data_size = sizeof(uint(X));							\
	uint(X) x = INT ## X ## _C(-1);								\
	x = x >> ((sizeof(uint(X)) - data_size) * CHAR_BIT);	\
	return x;													\
}

uint8_t get_UINT8_MAX(uint32_t data_size)		GET_UINT_MAX(8)
uint16_t get_UINT16_MAX(uint32_t data_size)		GET_UINT_MAX(16)
uint32_t get_UINT32_MAX(uint32_t data_size)		GET_UINT_MAX(32)
uint64_t get_UINT64_MAX(uint32_t data_size)		GET_UINT_MAX(64)

#define GET_INT_MIN(X)									\
{														\
	if(data_size == 0)									\
		return 0;										\
	if(data_size > sizeof(sint(X)))						\
		data_size = sizeof(sint(X));					\
	sint(X) x = INT ## X ## _C(-1);						\
	x = x << ((CHAR_BIT * data_size) - 1);				\
	return x;											\
}

int8_t get_INT8_MIN(uint32_t data_size)			GET_INT_MIN(8)
int16_t get_INT16_MIN(uint32_t data_size)		GET_INT_MIN(16)
int32_t get_INT32_MIN(uint32_t data_size)		GET_INT_MIN(32)
int64_t get_INT64_MIN(uint32_t data_size)		GET_INT_MIN(64)

#define GET_INT_MAX(X)											\
{																\
	if(data_size == 0)											\
		return 0;												\
	if(data_size > sizeof(sint(X)))								\
		data_size = sizeof(sint(X));							\
	uint(X) x = INT ## X ## _C(-1);								\
	x = x >> (((sizeof(uint(X)) - data_size) * CHAR_BIT) + 1);	\
	return x;													\
}

int8_t get_INT8_MAX(uint32_t data_size)			GET_INT_MAX(8)
int16_t get_INT16_MAX(uint32_t data_size)		GET_INT_MAX(16)
int32_t get_INT32_MAX(uint32_t data_size)		GET_INT_MAX(32)
int64_t get_INT64_MAX(uint32_t data_size)		GET_INT_MAX(64)

float read_float(const void* data)
{
	float x;
	memory_move(&x, data, sizeof(float));
	return x;
}

double read_double(const void* data)
{
	double x;
	memory_move(&x, data, sizeof(double));
	return x;
}

void write_float(void* data, float x)
{
	memory_move(data, &x, sizeof(float));
}

void write_double(void* data, double x)
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
