#include<page_layout_fixed_array.h>

#include<stdint.h>

#include<bitmap.h>

#include<tuple.h>

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

static inline uint32_t get_offset_to_start_of_free_space(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	const void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);

	return get_offset_to_ith_tuple(page, page_size, tpl_d, tuple_count_val);
}

static inline uint32_t get_offset_to_end_of_free_space(uint32_t page_size)
{
	return page_size;
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

int init_fixed_array_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d)
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

int can_insert_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return get_tuple_count_fixed_array_page(page, page_size) < get_tuple_capacity(page, page_size, tpl_d);
}

int delete_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_fixed_array_page(page, page_size))
		return 0;

	char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);

	// indexed tuple does not exist, so can not delete it
	if(get_bit(is_valid, index) == 0)
		return 0;

	// mark deleted
	reset_bit(is_valid, index);

	// retract tuple_count if possible
	if(index == get_tuple_count_fixed_array_page(page, page_size))
	{
		void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
		while(1)
		{
			if(get_bit(is_valid, index) == 0)
				write_value_to_page(tuple_count, page_size, index);
			else
				break;

			if(index == 0)
				break;
			index--;
		}
	}

	return 1;
}

int delete_all_tuples_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	// just make tuple count as 0
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	write_value_to_page(tuple_count, page_size, 0);
	return 1;
}

int exists_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_fixed_array_page(page, page_size))
		return 0;

	const char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);

	return get_bit(is_valid, index) != 0;
}

const void* get_nth_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_fixed_array_page(page, page_size))
		return NULL;

	const char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);

	// indexed tuple does not exist
	if(get_bit(is_valid, index) == 0)
		return NULL;

	return page + get_offset_to_ith_tuple(page, page_size, tpl_d, index);
}

uint32_t get_free_space_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return get_offset_to_end_of_free_space(page_size) - get_offset_to_start_of_free_space(page, page_size, tpl_d);
}