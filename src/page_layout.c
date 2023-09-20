#include<page_layout.h>

#include<tuple.h>

#include<page_layout_slotted.h>
#include<page_layout_fixed_array.h>
#include<page_layout_util.h>

#include<string.h>

// get page_layout to use for the given tuple definition
page_layout get_page_layout_type(const tuple_def* tpl_d)
{
	if(is_variable_sized_tuple_def(tpl_d))
		return SLOTTED_PAGE_LAYOUT;
	else
		return FIXED_ARRAY_PAGE_LAYOUT;
}

uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_minimum_page_size_for_slotted_page(page_header_size, tpl_d, tuple_count);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_minimum_page_size_for_fixed_array_page(page_header_size, tpl_d, tuple_count);
	}
	return 0;
}

int init_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return init_slotted_page(page, page_size, page_header_size, tpl_d);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return init_fixed_array_page(page, page_size, page_header_size, tpl_d);
	}
	return 0;
}

uint32_t get_tuple_count_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_tuple_count_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_tuple_count_fixed_array_page(page, page_size);
	}
	return 0;
}

uint32_t get_tomb_stone_count_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_tomb_stone_count_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_tomb_stone_count_fixed_array_page(page, page_size);
	}
	return 0;
}

int append_tuple_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return append_tuple_slotted_page(page, page_size, tpl_d, external_tuple);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return append_tuple_fixed_array_page(page, page_size, tpl_d, external_tuple);
	}
	return 0;
}

int can_append_tuple_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return can_append_tuple_slotted_page(page, page_size, tpl_d, external_tuple);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return can_append_tuple_fixed_array_page(page, page_size, tpl_d);
	}
	return 0;
}

uint32_t append_tuples_from_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* page_src, uint32_t start_index, uint32_t last_index)
{
	uint16_t tuple_count = get_tuple_count_on_page(page_src, page_size, tpl_d);

	// copy is not possible if
	// start_index is greater than last_index or the last_index in the tuple
	if((start_index > last_index) || (start_index >= tuple_count))
		return 0;

	if(last_index >= tuple_count)
		last_index = tuple_count - 1;

	uint16_t tuples_copied = 0;

	for(uint16_t index = start_index; index <= last_index; index++, tuples_copied++)
	{
		if(exists_tuple_on_page(page_src, page_size, tpl_d, index))
		{
			const void* tuple = get_nth_tuple_on_page(page_src, page_size, tpl_d, index);
			int inserted = append_tuple_on_page(page, page_size, tpl_d, tuple);
			if(!inserted)
				break;
		}
	}

	return tuples_copied;
}

int update_tuple_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return update_tuple_slotted_page(page, page_size, tpl_d, index, external_tuple);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return update_tuple_fixed_array_page(page, page_size, tpl_d, index, external_tuple);
	}
	return 0;
}

int delete_tuple_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return delete_tuple_slotted_page(page, page_size, tpl_d, index);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return delete_tuple_fixed_array_page(page, page_size, tpl_d, index);
	}
	return 0;
}

int delete_all_tuples_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return delete_all_tuples_slotted_page(page, page_size, tpl_d);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return delete_all_tuples_fixed_array_page(page, page_size, tpl_d);
	}
	return 0;
}

int exists_tuple_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return exists_tuple_slotted_page(page, page_size, tpl_d, index);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return exists_tuple_fixed_array_page(page, page_size, tpl_d, index);
	}
	return 0;
}

int swap_tuples_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return swap_tuples_slotted_page(page, page_size, tpl_d, i1, i2);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return swap_tuples_fixed_array_page(page, page_size, tpl_d, i1, i2);
	}
	return 0;
}

const void* get_nth_tuple_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_nth_tuple_slotted_page(page, page_size, tpl_d, index);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_nth_tuple_fixed_array_page(page, page_size, tpl_d, index);
	}
	return 0;
}

