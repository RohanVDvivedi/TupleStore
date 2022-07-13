#include<memswap.h>

#include<cutlery_stds.h>

//#include<limits.h>

int memswap(void* data1, void* data2, uint32_t size)
{
	if(size <= UINT_MAX)
		return memory_swap(data1, data2, size);
	else
	{
		if(size == 0 || data1 == data2)
			return 0;

		if((data1 <= data2 && data2 < (data1 + size)) || (data2 <= data1 && data1 < (data2 + size)))
			return 0;

		for(uint32_t i = 0; i < size; i++)
		{
			char c = ((char*)data1)[i];
			((char*)data1)[i] = ((char*)data2)[i];
			((char*)data2)[i] = c;
		}

		return 1;
	}
}