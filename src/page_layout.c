#include<page_layout.h>

#include<tuple.h>

#include<page_layout_slotted.h>
#include<page_layout_fixed_array.h>
#include<page_layout_util.h>

#include<cutlery_stds.h>

// get page_layout to use for the given tuple definition
page_layout get_page_layout_type(const tuple_size_def* tpl_sz_d)
{
	if(is_variable_sized_tuple_size_def(tpl_sz_d))
		return SLOTTED_PAGE_LAYOUT;
	else
		return FIXED_ARRAY_PAGE_LAYOUT;
}

uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_size_def* tpl_sz_d, uint32_t tuple_count)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_minimum_page_size_for_slotted_page(page_header_size, tpl_sz_d, tuple_count);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_minimum_page_size_for_fixed_array_page(page_header_size, tpl_sz_d, tuple_count);
	}
	return 0;
}

uint32_t get_maximum_tuple_count_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	return get_space_to_be_allotted_to_all_tuples_on_page(page_header_size, page_size, tpl_sz_d) / (get_minimum_tuple_size_using_tuple_size_def(tpl_sz_d) + get_additional_space_overhead_per_tuple_on_page(page_size, tpl_sz_d));
}

int init_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return init_slotted_page(page, page_size, page_header_size, tpl_sz_d);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return init_fixed_array_page(page, page_size, page_header_size, tpl_sz_d);
	}
	return 0;
}

uint32_t get_tuple_count_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_tuple_count_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_tuple_count_fixed_array_page(page, page_size);
	}
	return 0;
}

uint32_t get_tomb_stone_count_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_tomb_stone_count_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_tomb_stone_count_fixed_array_page(page, page_size);
	}
	return 0;
}

int append_tuple_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return append_tuple_slotted_page(page, page_size, tpl_sz_d, external_tuple);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return append_tuple_fixed_array_page(page, page_size, tpl_sz_d, external_tuple);
	}
	return 0;
}

int can_append_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return can_append_tuple_slotted_page(page, page_size, tpl_sz_d, external_tuple);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return can_append_tuple_fixed_array_page(page, page_size, tpl_sz_d);
	}
	return 0;
}

int insert_tuple_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple)
{
	// a valid index is, 0 <= index <= tuple_count
	if(index > get_tuple_count_on_page(page, page_size, tpl_sz_d))
		return 0;

	// attempt an append and fail, if it fails
	if(!append_tuple_on_page(page, page_size, tpl_sz_d, external_tuple))
		return 0;

	// tuple always gets appended at the end of the page, on an append call
	// so swap it with its previous, until is it not at the right index
	for(uint32_t external_tuple_index = get_tuple_count_on_page(page, page_size, tpl_sz_d) - 1; external_tuple_index != index; external_tuple_index--)
		swap_tuples_on_page(page, page_size, tpl_sz_d, external_tuple_index, external_tuple_index - 1);

	return 1;
}

int can_insert_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple)
{
	// a valid index is, 0 <= index <= tuple_count
	return (index <= get_tuple_count_on_page(page, page_size, tpl_sz_d)) && can_append_tuple_on_page(page, page_size, tpl_sz_d, external_tuple);
}

int update_tuple_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return update_tuple_slotted_page(page, page_size, tpl_sz_d, index, external_tuple);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return update_tuple_fixed_array_page(page, page_size, tpl_sz_d, index, external_tuple);
	}
	return 0;
}

int can_update_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return can_update_tuple_slotted_page(page, page_size, tpl_sz_d, index, external_tuple);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return can_update_tuple_fixed_array_page(page, page_size, index);
	}
	return 0;
}

int discard_tuple_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return discard_tuple_slotted_page(page, page_size, tpl_sz_d, index);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return discard_tuple_fixed_array_page(page, page_size, tpl_sz_d, index);
	}
	return 0;
}

void discard_all_tuples_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			discard_all_tuples_slotted_page(page, page_size);
			return;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			discard_all_tuples_fixed_array_page(page, page_size);
			return;
		}
	}
}

uint32_t discard_trailing_tomb_stones_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return discard_trailing_tomb_stones_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return discard_trailing_tomb_stones_fixed_array_page(page, page_size);
	}
	return 0;
}

int exists_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return exists_tuple_slotted_page(page, page_size, tpl_sz_d, index);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return exists_tuple_fixed_array_page(page, page_size, tpl_sz_d, index);
	}
	return 0;
}

int swap_tuples_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t i1, uint32_t i2)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return swap_tuples_slotted_page(page, page_size, tpl_sz_d, i1, i2);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return swap_tuples_fixed_array_page(page, page_size, tpl_sz_d, i1, i2);
	}
	return 0;
}

const void* get_nth_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_nth_tuple_slotted_page(page, page_size, tpl_sz_d, index);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_nth_tuple_fixed_array_page(page, page_size, tpl_sz_d, index);
	}
	return 0;
}

