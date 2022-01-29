#include<page_layout_slotted.h>

#include<stdint.h>
#include<stdlib.h>
#include<string.h>

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

int init_slotted_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d)
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

int insert_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	// if can not insert new tuple, then fail with 0
	if(!can_insert_tuple_slotted_page(page, page_size, tpl_d, external_tuple))
		return 0;

	// calculate the size of tuple to be inserted
	uint32_t external_tuple_size = get_tuple_size(tpl_d, external_tuple);

	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);

	// increment tuple count on the page
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);
	write_value_to_page(tuple_count, page_size, ++tuple_count_val);

	// update end of free space offset
	uint32_t end_of_free_space_offset_val = get_offset_to_end_of_free_space(page, page_size);
	end_of_free_space_offset_val -= external_tuple_size;
	write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);

	// update offset where you want to place this tuple
	void* new_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, tuple_count_val - 1);
	write_value_to_page(new_tuple_offset, page_size, end_of_free_space_offset_val);

	// find the new tuple on the page
	void* new_tuple = page + end_of_free_space_offset_val;

	// move data from external tuple to the tuple in the page
	memmove(new_tuple, external_tuple, external_tuple_size);

	return 1;
}

int can_insert_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	// tuple needs space for itself and its offset
	uint32_t size_required_for_new_tuple = get_value_size_on_page(page_size) + get_tuple_size(tpl_d, external_tuple);

	return size_required_for_new_tuple <= get_free_space_slotted_page(page, page_size);
}

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

	if(exists_tuple_slotted_page(page, page_size, tpl_d, index))
	{
		void* existing_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, index);
		uint32_t existing_tuple_offset_val = get_offset_to_ith_tuple(page, page_size, index);
		void* existing_tuple = page + existing_tuple_offset_val;
		uint32_t existing_tuple_size = get_tuple_size(tpl_d, existing_tuple);

		if(existing_tuple_offset_val == end_of_free_space_offset_val)
		{
			if(free_space + existing_tuple_size >= external_tuple_size)
			{
				// move existing_tuple_offset and end_of_free_space_offset to accomodate the new tuple

				end_of_free_space_offset_val = end_of_free_space_offset_val + existing_tuple_size - external_tuple_size;
				write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);

				existing_tuple_offset_val = end_of_free_space_offset_val;
				write_value_to_page(existing_tuple_offset, page_size, existing_tuple_offset_val);

				existing_tuple = page + existing_tuple_offset_val;

				// move data from external tuple to the tuple in the page
				memmove(existing_tuple, external_tuple, external_tuple_size);

				return 1;
			}
			else
				return 0;
		}
	}
	

	if(free_space >= external_tuple_size)
	{
		// update end of free space offset
		end_of_free_space_offset_val -= external_tuple_size;
		write_value_to_page(end_of_free_space_offset, page_size, end_of_free_space_offset_val);

		// update offset where you want to place this tuple
		void* new_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, index);
		write_value_to_page(new_tuple_offset, page_size, end_of_free_space_offset_val);

		// find the new tuple on the page
		void* new_tuple = page + end_of_free_space_offset_val;

		// move data from external tuple to the tuple in the page
		memmove(new_tuple, external_tuple, external_tuple_size);

		return 1;
	}
	else
		return 0;
}

static inline void retract_tuple_count(void* page, uint32_t page_size)
{
	void* tuple_count = page + get_offset_to_tuple_count(page, page_size);

	// cache tuple count
	uint32_t tuple_count_val = read_value_from_page(tuple_count, page_size);

	// get a valif tuple count
	while(tuple_count_val > 0)
	{
		if(get_offset_to_ith_tuple(page, page_size, tuple_count_val - 1) == 0)
			tuple_count_val--;
		else
			break;
	}

	// write the calculated valid tuple count
	write_value_to_page(tuple_count, page_size, tuple_count_val);
}

