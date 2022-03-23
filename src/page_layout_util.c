#include<page_layout_util.h>

uint32_t get_value_size_on_page(uint32_t page_size)
{
	if(page_size <= (1 << 8))
		return 1;
	else if(page_size <= (1 << 16))
		return 2;
	else
		return 4;
}

uint32_t read_value_from(const void* value, uint32_t bytes_to_read)
{
	switch(bytes_to_read)
	{
		case 1 :
			return *((const uint8_t*)value);
		case 2 :
			return *((const uint16_t*)value);
		case 4 :
		default :
			return *((const uint32_t*)value);
	}
}

uint32_t read_value_from_page(const void* value, uint32_t page_size)
{
	return read_value_from(value, get_value_size_on_page(page_size));
}

void write_value_to(void* value, uint32_t bytes_to_read, uint32_t to_write)
{
	switch(bytes_to_read)
	{
		case 1 :
			{*((uint8_t*)value) = to_write;	return;}
		case 2 :
			{*((uint16_t*)value) = to_write; return;}
		case 4 :
		default :
			{*((uint32_t*)value) = to_write; return;}
	}
}

void write_value_to_page(void* value, uint32_t page_size, uint32_t to_write)
{
	write_value_to(value, get_value_size_on_page(page_size), to_write);
}