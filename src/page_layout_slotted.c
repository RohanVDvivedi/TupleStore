#include<page_layout_slotted.h>

uint32_t get_minimum_page_size_for_slotted_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count)
{
	uint32_t min_size_8 = sizeof(uint8_t) + page_header_size + ((2 + tuple_count) * sizeof(uint8_t)) + (tuple_count * get_minimum_tuple_size(tpl_d));
	if(min_size_8 <= (1<<8))
		return min_size_8;

	uint32_t min_size_16 = sizeof(uint16_t) + page_header_size + ((2 + tuple_count) * sizeof(uint16_t)) + (tuple_count * get_minimum_tuple_size(tpl_d));
	if(min_size_16 <= (1<<16))
		return min_size_16;

	uint32_t min_size_32 = sizeof(uint32_t) + page_header_size + ((2 + tuple_count) * sizeof(uint32_t)) + (tuple_count * get_minimum_tuple_size(tpl_d));
	return min_size_32;
}