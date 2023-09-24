#include<page_layout_fixed_array.h>

#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#include<bitmap.h>

#include<tuple.h>

#include<page_header.h>
#include<page_layout_util.h>
#include<memswap.h>

/*
** tuple capacity for a fixed array page can be pre-calculated
*/
static inline uint32_t get_offset_to_is_valid_bitmap(const void* page, uint32_t page_size);

static inline uint32_t get_tuple_capacity(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return ((page_size - get_offset_to_is_valid_bitmap(page, page_size)) * 8) / ((tpl_d->size * 8) + 1);
}

/*
** 		offset calculation functions
*/

static inline uint32_t get_offset_to_tuple_count(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_page_header(page, page_size);
}

static inline uint32_t get_offset_to_tomb_stone_count(const void* page, uint32_t page_size)
{
	return get_offset_to_tuple_count(page, page_size) + get_value_size_on_page(page_size);
}

static inline uint32_t get_offset_to_is_valid_bitmap(const void* page, uint32_t page_size)
{
	return get_offset_to_tomb_stone_count(page, page_size) + get_value_size_on_page(page_size);
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
	uint32_t min_size_8 = 1 + page_header_size + (1 * 2) + bitmap_size_in_bytes(tuple_count) + (tuple_count * tpl_d->size);
	if(min_size_8 <= (1<<8))
		return min_size_8;

	uint32_t min_size_16 = 2 + page_header_size + (2 * 2) + bitmap_size_in_bytes(tuple_count) + (tuple_count * tpl_d->size);
	if(min_size_16 <= (1<<16))
		return min_size_16;

	uint32_t min_size_24 = 3 + page_header_size + (3 * 2) + bitmap_size_in_bytes(tuple_count) + (tuple_count * tpl_d->size);
	if(min_size_24 <= (1<<24))
		return min_size_24;

	uint32_t min_size_32 = 4 + page_header_size + (4 * 2) + bitmap_size_in_bytes(tuple_count) + (tuple_count * tpl_d->size);
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

	// write 0 to tomb_stone_count
	void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
	write_value_to_page(tomb_stone_count, page_size, 0);

	// initialize is_valid bitmap to all 0s
	char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);
	uint32_t tuple_capacity = get_tuple_capacity(page, page_size, tpl_d);
	reset_all_bits(is_valid, tuple_capacity);

	return 1;
}

uint32_t get_tuple_count_fixed_array_page(const void* page, uint32_t page_size)
{
	const void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	return read_value_from_page(tuple_count, page_size);
}

uint32_t get_tomb_stone_count_fixed_array_page(const void* page, uint32_t page_size)
{
	const void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
	return read_value_from_page(tomb_stone_count, page_size);
}

int append_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	if(!can_append_tuple_fixed_array_page(page, page_size, tpl_d))
		return 0;

	// increment the tuple counter on the page
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);
	write_value_to_page(tuple_count, page_size, ++tuple_count_val);

	// the index to the slot, where this external_tuple will be inserted
	uint32_t index = tuple_count_val - 1;

	if(external_tuple != NULL) // append tuple to the new slot
	{
		// set valid bit for the newly created slot, for the new external_tuple
		char* is_valid  = page + get_offset_to_is_valid_bitmap(page, page_size);
		set_bit(is_valid, index);

		// get pointer to the new slot on the page
		void* slot = page + get_offset_to_ith_tuple(page, page_size, tpl_d, index);

		// move data from external_tuple to the slot on the page
		memmove(slot, external_tuple, tpl_d->size);
	}
	else
	{
		// reset valid bit for the newly created slot, for the new external_tuple
		char* is_valid  = page + get_offset_to_is_valid_bitmap(page, page_size);
		reset_bit(is_valid, index);

		// increment the tombstone counter on the page
		void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
		uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);
		write_value_to_page(tomb_stone_count, page_size, ++tomb_stone_count_val);
	}

	return 1;
}

int can_append_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return get_tuple_count_fixed_array_page(page, page_size) < get_tuple_capacity(page, page_size, tpl_d);
}