void clone_page(void* page, uint32_t page_size, const tuple_def* tpl_d, int discard_tomb_stones, const void* page_src)
{
	if(discard_tomb_stones)
	{
		uint32_t page_header_size = get_page_header_size(page_src, page_size);

		// intialize page
		init_page(page, page_size, page_header_size, tpl_d);

		// copy header of the page
		memmove(get_page_header(page, page_size), (const void*)get_page_header((void*)page_src, page_size), page_header_size);

		uint32_t tuple_count = get_tuple_count_on_page(page_src, page_size, tpl_d);

		// insert all tuples from page_src to page
		if(tuple_count > 0)
			append_tuples_from_page(page, page_size, tpl_d, page_src, 0, tuple_count - 1);
	}
	else
	{
		// perform a plain copy that is easier and better
		memmove(page, page_src, page_size);
	}
}

void run_page_compaction(void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			{run_page_compaction_slotted_page(page, page_size, tpl_d); return;}
		case FIXED_ARRAY_PAGE_LAYOUT :
			{run_page_compaction_fixed_array_page(page, page_size, tpl_d); return;}
	}
}

uint32_t get_free_space_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_free_space_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_free_space_fixed_array_page(page, page_size, tpl_d);
	}
	return 0;
}

uint32_t get_space_occupied_by_tuples_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t start_index, uint32_t last_index)
{
	uint16_t tuple_count = get_tuple_count_on_page(page, page_size, tpl_d);
	if((start_index > last_index) || (last_index >= tuple_count))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_occupied_by_tuples_slotted_page(page, page_size, tpl_d, start_index, last_index);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_occupied_by_tuples_fixed_array_page(page, page_size, tpl_d, start_index, last_index);
	}
	return 0;
}

uint32_t get_space_occupied_by_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_occupied_by_all_tuples_slotted_page(page, page_size, tpl_d);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_occupied_by_all_tuples_fixed_array_page(page, page_size, tpl_d);
	}
	return 0;
}

uint32_t get_space_occupied_by_all_tomb_stones_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_occupied_by_all_tomb_stones_slotted_page(page, page_size, tpl_d);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_occupied_by_all_tomb_stones_fixed_array_page(page, page_size, tpl_d);
	}
	return 0;
}

uint32_t get_space_allotted_to_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_allotted_to_all_tuples_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_allotted_to_all_tuples_fixed_array_page(page, page_size, tpl_d);
	}
	return 0;
}

uint32_t get_space_to_be_allotted_to_all_tuples_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_to_be_allotted_to_all_tuples_slotted_page(page_header_size, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_to_be_allotted_to_all_tuples_fixed_array_page(page_header_size, page_size, tpl_d);
	}
	return 0;
}

uint32_t get_fragmentation_space_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return 	get_space_allotted_to_all_tuples_on_page(page, page_size, tpl_d)
		- (	  get_space_occupied_by_all_tuples_on_page(page, page_size, tpl_d)
			+ get_free_space_on_page(page, page_size, tpl_d)	);
}

uint32_t get_additional_space_overhead_per_tuple_on_page(uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_additional_space_overhead_per_tuple_slotted_page(page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_additional_space_overhead_per_tuple_fixed_array_page();
	}
	return 0;
}

void print_page(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	printf("PAGE : \n");
	printf("space allotted   = %"PRIu32"\n", get_space_allotted_to_all_tuples_on_page(page, page_size, tpl_d));
	printf("space occupied   = %"PRIu32" (tomb_stones occupy %"PRIu32")\n", get_space_occupied_by_all_tuples_on_page(page, page_size, tpl_d), get_space_occupied_by_all_tomb_stones_on_page(page, page_size, tpl_d));
	printf("free space       = %"PRIu32"\n", get_free_space_on_page(page, page_size, tpl_d));
	printf("fragmented space = %"PRIu32"\n", get_fragmentation_space_on_page(page, page_size, tpl_d));
	printf("\n");
	printf("tuple_count      = %"PRIu32"\n", get_tuple_count_on_page(page, page_size, tpl_d));
	printf("tomb_stone_count = %"PRIu32"\n", get_tomb_stone_count_on_page(page, page_size, tpl_d));
	printf("\n");
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			{print_slotted_page(page, page_size, tpl_d); return;}
		case FIXED_ARRAY_PAGE_LAYOUT :
			{print_fixed_array_page(page, page_size, tpl_d); return;}
	}
}

void print_page_in_hex(const void* page, uint32_t page_size)
{
	for(int i = 0; i < page_size; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");
}