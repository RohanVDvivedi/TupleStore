#include<page_layout_slotted.h>

#include<stdint.h>

#include<page_header.h>
#include<page_layout_util.h>

/*
** 		offset calculation functions
*/

static inline uint32_t get_offset_to_tuple_count(void* page, uint32_t page_size)
{
	return get_offset_to_end_of_page_header(page, page_size);
}

static inline uint32_t get_offset_to_end_of_free_space_offset(void* page, uint32_t page_size)
{
	return get_offset_to_tuple_count(page, page_size) + get_value_size_on_page(page_size);
}

static inline uint32_t get_offset_to_tuple_offsets(void* page, uint32_t page_size)
{
	return get_offset_to_end_of_free_space_offset(page, page_size) + get_value_size_on_page(page_size);
}

static inline uint32_t get_offset_to_ith_tuple_offset(void* page, uint32_t page_size, uint32_t ith)
{
	return get_offset_to_tuple_offsets(page, page_size) + (ith * get_value_size_on_page(page_size));
}

static inline uint32_t get_offset_to_start_of_free_space(void* page, uint32_t page_size)
{
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);
	return get_offset_to_ith_tuple_offset(page, page_size, tuple_count_val);
}

static inline uint32_t get_offset_to_end_of_free_space(void* page, uint32_t page_size)
{
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);

	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	uint32_t end_of_free_space_offset_val = read_value_from_page(end_of_free_space_offset, page_size);

	// if the tuple_count == 0, the free space end with the end of page
	return (tuple_count_val == 0) ? page_size : end_of_free_space_offset_val;
}

static inline uint32_t get_offset_to_ith_tuple(void* page, uint32_t page_size, uint32_t ith)
{
	void* ith_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, ith);
	return read_value_from_page(ith_tuple_offset, page_size);
}

/*
***********************************************************************************************/

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

int init_slotted_page(void* page, uint32_t page_size, uint8_t page_header_size, const tuple_def* tpl_d)
{
	// the page must be able to accomodate atleast 1 tuple
	if(page_size < get_minimum_page_size_for_slotted_page(page_header_size, tpl_d, 1))
		return 0;

	// initialize page header
	if(init_page_header(page, page_size, page_header_size) == 0)
		return 0;

	// write 0 to tuple_count
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	write_value_to_page(tuple_count, page_size, 0);

	// write page_size to end_of_free_space_offset
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, page_size);

	return 1;
}