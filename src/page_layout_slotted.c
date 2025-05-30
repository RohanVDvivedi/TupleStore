#include<tuplestore/page_layout_slotted.h>

#include<cutlery/cutlery_math.h> // used for defragmentation
#include<cutlery/value_arraylist.h> // used for defragmentation

#include<stdlib.h>

#include<tuplestore/page_header.h>
#include<tuplestore/page_layout_util.h>

#include<serint/serial_int.h>

#include<cutlery/cutlery_stds.h>

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

	// end_of_free_space_offset is never 0, hence if it is 0, it is implied is equal to page_size
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

	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);
}

// allocates space (from free space) for the tuple (only the tuple not its offset) of a given size on the page, and returns the allocation offset
// a 0 return from this function implies out of memory, else a valid offset on the page is returned
// it manages the end_of_free_space_offset and decrements it by tuple_size, it basically acts as a stack allocator
static inline uint32_t allocate_space_for_tuple_from_free_space(void* page, uint32_t page_size, uint32_t tuple_size)
{
	// fail if tuple_size is greater than the free_space_on_page
	if(tuple_size > get_free_space_slotted_page(page, page_size))
		return 0;

	// read end_of_free_space_offset from page
	uint32_t end_of_free_space_offset_val = get_offset_to_end_of_free_space(page, page_size);

	// allocate space for the new tuple
	end_of_free_space_offset_val -= tuple_size;

	// write the end_of_free_space_offset back to the page
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);

	// this is the offset there you should copy your tuple
	return end_of_free_space_offset_val;
}

/*
***********************************************************************************************/

uint32_t get_minimum_page_size_for_slotted_page(uint32_t page_header_size, const tuple_size_def* tpl_sz_d, uint32_t tuple_count)
{
	uint32_t min_size_8 = 1 + page_header_size + ((4 + tuple_count) * 1) + (tuple_count * get_minimum_tuple_size_using_tuple_size_def(tpl_sz_d));
	if(min_size_8 <= (UINT32_C(1)<<8))
		return min_size_8;

	uint32_t min_size_16 = 2 + page_header_size + ((4 + tuple_count) * 2) + (tuple_count * get_minimum_tuple_size_using_tuple_size_def(tpl_sz_d));
	if(min_size_16 <= (UINT32_C(1)<<16))
		return min_size_16;

	uint32_t min_size_24 = 3 + page_header_size + ((4 + tuple_count) * 3) + (tuple_count * get_minimum_tuple_size_using_tuple_size_def(tpl_sz_d));
	if(min_size_24 <= (UINT32_C(1)<<24))
		return min_size_24;

	uint32_t min_size_32 = 4 + page_header_size + ((4 + tuple_count) * 4) + (tuple_count * get_minimum_tuple_size_using_tuple_size_def(tpl_sz_d));
	return min_size_32;
}

uint32_t get_maximum_tuple_count_slotted_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	return get_space_to_be_allotted_to_all_tuples_slotted_page(page_header_size, page_size) / (get_minimum_tuple_size_using_tuple_size_def(tpl_sz_d) + get_additional_space_overhead_per_tuple_slotted_page(page_size));
}

uint32_t get_maximum_tuple_size_slotted_page(uint32_t page_header_size, uint32_t page_size)
{
	uint32_t total_space_for_only_tuple = get_space_to_be_allotted_to_all_tuples_slotted_page(page_header_size, page_size);
	uint32_t space_overhead_for_only_tuple = get_additional_space_overhead_per_tuple_slotted_page(page_size);

	if(total_space_for_only_tuple > space_overhead_for_only_tuple)
		return total_space_for_only_tuple - space_overhead_for_only_tuple;

	return 0;
}

