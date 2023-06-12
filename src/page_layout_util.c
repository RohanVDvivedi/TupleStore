#include<page_layout_util.h>

#include<int_accesses.h>

uint32_t get_value_size_on_page(uint32_t page_size)
{
	if(page_size <= (UINT32_C(1) << 8))
		return 1;
	else if(page_size <= (UINT32_C(1) << 16))
		return 2;
	else if(page_size <= (UINT32_C(1) << 24))
		return 3;
	else
		return 4;
}

uint32_t read_value_from_page(const void* value, uint32_t page_size)
{
	return read_uint32(value, get_value_size_on_page(page_size));
}

void write_value_to_page(void* value, uint32_t page_size, uint32_t to_write)
{
	write_uint32(value, get_value_size_on_page(page_size), to_write);
}