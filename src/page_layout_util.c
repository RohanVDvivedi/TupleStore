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

uint32_t read_value_from_page(void* value, uint32_t page_size)
{
	switch(get_value_size_on_page(page_size))
	{
		case 1 :
			return *((uint8_t*)value);
		case 2 :
			return *((uint16_t*)value);
		case 4 :
		default :
			return *((uint32_t*)value);
	}
}