int init_slotted_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_size_def* tpl_sz_d)
{
	// the page must be able to accomodate atleast 1 tuple
	if(page_size < get_minimum_page_size_for_slotted_page(page_header_size, tpl_sz_d, 1))
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

int append_tuple_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple)
{
	// if can not append new tuple, then fail with 0
	if(!can_append_tuple_slotted_page(page, page_size, tpl_sz_d, external_tuple))
		return 0;

	// increment tuple count on the page
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);
	write_value_to_page(tuple_count, page_size, ++tuple_count_val);

	// space that will be occupied by this tuple in the page
	// it has to any way occupy a slot for its offset (even if it is a tomb_stone)
	uint32_t space_occupied_by_external_tuple_on_page = get_additional_space_overhead_per_tuple_slotted_page(page_size);

	// new tuple's offset
	void* new_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, tuple_count_val - 1);

	if(external_tuple == NULL)
	{
		// update this new last slot to 0, we are setting it as a tomb_stone
		write_value_to_page(new_tuple_offset, page_size, 0);

		// increment tomb_stone_count on the page
		void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
		uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);
		write_value_to_page(tomb_stone_count, page_size, ++tomb_stone_count_val);
	}
	else
	{
		// calculate the size of tuple to be inserted
		uint32_t external_tuple_size = get_tuple_size_using_tuple_size_def(tpl_sz_d, external_tuple);

		// if it is not a tomb_stone then it will also occupy space for its contents
		space_occupied_by_external_tuple_on_page += external_tuple_size;

		// allocate spze for this new tuple from the free space
		uint32_t new_tuple_offset_val = allocate_space_for_tuple_from_free_space(page, page_size, external_tuple_size);

		// update the new_tuple_offset_val to new_tuple_offset
		write_value_to_page(new_tuple_offset, page_size, new_tuple_offset_val);

		// copy the tuple contents
		memory_move(page + new_tuple_offset_val, external_tuple, external_tuple_size);
	}

	// increment the space_occupied_by_tuples value on the page, by the space that is/will be occupied by this external tuple 
	void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
	uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
	space_occupied_by_tuples_val += space_occupied_by_external_tuple_on_page;
	write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);

	return 1;
}

int can_append_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple)
{
	return get_space_to_be_occupied_by_tuple_slotted_page(page_size, tpl_sz_d, external_tuple) <= get_free_space_slotted_page(page, page_size);
}

int insert_tuple_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple)
{
	// a valid index is, 0 <= index <= tuple_count
	if(index > get_tuple_count_slotted_page(page, page_size))
		return 0;

	// attempt an append and fail, if it fails
	if(!append_tuple_slotted_page(page, page_size, tpl_sz_d, external_tuple))
		return 0;

	// tuple always gets appended at the end of the page, on an append call

	// so our task now is to bring it to the "right" index, by any means possible

	// get the new tuple count value after the append
	// all modifications to tuple_count are done, so it can be cached
	uint32_t tuple_count_val = get_tuple_count_slotted_page(page, page_size);

	// if the external_tuple was to be placed at the last, then return success, immediately
	if(index == tuple_count_val - 1)
		return 1;

	// right rotate the offsets array at index, by the size of offset, to bring the last offset at the "right" index
	memory_right_rotate(page + get_offset_to_ith_tuple_offset(page, page_size, index), (tuple_count_val - index) * get_value_size_on_page(page_size), get_value_size_on_page(page_size));

	return 1;
}

