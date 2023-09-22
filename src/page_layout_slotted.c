#include<page_layout_slotted.h>

#include<cutlery_math.h> // used for defragmentation
#include<value_arraylist.h> // used for defragmentation

#include<stdlib.h>
#include<string.h>

#include<tuple.h>

#include<int_accesses.h>
#include<page_header.h>
#include<page_layout_util.h>

/*
** 		offset calculation functions
*/

static inline uint32_t get_offset_to_space_occupied_by_tuples(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_page_header(page, page_size);
}

static inline uint32_t get_offset_to_tuple_count(const void* page, uint32_t page_size)
{
	return get_offset_to_space_occupied_by_tuples(page, page_size) + get_value_size_on_page(page_size);
}

static inline uint32_t get_offset_to_tomb_stone_count(const void* page, uint32_t page_size)
{
	return get_offset_to_tuple_count(page, page_size) + get_value_size_on_page(page_size);
}

static inline uint32_t get_offset_to_end_of_free_space_offset(const void* page, uint32_t page_size)
{
	return get_offset_to_tomb_stone_count(page, page_size) + get_value_size_on_page(page_size);
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
	const void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	uint32_t end_of_free_space_offset_val = read_value_from_page(end_of_free_space_offset, page_size);

	// if the tuple_count == 0, the free space end with the end of page
	return (end_of_free_space_offset_val == 0) ? page_size : end_of_free_space_offset_val;
}

static inline uint32_t get_offset_to_ith_tuple(const void* page, uint32_t page_size, uint32_t ith)
{
	const void* ith_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, ith);
	return read_value_from_page(ith_tuple_offset, page_size);
}

// call this function to loop over all the tuple offsets and recompute the end_of_free_space_offset on page
// this is O(n) operation, hence use it only when utmost necessary
static inline void recompute_end_of_free_space_offset(void* page, uint32_t page_size)
{
	// initialize end_of_free_space_offset_val to page_size
	uint32_t end_of_free_space_offset_val = page_size;

	const void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	const uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);

	for(uint32_t i = 0; i < tuple_count_val; i++)
	{
		uint32_t ith_tuple_offset = get_offset_to_ith_tuple(page, page_size, i);
		if(ith_tuple_offset != 0) // if the tuple exists, then we may have to bump up the end_of_free_space_offset_val
			end_of_free_space_offset_val = min(end_of_free_space_offset_val, ith_tuple_offset);
	}

	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);
}

/*
***********************************************************************************************/

uint32_t get_minimum_page_size_for_slotted_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count)
{
	uint32_t min_size_8 = 1 + page_header_size + ((4 + tuple_count) * 1) + (tuple_count * get_minimum_tuple_size(tpl_d));
	if(min_size_8 <= (UINT32_C(1)<<8))
		return min_size_8;

	uint32_t min_size_16 = 2 + page_header_size + ((4 + tuple_count) * 2) + (tuple_count * get_minimum_tuple_size(tpl_d));
	if(min_size_16 <= (UINT32_C(1)<<16))
		return min_size_16;

	uint32_t min_size_24 = 3 + page_header_size + ((4 + tuple_count) * 3) + (tuple_count * get_minimum_tuple_size(tpl_d));
	if(min_size_24 <= (UINT32_C(1)<<24))
		return min_size_24;

	uint32_t min_size_32 = 4 + page_header_size + ((4 + tuple_count) * 4) + (tuple_count * get_minimum_tuple_size(tpl_d));
	return min_size_32;
}

