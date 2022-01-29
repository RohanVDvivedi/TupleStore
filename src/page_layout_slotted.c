#include<page_layout_slotted.h>

#include<stdint.h>

#include<tuple.h>

#include<page_header.h>
#include<page_layout_util.h>

/*
** 		offset calculation functions
*/

static inline uint32_t get_offset_to_tuple_count(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_page_header(page, page_size);
}

static inline uint32_t get_offset_to_end_of_free_space_offset(const void* page, uint32_t page_size)
{
	return get_offset_to_tuple_count(page, page_size) + get_value_size_on_page(page_size);
}

static inline uint32_t get_offset_to_tuple_offsets(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_free_space_offset(page, page_size) + get_value_size_on_page(page_size);
}

static inline uint32_t get_offset_to_ith_tuple_offset(const void* page, uint32_t page_size, uint32_t ith)
{
	return get_offset_to_tuple_offsets(page, page_size) + (ith * get_value_size_on_page(page_size));
}

static inline uint32_t get_offset_to_start_of_free_space(const void* page, uint32_t page_size)
{
	const void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);
	return get_offset_to_ith_tuple_offset(page, page_size, tuple_count_val);
}

static inline uint32_t get_offset_to_end_of_free_space(const void* page, uint32_t page_size)
{
	const void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);

	const void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	uint32_t end_of_free_space_offset_val = read_value_from_page(end_of_free_space_offset, page_size);

	// if the tuple_count == 0, the free space end with the end of page
	return (tuple_count_val == 0) ? page_size : end_of_free_space_offset_val;
}

static inline uint32_t get_offset_to_ith_tuple(const void* page, uint32_t page_size, uint32_t ith)
{
	const void* ith_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, ith);
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

uint32_t get_tuple_count_slotted_page(const void* page, uint32_t page_size)
{
	const void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	return read_value_from_page(tuple_count, page_size);
}

int can_insert_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	// tuple needs space for itself and its offset
	uint32_t size_required_for_new_tuple = get_value_size_on_page(page_size) + get_tuple_size(tpl_d, external_tuple);

	return size_required_for_new_tuple <= get_free_space_slotted_page(page, page_size);
}

int delete_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	void* ith_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, index);
	uint32_t ith_tuple_offset_val = read_value_from_page(ith_tuple_offset, page_size);

	// tuple offset at ith index is 0, indicating it does not exists
	if(ith_tuple_offset_val == 0)
		return 0;

	// set the tuple offset of the tuple to be deleted to 0, i.e. mark deleted
	write_value_to_page(ith_tuple_offset, page_size, 0);

	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	uint32_t end_of_free_space_offset_val = get_offset_to_end_of_free_space(page, page_size);

	// if the tuple to be removed is at the end of free space
	// we need to find the min of the existing tuple offsets
	if(end_of_free_space_offset_val == ith_tuple_offset_val)
	{
		uint32_t new_end_of_free_space_offset = page_size;
		for(uint32_t i = 0; i < get_tuple_count_slotted_page(page, page_size); i++)
		{
			#define min(a,b) (((a)<(b))?(a):(b))
			new_end_of_free_space_offset = min(get_offset_to_ith_tuple(page, page_size, i), new_end_of_free_space_offset);
		}
		write_value_to_page(end_of_free_space_offset, page_size, new_end_of_free_space_offset);
	}

	// retract tuple count if possible
	if(index == get_tuple_count_slotted_page(page, page_size))
	{
		void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
		while(1)
		{
			if(get_offset_to_ith_tuple(page, page_size, index) == 0)
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

int delete_all_tuples_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	// write 0 to tuple_count
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	write_value_to_page(tuple_count, page_size, 0);

	// write page_size to end_of_free_space_offset
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, page_size);

	return 1;
}

int exists_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint16_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	return get_offset_to_ith_tuple(page, page_size, index) != 0;
}

uint32_t get_free_space_slotted_page(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_free_space(page, page_size) - get_offset_to_start_of_free_space(page, page_size);
}