int delete_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
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
	retract_tuple_count(page, page_size);

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
	if(i1 == i2 || i1 >= get_tuple_count_slotted_page(page, page_size) || i2 >= get_tuple_count_slotted_page(page, page_size))
		return 0;

	void* i1th_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, i1);
	void* i2th_tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, i2);

	uint32_t i1th_tuple_offset_val = get_offset_to_ith_tuple(page, page_size, i1);
	uint32_t i2th_tuple_offset_val = get_offset_to_ith_tuple(page, page_size, i2);

	if(i1th_tuple_offset_val == 0 && i2th_tuple_offset_val == 0)
		return 0;

	// swap tuple offsets
	uint32_t temp = i1th_tuple_offset_val;
	write_value_to_page(i1th_tuple_offset, page_size, i2th_tuple_offset_val);
	write_value_to_page(i2th_tuple_offset, page_size, temp);

	// retract tuple count if possible
	retract_tuple_count(page, page_size);

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

void run_page_compaction_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, int discard_tomb_stones)
{
	if(discard_tomb_stones)
	{
		uint32_t new_tuple_count = 0;
		for(uint32_t i = 0; i < get_tuple_count_slotted_page(page, page_size); i++)
		{
			if(exists_tuple_slotted_page(page, page_size, tpl_d, i))
			{
				if(i != new_tuple_count)
					swap_tuples_slotted_page(page, page_size, tpl_d, i, new_tuple_count);
				new_tuple_count++;
			}
		}

		void* tuple_count = page + get_offset_to_tuple_count(page, page_size);
		write_value_to_page(tuple_count, page_size, new_tuple_count);
	}

	// now defragmenting the page

	// create a copy page
	void* copy_page = malloc(page_size);
	memmove(copy_page, page, page_size);

	uint16_t tuple_count = get_tuple_count_slotted_page(page, page_size);

	// reset the allocator offset for the actual page
	void* end_of_free_space_offset = page + get_offset_to_end_of_free_space_offset(page, page_size);
	write_value_to_page(end_of_free_space_offset, page_size, page_size);

	// now for each tuple in copy page
	for(uint16_t index = 0; index <= tuple_count; index++)
	{
		if(exists_tuple_slotted_page(copy_page, page_size, tpl_d, index))
		{
			const void* tuple = get_nth_tuple_slotted_page(copy_page, page_size, tpl_d, index);

			// set index the offset to 0, marking it as deleted
			void* tuple_offset = page + get_offset_to_ith_tuple_offset(page, page_size, index);
			write_value_to_page(tuple_offset, page_size, 0);

			// now update the tuple at that index in the page
			update_tuple_slotted_page(page, page_size, tpl_d, index, tuple);
		}
	}

	free(copy_page);
}

uint32_t get_free_space_slotted_page(const void* page, uint32_t page_size)
{
	return get_offset_to_end_of_free_space(page, page_size) - get_offset_to_start_of_free_space(page, page_size);
}

uint32_t get_space_allotted_to_all_tuples_in_slotted_page(const void* page, uint32_t page_size)
{
	return page_size - get_offset_to_tuple_offsets(page, page_size);
}

void print_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	printf("SLOTTED PAGE :\n\n");

	uint32_t tup_count = get_tuple_count_slotted_page(page, page_size);
	printf("\tTuples :: (%u)\n", tup_count);

	for(uint16_t i = 0; i < tup_count; i++)
	{
		printf("\t\ttuple %u\n", i);
		if(exists_tuple_slotted_page(page, page_size, tpl_d, i))
		{
			const void* tuple = get_nth_tuple_slotted_page(page, page_size, tpl_d, i);
			uint32_t tuple_size = get_tuple_size(tpl_d, tuple);
			char* print_buffer = malloc(tuple_size + (tpl_d->element_count * 32));
			sprint_tuple(print_buffer, tuple, tpl_d);
			printf("\t\t\toffset[%lu] size(%u) :: %s\n\n", ((uintptr_t)(tuple - page)), tuple_size, print_buffer);
			free(print_buffer);
		}
		else
			printf("\t\t\t%s\n\n", "DELETED");
	}
	printf("\n\n\n");
}