int update_tuple_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple)
{
	if(!can_update_tuple_slotted_page(page, page_size, tpl_sz_d, index, external_tuple))
		return 0;

	void* ith_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, index);
	uint32_t ith_tuple_offset_val = read_value_from_page(ith_tuple_offset, page_size);

	int discard_ith_tuple = 0;

	// atmost only one of below options must be selected
	int append_in_free_space = 0;
	int update_in_place = 0;

	// atmost only one of below options must be selected
	int increment_tomb_stone_count = 0;
	int decrement_tomb_stone_count = 0;

	if(ith_tuple_offset_val == 0 && external_tuple != NULL) // ith tuple is a tomb_stone, so we need to append it in free space
	{
		append_in_free_space = 1;
		decrement_tomb_stone_count = 1;
	}
	else if(ith_tuple_offset_val != 0 && external_tuple == NULL) // no space required for external_tuple, we only need to discard the ith tuple
	{
		discard_ith_tuple = 1;
		increment_tomb_stone_count = 1;
	}
	else if(ith_tuple_offset_val != 0 && external_tuple != NULL) // noone is tomb_stone
	{
		discard_ith_tuple = 1;
		uint32_t ith_tuple_size = get_tuple_size_using_tuple_size_def(tpl_sz_d, page + ith_tuple_offset_val);
		uint32_t external_tuple_size = get_tuple_size_using_tuple_size_def(tpl_sz_d, external_tuple);
		if(ith_tuple_offset_val == get_offset_to_end_of_free_space(page, page_size) || ith_tuple_size < external_tuple_size)
			append_in_free_space = 1;
		else if(ith_tuple_size >= external_tuple_size)
			update_in_place = 1;
	}
	else {} // no need to replace a tomb_stone with another one, nothing to be done

	// cache the old slot offset
	const uint32_t ith_tuple_offset_val_old = ith_tuple_offset_val;

	if(discard_ith_tuple)
	{
		// write a tomb_stone at the ith tuple offset
		ith_tuple_offset_val = 0;
		write_value_to_page(ith_tuple_offset, page_size, ith_tuple_offset_val);

		// compute ith tuple_size
		uint32_t ith_tuple_size = get_tuple_size_using_tuple_size_def(tpl_sz_d, page + ith_tuple_offset_val_old);

		// decrement the space_occupied_by_tuples value on the page, by the space of the ith old tuple, that is freed
		void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
		uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
		space_occupied_by_tuples_val -= ith_tuple_size;
		write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);

		if(ith_tuple_offset_val_old == get_offset_to_end_of_free_space(page, page_size))
			recompute_end_of_free_space_offset(page, page_size);
	}

	if(append_in_free_space || update_in_place)
	{
		uint32_t external_tuple_size = get_tuple_size_using_tuple_size_def(tpl_sz_d, external_tuple);

		if(append_in_free_space)
		{
			// allocate and update the ith_tuple_offset
			ith_tuple_offset_val = allocate_space_for_tuple_from_free_space(page, page_size, external_tuple_size);
			write_value_to_page(ith_tuple_offset, page_size, ith_tuple_offset_val);
		}
		else
		{
			// update the ith tuple at the old offset in place
			ith_tuple_offset_val = ith_tuple_offset_val_old;
			write_value_to_page(ith_tuple_offset, page_size, ith_tuple_offset_val);
		}

		// copy the ith tuple at the offset
		memory_move(page + ith_tuple_offset_val, external_tuple, external_tuple_size);

		// increment the space_occupied_by_tuples value on the page, by the space of the external_tuple
		void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
		uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
		space_occupied_by_tuples_val += external_tuple_size;
		write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);
	}

	if(increment_tomb_stone_count)
	{
		// increment tomb_stone_count on the page
		void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
		uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);
		write_value_to_page(tomb_stone_count, page_size, ++tomb_stone_count_val);
	}
	else if(decrement_tomb_stone_count)
	{
		// decrement tomb_stone_count on the page
		void* tomb_stone_count = page + get_offset_to_tomb_stone_count(page, page_size);
		uint32_t tomb_stone_count_val = read_value_from_page(tomb_stone_count, page_size);
		write_value_to_page(tomb_stone_count, page_size, --tomb_stone_count_val);
	}

	return 1;
}

int can_update_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple)
{
	// index out of bounds, can't update
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	// a tomb stone can be inserted without much thought,
	// since in this case we only need to set the ith tuple_offset to 0
	if(external_tuple == NULL)
		return 1;

	// calculate size of external_tuple
	uint32_t external_tuple_size = get_tuple_size_using_tuple_size_def(tpl_sz_d, external_tuple);

	// get offset of the tuple existing at ith index
	uint32_t ith_tuple_offset_val = get_offset_to_ith_tuple(page, page_size, index);

	// get actual free space on page
	uint32_t free_space_on_page = get_free_space_slotted_page(page, page_size);

	// if the tuple exists at the ith index, then
	if(ith_tuple_offset_val != 0)
	{
		uint32_t ith_tuple_size = get_tuple_size_using_tuple_size_def(tpl_sz_d, page + ith_tuple_offset_val);

		// if it is at the end of free space, then it will add to the free space on being tomb_stoned
		if(ith_tuple_offset_val == get_offset_to_end_of_free_space(page, page_size))
		{
			if(free_space_on_page + ith_tuple_size >= external_tuple_size)
				return 1;
			else
				return 0;
		}
		// else try to fit external_tuple in the space of the old tuple
		else if(ith_tuple_size >= external_tuple_size)
			return 1;
	}
	
	// if nothing succeeds, we would have to allocate from the free space
	if(free_space_on_page >= external_tuple_size)
		return 1;

	// return of 0, here indicates that it MAY not fit on the page
	return 0;
}