int update_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple)
{
	// check for: index out of bounds
	if(!can_update_tuple_fixed_array_page(page, page_size, index))
		return 0;

	char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);

	void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);

	// if the tuple at given index exists (was not a tomb_stone) prior to this update
	// then convert it into a tombstone, first
	if(get_bit(is_valid, index) == 1)
	{
		// reset is_valid bit for the concerned slot
		reset_bit(is_valid, index);

		// increment tombstone count
		uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);
		write_value_to_page(tomb_stone_count, page_size, ++tomb_stone_count_val);
	}

	// now the tuple at the concerned index is a tombstone

	// if external tuple is not NULL, then update the slot with this new tuple
	if(external_tuple != NULL)
	{
		// set is_valid bit for the concerned slot
		set_bit(is_valid, index);

		// decrement tombstone count
		uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);
		write_value_to_page(tomb_stone_count, page_size, --tomb_stone_count_val);

		// get pointer to the concerned slot
		void* slot = page + get_offset_to_ith_tuple(page, page_size, tpl_d, index);

		// copy external_tuple to the slot on the page (at index)
		memmove(slot, external_tuple, tpl_d->size);
	}

	return 1;
}

int can_update_tuple_fixed_array_page(const void* page, uint32_t page_size, uint32_t index)
{
	return index < get_tuple_count_fixed_array_page(page, page_size);
}

int discard_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	// index must not be out of bounds
	if(index >= get_tuple_count_fixed_array_page(page, page_size))
		return 0;

	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);

	char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);

	// check if the tuple at concerned index was valid
	int was_valid_at_index = get_bit(is_valid, index);

	for(uint32_t i = index + 1; i < tuple_count_val; i++)
	{
		if(get_bit(is_valid, i) == 1)
		{
			// set the i-1 th bit
			set_bit(is_valid, i-1);

			// copy the tuple contents from (i)th tuple to (i-1)th position
			memory_move(page + get_offset_to_ith_tuple(page, page_size, tpl_d, i-1), page + get_offset_to_ith_tuple(page, page_size, tpl_d, i), tpl_d->size);
		}
		else // we only need to reset the is valid bit
			reset_bit(is_valid, i-1);
	}

	// decrement the tuple count
	write_value_to_page(tuple_count, page_size, --tuple_count_val);

	// if the tuple at the concerned index was a tombstone, then decrement tomb_stone_count, since we discarded a tomb_stone
	if(was_valid_at_index == 0)
	{
		void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
		uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);
		write_value_to_page(tomb_stone_count, page_size, --tomb_stone_count_val);
	}

	return 1;
}

uint32_t discard_trailing_tomb_stones_fixed_array_page(void* page, uint32_t page_size)
{
	uint32_t tomb_stones_discarded = 0;

	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);

	// cache tuple count and tomb_stone_count
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);
	uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);

	char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);

	// while tuple_count is greater than 0, and the last tuple is a tombstone
	while(tuple_count_val > 0 && get_bit(is_valid, tuple_count_val - 1) == 0)
	{
		// we decrement the tuple_count and the tomb_stone_count
		tuple_count_val--;
		tomb_stone_count_val--;
		tomb_stones_discarded++;
	}

	// write the calculated valid tuple count and tomb_stone count
	write_value_to_page(tuple_count, page_size, tuple_count_val);
	write_value_to_page(tomb_stone_count, page_size, tomb_stone_count_val);

	// return the number of tombstoned discarded
	return tomb_stones_discarded;
}