int init_slotted_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d)
{
	// the page must be able to accomodate atleast 1 tuple
	if(page_size < get_minimum_page_size_for_slotted_page(page_header_size, tpl_d, 1))
		return 0;

	// initialize page header
	if(init_page_header(page, page_size, page_header_size) == 0)
		return 0;

	// write 0 to space_occupied_by_tuples
	void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
	write_value_to_page(space_occupied_by_tuples, page_size, 0);

	// write 0 to tuple_count
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	write_value_to_page(tuple_count, page_size, 0);

	// write 0 to tomb_stone_count
	void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
	write_value_to_page(tomb_stone_count, page_size, 0);

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

uint32_t get_tomb_stone_count_slotted_page(const void* page, uint32_t page_size)
{
	const void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
	return read_value_from_page(tomb_stone_count, page_size);
}

// TODO : reimplement
int append_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	// if can not append new tuple, then fail with 0
	if(!can_append_tuple_slotted_page(page, page_size, tpl_d, external_tuple))
		return 0;

	// calculate the size of tuple to be inserted
	uint32_t external_tuple_size = get_tuple_size(tpl_d, external_tuple);

	// increment tuple count on the page
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);
	write_value_to_page(tuple_count, page_size, ++tuple_count_val);

	// update end of free space offset
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	uint32_t end_of_free_space_offset_val = get_offset_to_end_of_free_space(page, page_size);
	end_of_free_space_offset_val -= external_tuple_size;
	write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);

	// increment the space_occupied_by_tuples value on the page, by the space that is/will be occupied by this external tuple 
	void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
	uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
	space_occupied_by_tuples_val += external_tuple_size + get_additional_space_overhead_per_tuple_slotted_page(page_size);
	write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);

	// update offset where you want to place this tuple
	void* new_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, tuple_count_val - 1);
	write_value_to_page(new_tuple_offset, page_size, end_of_free_space_offset_val);

	// get pointer to the new slot (new_tuple) on the page
	void* new_tuple = page + end_of_free_space_offset_val;

	// move data from external tuple to the slot (new_tuple) in the page
	memmove(new_tuple, external_tuple, external_tuple_size);

	return 1;
}

int can_append_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	uint32_t space_required_for_tuple = (external_tuple == NULL) ? 0 : get_tuple_size(tpl_d, external_tuple);

	uint32_t space_required_for_tuple_offest = get_additional_space_overhead_per_tuple_slotted_page(page_size);

	// tuple needs space for itself and its offset
	uint32_t total_space_required_for_new_tuple = space_required_for_tuple + space_required_for_tuple_offest;

	return total_space_required_for_new_tuple <= get_free_space_slotted_page(page, page_size);
}

// TODO : reimplement
int update_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple)
{
	// index out of bounds
	uint32_t tuple_count_val = get_tuple_count_slotted_page(page, page_size);
	if(index >= tuple_count_val)
		return 0;

	// calculate the size of tuple to be inserted in place of an old one
	uint32_t external_tuple_size = get_tuple_size(tpl_d, external_tuple);

	// get free space on page
	uint32_t free_space = get_free_space_slotted_page(page, page_size);

	// we might have to update this
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	uint32_t end_of_free_space_offset_val = get_offset_to_end_of_free_space(page, page_size);

	void* existing_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, index);
	uint32_t existing_tuple_offset_val = get_offset_to_ith_tuple(page, page_size, index);

	// if a tuple already exists at the given index
	if(existing_tuple_offset_val != 0)
	{
		void* existing_tuple = page + existing_tuple_offset_val;
		uint32_t existing_tuple_size = get_tuple_size(tpl_d, existing_tuple);

		// if the exiting tuple is physically the last tuple
		if(existing_tuple_offset_val == end_of_free_space_offset_val)
		{
			if(free_space + existing_tuple_size >= external_tuple_size)
			{
				// move existing_tuple_offset and end_of_free_space_offset to accomodate the new tuple

				end_of_free_space_offset_val = end_of_free_space_offset_val + existing_tuple_size - external_tuple_size;
				write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);

				existing_tuple_offset_val = end_of_free_space_offset_val;
				write_value_to_page(existing_tuple_offset, page_size, existing_tuple_offset_val);

				// modify the space_occupied_by_tuples
				void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
				uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
				space_occupied_by_tuples_val = (space_occupied_by_tuples_val - existing_tuple_size) + external_tuple_size;
				write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);

				existing_tuple = page + existing_tuple_offset_val;

				// move data from external tuple to the tuple in the page
				memmove(existing_tuple, external_tuple, external_tuple_size);

				return 1;
			}
			else
				return 0;
		}
		// if exiting_tuple is larger than or equal to the external_tuple, then we can use the same slot
		else if(existing_tuple_size >= external_tuple_size)
		{
			// modify the space_occupied_by_tuples
			void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
			uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
			space_occupied_by_tuples_val = (space_occupied_by_tuples_val - existing_tuple_size) + external_tuple_size;
			write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);

			// move data from external tuple to the tuple in the page
			memmove(existing_tuple, external_tuple, external_tuple_size);

			return 1;
		}
	}
	
	// either the existing_tuple is NULL (i.e. deleted or is tomb_stone) OR there isn't enough space to fit in the external tuple
	// so we need a take space from the free space on the page
	if(free_space >= external_tuple_size)
	{
		// now here if we are updating to a slot that was holding a tomb_stone
		// then we need to decrement the tomb_stone count
		if(existing_tuple_offset_val == 0)
		{
			void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
			uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size) - 1;
			write_value_to_page(tomb_stone_count, page_size, tomb_stone_count_val);
		}

		// update end of free space offset
		end_of_free_space_offset_val -= external_tuple_size;
		write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);

		// update offset where you want to place this tuple
		void* new_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, index);
		write_value_to_page(new_tuple_offset, page_size, end_of_free_space_offset_val);

		// modify the space_occupied_by_tuples
		void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
		uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
		if(existing_tuple_offset_val == 0)
			space_occupied_by_tuples_val = space_occupied_by_tuples_val + external_tuple_size;
		else
		{
			void* existing_tuple = page + existing_tuple_offset_val;
			uint32_t existing_tuple_size = get_tuple_size(tpl_d, existing_tuple);
			space_occupied_by_tuples_val = (space_occupied_by_tuples_val - existing_tuple_size) + external_tuple_size;
		}
		write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);

		// find the new tuple on the page
		void* new_tuple = page + end_of_free_space_offset_val;

		// move data from external tuple to the tuple in the page
		memmove(new_tuple, external_tuple, external_tuple_size);

		return 1;
	}
	else
		return 0;
}

