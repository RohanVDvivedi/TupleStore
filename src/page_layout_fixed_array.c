#include<page_layout_fixed_array.h>

#include<stdint.h>

#include<bitmap.h>

#include<page_header.h>
#include<page_layout_util.h>

/*
** tuple capacity for a fixed array page can be pre-calculated
*/
static inline uint32_t get_offset_to_is_valid_bitmap(const void* page, uint32_t page_size);

static inline uint32_t get_tuple_capacity(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return (page_size - get_offset_to_is_valid_bitmap(page, page_size)) / ((tpl_d->size * 8) + 1);
}

/*
** 		offset calculation functions
*/

static inline uint32_t get_offset_to_tuple_count(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_page_header(page, page_size);
}

static inline uint32_t get_offset_to_is_valid_bitmap(const void* page, uint32_t page_size)
{
	return get_offset_to_tuple_count(page, page_size) + get_value_size_on_page(page_size);
}

static inline uint32_t get_offset_to_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return get_offset_to_is_valid_bitmap(page, page_size) + bitmap_size_in_bytes(get_tuple_capacity(page, page_size, tpl_d));
}

static inline uint32_t get_offset_to_ith_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t ith)
{
	return get_offset_to_tuples(page, page_size, tpl_d) + (ith * tpl_d->size);
}

/*
***********************************************************************************************/

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

int init_fixed_array_page(void* page, uint32_t page_size, uint8_t page_header_size, const tuple_def* tpl_d)
{
	// the page must be able to accomodate atleast 1 tuple
	if(page_size < get_minimum_page_size_for_fixed_array_page(page_header_size, tpl_d, 1))
		return 0;

	// initialize page header
	if(init_page_header(page, page_size, page_header_size) == 0)
		return 0;

	// write 0 to tuple_count
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	write_value_to_page(tuple_count, page_size, 0);

	return 1;
}

uint32_t get_tuple_count_fixed_array_page(const void* page, uint32_t page_size)
{
	const void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	return read_value_from_page(tuple_count, page_size);
}