int set_element_in_tuple_in_place_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t tuple_index, uint32_t element_index, const user_value* value)
{
	void* tuple_concerned = (void*) get_nth_tuple_on_page(page, page_size, &(tpl_d->size_def), tuple_index);

	// if the corresponding tuple does not exist, fail
	if(tuple_concerned == NULL)
		return 0;

	uint32_t new_tuple_size = 0;

	// fail if we can't set the tuple in the page
	if(!can_set_element_in_tuple(tpl_d, element_index, tuple_concerned, value, &new_tuple_size))
		return 0;

	// fail if new_tuple_size exceeds the tuple_size on the page
	if(new_tuple_size > get_tuple_size(tpl_d, tuple_concerned))
		return 0;

	// set the element
	return set_element_in_tuple(tpl_d, element_index, tuple_concerned, value);
}

void clone_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* page_src)
{
	// perform a plain copy that is easier and better
	memory_move(page, page_src, page_size);
}

int run_page_compaction(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, int* memory_allocation_error)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return run_page_compaction_slotted_page(page, page_size, tpl_sz_d, memory_allocation_error);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return run_page_compaction_fixed_array_page(page, page_size, tpl_sz_d, memory_allocation_error);
	}
	return 0;
}

uint32_t get_free_space_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_free_space_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_free_space_fixed_array_page(page, page_size, tpl_sz_d);
	}
	return 0;
}

uint32_t get_space_occupied_by_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t start_index, uint32_t last_index)
{
	uint32_t tuple_count = get_tuple_count_on_page(page, page_size, tpl_sz_d);
	if((start_index > last_index) || (last_index >= tuple_count))
		return 0;

	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_occupied_by_tuples_slotted_page(page, page_size, tpl_sz_d, start_index, last_index);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_occupied_by_tuples_fixed_array_page(page, page_size, tpl_sz_d, start_index, last_index);
	}
	return 0;
}

uint32_t get_space_occupied_by_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_occupied_by_all_tuples_slotted_page(page, page_size, tpl_sz_d);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_occupied_by_all_tuples_fixed_array_page(page, page_size, tpl_sz_d);
	}
	return 0;
}

uint32_t get_space_occupied_by_all_tomb_stones_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_occupied_by_all_tomb_stones_slotted_page(page, page_size, tpl_sz_d);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_occupied_by_all_tomb_stones_fixed_array_page(page, page_size, tpl_sz_d);
	}
	return 0;
}

uint32_t get_space_allotted_to_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_allotted_to_all_tuples_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_allotted_to_all_tuples_fixed_array_page(page, page_size, tpl_sz_d);
	}
	return 0;
}

uint32_t get_space_to_be_allotted_to_all_tuples_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_to_be_allotted_to_all_tuples_slotted_page(page_header_size, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_to_be_allotted_to_all_tuples_fixed_array_page(page_header_size, page_size, tpl_sz_d);
	}
	return 0;
}

uint32_t get_fragmentation_space_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	return 	get_space_allotted_to_all_tuples_on_page(page, page_size, tpl_sz_d)
		- (	  get_space_occupied_by_all_tuples_on_page(page, page_size, tpl_sz_d)
			+ get_free_space_on_page(page, page_size, tpl_sz_d)	);
}

uint32_t get_additional_space_overhead_per_tuple_on_page(uint32_t page_size, const tuple_size_def* tpl_sz_d)
{
	switch(get_page_layout_type(tpl_sz_d))
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
	printf("space allotted   = %"PRIu32"\n", get_space_allotted_to_all_tuples_on_page(page, page_size, &(tpl_d->size_def)));
	printf("space occupied   = %"PRIu32" (tomb_stones occupy %"PRIu32")\n", get_space_occupied_by_all_tuples_on_page(page, page_size, &(tpl_d->size_def)), get_space_occupied_by_all_tomb_stones_on_page(page, page_size, &(tpl_d->size_def)));
	printf("free space       = %"PRIu32"\n", get_free_space_on_page(page, page_size, &(tpl_d->size_def)));
	printf("fragmented space = %"PRIu32"\n", get_fragmentation_space_on_page(page, page_size, &(tpl_d->size_def)));
	printf("\n");
	printf("tuple_count      = %"PRIu32"\n", get_tuple_count_on_page(page, page_size, &(tpl_d->size_def)));
	printf("tomb_stone_count = %"PRIu32"\n", get_tomb_stone_count_on_page(page, page_size, &(tpl_d->size_def)));
	printf("\n");
	switch(get_page_layout_type(&(tpl_d->size_def)))
	{
		case SLOTTED_PAGE_LAYOUT :
			{print_slotted_page(page, page_size, tpl_d); return;}
		case FIXED_ARRAY_PAGE_LAYOUT :
			{print_fixed_array_page(page, page_size, tpl_d); return;}
	}
}

void print_page_in_hex(const void* page, uint32_t page_size)
{
	for(uint32_t i = 0; i < page_size; i++)
	{
		if(i % 8 == 0)
			printf("\n");
		printf("[%2d](%2x)%c \t ", i, 0xff & (*((char*)(page + i))), (*((char*)(page + i))));
	}
	printf("\n\n");
}