int discard_tuple_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	// pre-compute the offset and the size of the ith tuple that we need to discard
	uint32_t ith_tuple_offset_old_val = get_offset_to_ith_tuple(page, page_size, index);
	uint32_t ith_tuple_size_old = (ith_tuple_offset_old_val == 0) ? 0 : get_tuple_size_using_tuple_size_def(tpl_sz_d, page + ith_tuple_offset_old_val);

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

	return 1;
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

void discard_all_tuples_slotted_page(void* page, uint32_t page_size)
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
}

int exists_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	return get_offset_to_ith_tuple(page, page_size, index) != 0;
}

int swap_tuples_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t i1, uint32_t i2)
{
	uint32_t tuple_count_val =  get_tuple_count_slotted_page(page, page_size);

	// index out of bounds
	if(i1 >= tuple_count_val || i2 >= tuple_count_val)
		return 0;

	if(i1 == i2) // nothing to be done
		return 1;

	void* i1th_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, i1);
	void* i2th_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, i2);

	// swap tuple offsets
	// doesn't matter they are NULL or not, the effect is the same
	swap_values_on_page(i1th_tuple_offset, i2th_tuple_offset, page_size);

	return 1;
}

const void* get_nth_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index)
{
	// index out of bounds
	if(index >= get_tuple_count_slotted_page(page, page_size))
		return NULL;

	// get offset of this ith tuple
	uint32_t ith_tuple_offset_val = get_offset_to_ith_tuple(page, page_size, index);

	// tuple offset at ith index is 0, indicating it does not exists
	if(ith_tuple_offset_val == 0)
		return NULL;

	return page + ith_tuple_offset_val;
}

int set_element_in_tuple_in_place_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t tuple_index, positional_accessor element_index, const user_value* value)
{
	void* tuple_concerned = (void*) get_nth_tuple_slotted_page(page, page_size, &(tpl_d->size_def), tuple_index);

	// if the corresponding tuple does not exist, fail
	if(tuple_concerned == NULL)
		return 0;

	// cache the old tuple size
	uint32_t old_tuple_size = get_tuple_size(tpl_d, tuple_concerned);

	// fail if we can't set the tuple in the page without size increment
	if(!can_set_element_in_tuple(tpl_d, element_index, tuple_concerned, value, 0))
		return 0;

	// set the element, without any size increment
	int set_in_place_success = set_element_in_tuple(tpl_d, element_index, tuple_concerned, value, 0);

	// if set_element_in_tuple was a success, i.e tuple was updated, then do book-keeping
	// i.e. we must update the space_occupied_by_tuples on the page
	if(set_in_place_success)
	{
		uint32_t new_tuple_size = get_tuple_size(tpl_d, tuple_concerned);

		void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
		uint32_t space_occupied_by_tuples_val = read_value_from_page(space_occupied_by_tuples, page_size);
		// update space_occupied_by_tuples_val += (new_tuple_size - old_tuple_size)
		space_occupied_by_tuples_val += (new_tuple_size - old_tuple_size);
		write_value_to_page(space_occupied_by_tuples, page_size, space_occupied_by_tuples_val);
	}

	return set_in_place_success;
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
declarations_value_arraylist(tuple_offset_indexed_list, tuple_offset_indexed, static inline)
#define EXPANSION_FACTOR 1.5
function_definitions_value_arraylist(tuple_offset_indexed_list, tuple_offset_indexed, static inline)

int run_page_compaction_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, int* memory_allocation_error)
{
	// return value of the function
	int was_page_compacted = 0;

	// space_occupied_by_tuples value remains the same even after compaction
	// the logical contents of the page are unaltered, including the tuple_indices and the tomb stone indices

	uint32_t tuple_count = get_tuple_count_slotted_page(page, page_size);

	tuple_offset_indexed_list tuple_offset_list;

	// fail compaction, if malloc fails
	if(!initialize_tuple_offset_indexed_list(&tuple_offset_list, tuple_count))
	{
		(*memory_allocation_error) = 1;
		return 0;
	}

	// tuple_offset_list must consists only of existing tuples
	for(uint32_t index = 0; index < tuple_count; index++)
	{
		if(exists_tuple_slotted_page(page, page_size, tpl_sz_d, index))
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
		uint32_t tuple_size = get_tuple_size_using_tuple_size_def(tpl_sz_d, tuple);

		// allocate space for the tuple that is to be moved
		end_of_free_space_offset_val -= tuple_size;

		// end_of_free_space_offset_val is the new offset of the tuple
		// if it does not equal the curent_offset fo the tuple, then the page was compacted by moving atleast a tuple
		if(end_of_free_space_offset_val != tuple_offset_and_index.offset)
			was_page_compacted = 1;

		// move the tuple to the allocated space
		memory_move(page + end_of_free_space_offset_val, tuple, tuple_size);

		// update the tuple_offset on the page
		write_value_to_page(tuple_offset, page_size, end_of_free_space_offset_val);
	}

	deinitialize_tuple_offset_indexed_list(&tuple_offset_list);

	// reset the end of free space offset for the page
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);

	return was_page_compacted;
}

