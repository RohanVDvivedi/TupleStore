#include<data_type_methods.h>

#include<string.h>

void init_methods_of_data_type()
{
	
}

uint64_t get_size_UINT_1_INT_1_SIZE_1(const void* data){return 1;}
uint64_t get_size_UINT_2_INT_2_SIZE_2(const void* data){return 2;}
uint64_t get_size_UINT_4_INT_4_FLOAT_4_SIZE_4(const void* data){return 4;}
uint64_t get_size_UINT_8_INT_8_FLOAT_8_SIZE_8(const void* data){return 8;}
uint64_t get_size_STR(const void* data){return strlen(((char*)data)) + 1;}

int compare_UINT_1_SIZE_1(const void* data1, const void* data2)
{
	uint8_t d1 = *((uint8_t*)data1);
	uint8_t d2 = *((uint8_t*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_UINT_2_SIZE_2(const void* data1, const void* data2)
{
	uint16_t d1 = *((uint16_t*)data1);
	uint16_t d2 = *((uint16_t*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_UINT_4_SIZE_4(const void* data1, const void* data2)
{
	uint32_t d1 = *((uint32_t*)data1);
	uint32_t d2 = *((uint32_t*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_UINT_8_SIZE_8(const void* data1, const void* data2)
{
	uint64_t d1 = *((uint64_t*)data1);
	uint64_t d2 = *((uint64_t*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_INT_1(const void* data1, const void* data2)
{
	int8_t d1 = *((int8_t*)data1);
	int8_t d2 = *((int8_t*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_INT_2(const void* data1, const void* data2)
{
	int16_t d1 = *((int16_t*)data1);
	int16_t d2 = *((int16_t*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_INT_4(const void* data1, const void* data2)
{
	int32_t d1 = *((int32_t*)data1);
	int32_t d2 = *((int32_t*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_INT_8(const void* data1, const void* data2)
{
	int64_t d1 = *((int64_t*)data1);
	int64_t d2 = *((int64_t*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_FLOAT_4(const void* data1, const void* data2)
{
	float d1 = *((float*)data1);
	float d2 = *((float*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}

int compare_FLOAT_8(const void* data1, const void* data2)
{
	double d1 = *((double*)data1);
	double d2 = *((double*)data2);
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else
		return 0;
}