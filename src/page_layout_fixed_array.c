#include<page_layout_fixed_array.h>

uint32_t get_minimum_page_size_for_fixed_array_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count)
{
	uint32_t min_size_8 = sizeof(uint8_t) + page_header_size + sizeof(uint8_t) + ((tuple_count + 7) / 8) + (tuple_count * tpl_d->size);
	if(min_size_8 <= (1<<8))
		return min_size_8;

	uint32_t min_size_16 = sizeof(uint16_t) + page_header_size + sizeof(uint16_t) + ((tuple_count + 7) / 8) + (tuple_count * tpl_d->size);
	if(min_size_16 <= (1<<16))
		return min_size_16;

	uint32_t min_size_32 = sizeof(uint32_t) + page_header_size + sizeof(uint32_t) + ((tuple_count + 7) / 8) + (tuple_count * tpl_d->size);
	return min_size_32;
}