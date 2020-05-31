#include<cell_definition.h>

uint64_t get_unsigned_int_value(void* position, cell_definition* cell_def)
{
	if(cell_def->type == UNSIGNED_INT)
	{
		if(cell_def->size_in_bytes == 1)
		{
			return (uint64_t)(*((uint8_t*)position));
		}
		else if(cell_def->size_in_bytes == 2)
		{
			return (uint64_t)(*((uint16_t*)position));
		}
		else if(cell_def->size_in_bytes == 4)
		{
			return (uint64_t)(*((uint32_t*)position));
		}
		else if(cell_def->size_in_bytes == 8)
		{
			return (uint64_t)(*((uint64_t*)position));
		}
	}
	return 0;
}

int64_t get_signed_int_value(void* position, cell_definition* cell_def)
{
	if(cell_def->type == SIGNED_INT)
	{
		if(cell_def->size_in_bytes == 1)
		{
			return (int64_t)(*((int8_t*)position));
		}
		else if(cell_def->size_in_bytes == 2)
		{
			return (int64_t)(*((int16_t*)position));
		}
		else if(cell_def->size_in_bytes == 4)
		{
			return (int64_t)(*((int32_t*)position));
		}
		else if(cell_def->size_in_bytes == 8)
		{
			return (int64_t)(*((int64_t*)position));
		}
	}
	return 0;
}

double get_floating_point_value(void* position, cell_definition* cell_def)
{
	if(cell_def->type == FLOATING_NUM)
	{
		if(cell_def->size_in_bytes == 4)
		{
			return (double)(*((float*)position));
		}
		else if(cell_def->size_in_bytes == 8)
		{
			return (double)(*((double*)position));
		}
	}
	return 0;
}