int discard_all_tuples_fixed_array_page(void* page, uint32_t page_size)
{
	// make tuple count as 0
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	write_value_to_page(tuple_count, page_size, 0);

	// make tomb stone count as 0
	void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
	write_value_to_page(tomb_stone_count, page_size, 0);

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

int swap_tuples_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2)
{
	uint32_t tuple_count_val = get_tuple_count_fixed_array_page(page, page_size);

	// index out of bounds
	if(i1 >= tuple_count_val || i2 >= tuple_count_val)
		return 0;

	if(i1 == i2) // nothing to be done
		return 1;

	char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);

	int bit_i1 = get_bit(is_valid, i1);
	int bit_i2 = get_bit(is_valid, i2);

	if(bit_i1 == 0 && bit_i2 == 0) // both the tuples are tomb stones, nothing to be done
		return 1;

	// swap bits of the is_valid bitmap
	bit_i1 ? set_bit(is_valid, i2) : reset_bit(is_valid, i2);
	bit_i2 ? set_bit(is_valid, i1) : reset_bit(is_valid, i1);

	// swap tuple data
	void* tuple_i1 = page + get_offset_to_ith_tuple(page, page_size, tpl_d, i1);
	void* tuple_i2 = page + get_offset_to_ith_tuple(page, page_size, tpl_d, i2);

	memswap(tuple_i1, tuple_i2, tpl_d->size);

	return 1;
}

const void* get_nth_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_fixed_array_page(page, page_size))
		return NULL;

	// indexed tuple does not exist
	const char* is_valid = page + get_offset_to_is_valid_bitmap(page, page_size);
	if(get_bit(is_valid, index) == 0)
		return NULL;

	return page + get_offset_to_ith_tuple(page, page_size, tpl_d, index);
}

void run_page_compaction_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	// Nothing to be done
}

uint32_t get_free_space_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return get_offset_to_end_of_free_space(page_size) - get_offset_to_start_of_free_space(page, page_size, tpl_d);
}

uint32_t get_space_occupied_by_tuples_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t start_index, uint32_t last_index)
{
	return (last_index - start_index + 1) * tpl_d->size;
}

uint32_t get_space_occupied_by_all_tuples_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return get_tuple_count_fixed_array_page(page, page_size) * tpl_d->size;
}

uint32_t get_space_occupied_by_all_tomb_stones_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return get_tomb_stone_count_fixed_array_page(page, page_size) * tpl_d->size;
}

uint32_t get_space_allotted_to_all_tuples_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return page_size - get_offset_to_tuples(page, page_size, tpl_d);
}

uint32_t get_space_to_be_allotted_to_all_tuples_fixed_array_page(uint32_t page_header_size, uint32_t page_size, const tuple_def* tpl_d)
{
	uint32_t space_allotted_to_all_tuples_PLUS_is_valid_bitmap_size_in_bytes = page_size - (get_value_size_on_page(page_size) + page_header_size + (get_value_size_on_page(page_size) * 2));
	uint32_t tuple_capacity = (space_allotted_to_all_tuples_PLUS_is_valid_bitmap_size_in_bytes * 8) / ((tpl_d->size * 8) + 1);
	uint32_t is_valid_bitmap_size_in_bytes = bitmap_size_in_bytes(tuple_capacity);
	return space_allotted_to_all_tuples_PLUS_is_valid_bitmap_size_in_bytes - is_valid_bitmap_size_in_bytes;
}

uint32_t get_additional_space_overhead_per_tuple_fixed_array_page()
{
	return 0;
}

void print_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	printf("FIXED ARRAY PAGE :\n\n");

	uint32_t tup_count = get_tuple_count_fixed_array_page(page, page_size);
	printf("\tTuples :: (%"PRIu32" of %"PRIu32")\n", tup_count, get_tuple_capacity(page, page_size, tpl_d));

	for(uint32_t i = 0; i < tup_count; i++)
	{
		printf("\t\ttuple %"PRIu32"\n", i);
		if(exists_tuple_fixed_array_page(page, page_size, tpl_d, i))
		{
			const void* tuple = get_nth_tuple_fixed_array_page(page, page_size, tpl_d, i);
			uint32_t tuple_size = tpl_d->size;
			printf("\t\t\toffset[%"PRIu32"] size(%"PRIu32") :: ", (uint32_t)((uintptr_t)(tuple - page)), tuple_size);
			print_tuple(tuple, tpl_d);
			printf("\n\n");
		}
		else
			printf("\t\t\t%s\n\n", "TOMB STONE");
	}
	printf("\n\n\n");
}