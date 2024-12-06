#include<page_layout_util.h>

#include<serial_int.h>

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
	// This function is very frequently called inside the TupleStore, hence an attempt to make it jump free
	// using the line below, but it turns out it makes not difference in performance numbers, hence opting to leave a more readable code in the source code
	//return 4 - (page_size <= (UINT32_C(1) << 24)) - (page_size <= (UINT32_C(1) << 16)) - (page_size <= (UINT32_C(1) << 8));
}

uint32_t read_value_from_page(const void* value, uint32_t page_size)
{
	return deserialize_uint32(value, get_value_size_on_page(page_size));
}

void write_value_to_page(void* value, uint32_t page_size, uint32_t to_write)
{
	serialize_uint32(value, get_value_size_on_page(page_size), to_write);
}