int zero_out_free_space_slotted_page(void* page, uint32_t page_size)
{
	if(0 == get_free_space_slotted_page(page, page_size))
		return 0;

	memory_set(page + get_offset_to_start_of_free_space(page, page_size), 0, get_offset_to_end_of_free_space(page, page_size) - get_offset_to_start_of_free_space(page, page_size));
	return 1;
}

uint32_t get_free_space_slotted_page(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_free_space(page, page_size) - get_offset_to_start_of_free_space(page, page_size);
}

uint32_t get_space_occupied_by_tuples_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t start_index, uint32_t last_index)
{
	uint32_t tuples_total_size = 0;
	for(uint32_t i = start_index; i <= last_index; i++)
	{
		if(exists_tuple_slotted_page(page, page_size, tpl_sz_d, i))
			tuples_total_size += get_tuple_size_using_tuple_size_def(tpl_sz_d, get_nth_tuple_slotted_page(page, page_size, tpl_sz_d, i));
	}
	return tuples_total_size + (last_index - start_index + 1) * get_additional_space_overhead_per_tuple_slotted_page(page_size);
}

uint32_t get_space_occupied_by_all_tuples_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	// we cache space_occupied_by_tuples on the slotted page
	const void* space_occupied_by_tuples = page + get_offset_to_space_occupied_by_tuples(page, page_size);
	return read_value_from_page(space_occupied_by_tuples, page_size);
}

uint32_t get_space_occupied_by_all_tomb_stones_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	return get_tomb_stone_count_slotted_page(page, page_size) * get_additional_space_overhead_per_tuple_slotted_page(page_size);
}

uint32_t get_space_to_be_occupied_by_tuple_slotted_page(uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple)
{
	// even a tomb_stone (external_tuple == NULL) occupies the additional_space_overhead_per_tuple
	// space for tuple data is occupied only if the tuple is not a tomb_stone (i.e. external_tuple != NULL)
	return get_additional_space_overhead_per_tuple_slotted_page(page_size)
	+ ((external_tuple == NULL) ? 0 : get_tuple_size_using_tuple_size_def(tpl_sz_d, external_tuple));
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

	for(uint32_t i = 0; i < tup_count; i++)
	{
		printf("\t\ttuple %"PRIu32"\n", i);
		if(exists_tuple_slotted_page(page, page_size, &(tpl_d->size_def), i))
		{
			const void* tuple = get_nth_tuple_slotted_page(page, page_size,  &(tpl_d->size_def), i);
			uint32_t tuple_size = get_tuple_size_using_tuple_size_def(&(tpl_d->size_def), tuple);
			printf("\t\t\toffset[%"PRIu32"] size(%"PRIu32") :: ", (uint32_t)((uintptr_t)(tuple - page)), tuple_size);
			print_tuple(tuple, tpl_d);
			printf("\n\n");
		}
		else
			printf("\t\t\t%s\n\n", "TOMB STONE");
	}
	printf("\n\n\n");
}