int can_update_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple)
{
	// TODO : implement
	return 0;
}

int discard_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	// pre-compute the offset and the size of the ith tuple that we need to discard
	uint32_t ith_tuple_offset_old_val = get_offset_to_ith_tuple(page, page_size, index);
	uint32_t ith_tuple_size_old = (ith_tuple_offset_old_val == 0) ? 0 : get_tuple_size(tpl_d, page + ith_tuple_offset_old_val);

	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);

	// move all tuple offsets after i ([i+1, tuple_count_val-1]), to ith index
	memory_move(page + get_offset_to_ith_tuple_offset(page, page_size, index),
				page + get_offset_to_ith_tuple_offset(page, page_size, index + 1),
				(tuple_count_val - (index + 1)) * get_additional_space_overhead_per_tuple_slotted_page(page_size));

	// decrement tuple_count
	write_value_to_page(tuple_count, page_size, --tuple_count_val);

	// decrement tomb_stone_count, if the discarded tuple was a tomb_stone
	if(ith_tuple_offset_old_val == 0)
	{
		void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
		uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);
		write_value_to_page(tomb_stone_count, page_size, --tomb_stone_count_val);
	}

	// decrement the space_occupied_by_tuples, by the tuple_size and the space occupied by its offset
	void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
	uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
	space_occupied_by_tuples_val -= (ith_tuple_size_old + get_additional_space_overhead_per_tuple_slotted_page(page_size));
	write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);

	// if the discarded tuple_offset was excatly at the end_of_free_space_offset then we need to recompute the end_of_free_space_offset
	if(ith_tuple_offset_old_val == get_offset_to_end_of_free_space(page, page_size))
		recompute_end_of_free_space_offset(page, page_size);

	return 0;
}

