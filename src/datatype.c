#include<datatype.h>

int is_size_allowed(datatype type, unsigned int size)
{
	switch(type)
	{
		case CHAR_STRING :
			return (size <= CHAR_STRING_max_length);
		case SIGNED_INT :
		case UNSIGNED_INT :
			return (size == 1) || (size == 2) || (size == 4) || (size == 8);
		case FLOATING_NUM :
			return (size == 4) || (size == 8);
		default :
			return 0;
	}
}

int compare_signed_unsigned(int64_t a, uint64_t b)
{
	if(a < 0)
		return -1;
	else
	{
		uint64_t ua = a;
		if(ua > b)
			return 1;
		else if(ua < b)
			return -1;
	}
	return 0;
}

int compare_signed_signed(int64_t a, int64_t b)
{
	if(a > b)
		return 1;
	else if(a < b)
		return -1;
	return 0;
}

int compare_unsigned_unsigned(uint64_t a, uint64_t b)
{
	if(a > b)
		return 1;
	else if(a < b)
		return -1;
	return 0;
}

int compare_float_float(double a, double b)
{
	if(a > b)
		return 1;
	else if(a < b)
		return -1;
	return 0;
}