uint32_t discard_trailing_tomb_stones_slotted_page(void* page, uint32_t page_size)
{
	uint32_t tomb_stones_discarded = 0;

	void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);

	// cache space_occupied_by_tuples, tuple count and tomb_stone count
	uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);
	uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);

	// pre-calculate the additional space that each of these tomb_stones are hoarding
	uint32_t additional_space_for_tomb_stones = get_additional_space_overhead_per_tuple_slotted_page(page_size);

	// while tuple_count is greater than 0, and the last tuple is a tombstone
	while(tuple_count_val > 0 && get_offset_to_ith_tuple(page, page_size, tuple_count_val - 1) == 0)
	{
		space_occupied_by_tuples_val -= additional_space_for_tomb_stones;
		tuple_count_val--;
		tomb_stone_count_val--;
		tomb_stones_discarded++;
	}

	// write the calculated valid space_occupied_by_tuples, tuple count and tomb_stone count back to the page
	write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);
	write_value_to_page(tuple_count, page_size, tuple_count_val);
	write_value_to_page(tomb_stone_count, page_size, tomb_stone_count_val);

	return tomb_stones_discarded;
}

int discard_all_tuples_slotted_page(void* page, uint32_t page_size)
{
	// write 0 to space_occupied_by_tuples
	void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
	write_value_to_page(space_occupied_by_tuples, page_size, 0);

	// write 0 to tuple_count
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	write_value_to_page(tuple_count, page_size, 0);

	// write 0 to tomb_stone_count
	void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
	write_value_to_page(tomb_stone_count, page_size, 0);

	// write page_size to end_of_free_space_offset
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, page_size);

	return 1;
}

int exists_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	return get_offset_to_ith_tuple(page, page_size, index) != 0;
}

int swap_tuples_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2)
{
	// index out of bounds
	if(i1 >= get_tuple_count_slotted_page(page, page_size) || i2 >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	if(i1 == i2) // nothing to be done
		return 1;

	void* i1th_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, i1);
	void* i2th_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, i2);

	uint32_t i1th_tuple_offset_val = get_offset_to_ith_tuple(page, page_size, i1);
	uint32_t i2th_tuple_offset_val = get_offset_to_ith_tuple(page, page_size, i2);

	if(i1th_tuple_offset_val == 0 && i2th_tuple_offset_val == 0)  // both the tuples are tomb stones, nothing to be done
		return 1;

	// swap tuple offsets
	write_value_to_page(i1th_tuple_offset, page_size, i2th_tuple_offset_val);
	write_value_to_page(i2th_tuple_offset, page_size, i1th_tuple_offset_val);

	return 1;
}

const void* get_nth_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return NULL;

	// tuple offset at ith index is 0, indicating it does not exists
	if(get_offset_to_ith_tuple(page, page_size, index) == 0)
		return NULL;

	return page + get_offset_to_ith_tuple(page, page_size, index);
}

// a small struct used for defragmentation
// required to store tuple offset and its index on page
typedef struct tuple_offset_indexed tuple_offset_indexed;
struct tuple_offset_indexed
{
	uint32_t offset;
	uint32_t index;
};

static int compare_by_offset_descending(const void* a, const void* b)
{
	return -compare_numbers(((const tuple_offset_indexed*)a)->offset, ((const tuple_offset_indexed*)b)->offset);
}

data_definitions_value_arraylist(tuple_offset_indexed_list, tuple_offset_indexed)
declarations_value_arraylist(tuple_offset_indexed_list, tuple_offset_indexed)
#define EXPANSION_FACTOR 1.5
function_definitions_value_arraylist(tuple_offset_indexed_list, tuple_offset_indexed)

void run_page_compaction_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	// space_occupied_by_tuples value remains the same even after compaction
	// the logical contents of the page are unaltered, including the tuple_indices and the tomb stone indices

	uint16_t tuple_count = get_tuple_count_slotted_page(page, page_size);

	tuple_offset_indexed_list tuple_offset_list;
	initialize_tuple_offset_indexed_list(&tuple_offset_list, tuple_count);

	// tuple_offset_list must consists only of existing tuples
	for(uint32_t index = 0; index <= tuple_count; index++)
	{
		if(exists_tuple_slotted_page(page, page_size, tpl_d, index))
		{
			push_back_to_tuple_offset_indexed_list(&tuple_offset_list, 
			&((const tuple_offset_indexed){
				.offset = get_offset_to_ith_tuple(page, page_size, index),
				.index = index,
			}));
		}
	}

	// sort tuples_offset_list by their offset in decreasing order
	if(get_element_count_tuple_offset_indexed_list(&tuple_offset_list) > 1)
		heap_sort_tuple_offset_indexed_list(&tuple_offset_list, 0, get_element_count_tuple_offset_indexed_list(&tuple_offset_list)-1, &simple_comparator(compare_by_offset_descending));

	// start allocating as if it is a new page
	uint32_t end_of_free_space_offset_val = page_size;

	// laydown the tuples on the page as if a new page
	while(!is_empty_tuple_offset_indexed_list(&tuple_offset_list))
	{
		// get the top of the tuple_offset_list and pop it
		tuple_offset_indexed tuple_offset_and_index = *get_front_of_tuple_offset_indexed_list(&tuple_offset_list);
		pop_front_from_tuple_offset_indexed_list(&tuple_offset_list);

		void* tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, tuple_offset_and_index.index);
		void* tuple = page + get_offset_to_ith_tuple(page, page_size, tuple_offset_and_index.index);

		// get tuple size
		uint32_t tuple_size = get_tuple_size(tpl_d, tuple);

		// allocate space for the tuple that is to be moved
		end_of_free_space_offset_val -= tuple_size;

		// move the tuple to the allocated space
		memmove(page + end_of_free_space_offset_val, tuple, tuple_size);

		// update the tuple_offset on the page
		write_value_to_page(tuple_offset, page_size, end_of_free_space_offset_val);
	}

	deinitialize_tuple_offset_indexed_list(&tuple_offset_list);

	// reset the end of free space offset for the page
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);
}

uint32_t get_free_space_slotted_page(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_free_space(page, page_size) - get_offset_to_start_of_free_space(page, page_size);
}

uint32_t get_space_occupied_by_tuples_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t start_index, uint32_t last_index)
{
	uint32_t tuples_total_size = 0;
	for(uint32_t i = start_index; i <= last_index; i++)
	{
		if(exists_tuple_slotted_page(page, page_size, tpl_d, i))
			tuples_total_size += get_tuple_size(tpl_d, get_nth_tuple_slotted_page(page, page_size, tpl_d, i));
	}
	return tuples_total_size + (last_index - start_index + 1) * get_additional_space_overhead_per_tuple_slotted_page(page_size);
}

uint32_t get_space_occupied_by_all_tuples_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	// we cache space_occupied_by_tuples on the slotted page
	const void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
	return read_value_from_page(space_occupied_by_tuples, page_size);
}

uint32_t get_space_occupied_by_all_tomb_stones_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return get_tomb_stone_count_slotted_page(page, page_size) * get_additional_space_overhead_per_tuple_slotted_page(page_size);
}

uint32_t get_space_allotted_to_all_tuples_slotted_page(const void* page, uint32_t page_size)
{
	return page_size - get_offset_to_tuple_offsets(page, page_size);
}

uint32_t get_space_to_be_allotted_to_all_tuples_slotted_page(uint32_t page_header_size, uint32_t page_size)
{
	return page_size - (get_value_size_on_page(page_size) + page_header_size + (4 * get_value_size_on_page(page_size)));
}

uint32_t get_additional_space_overhead_per_tuple_slotted_page(uint32_t page_size)
{
	// the additional space is taken up by tuple_offset per tuple, while storing the tuple
	return get_value_size_on_page(page_size);
}

void print_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	printf("SLOTTED PAGE :\n\n");

	uint32_t tup_count = get_tuple_count_slotted_page(page, page_size);
	printf("\tTuples :: (%"PRIu32")\n", tup_count);

	for(uint16_t i = 0; i < tup_count; i++)
	{
		printf("\t\ttuple %"PRIu32"\n", i);
		if(exists_tuple_slotted_page(page, page_size, tpl_d, i))
		{
			const void* tuple = get_nth_tuple_slotted_page(page, page_size, tpl_d, i);
			uint32_t tuple_size = get_tuple_size(tpl_d, tuple);
			printf("\t\t\toffset[%"PRIu32"] size(%"PRIu32") :: ", (uint32_t)((uintptr_t)(tuple - page)), tuple_size);
			print_tuple(tuple, tpl_d);
			printf("\n\n");
		}
		else
			printf("\t\t\t%s\n\n", "TOMB STONE");
	}
	printf("\n\n\n");
}