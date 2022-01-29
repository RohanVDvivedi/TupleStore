#include<page_layout.h>

#include<tuple.h>

#include<page_layout_slotted.h>
#include<page_layout_fixed_array.h>
#include<page_layout_util.h>

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

uint32_t get_tuple_count(const void* page, uint32_t page_size, const tuple_def* tpl_d)
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

int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

int can_insert_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return can_insert_tuple_slotted_page(page, page_size, tpl_d, external_tuple);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return can_insert_tuple_fixed_array_page(page, page_size, tpl_d);
	}
	return 0;
}

uint32_t insert_tuples_from_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* page_src, uint32_t start_index, uint32_t last_index)
{
	uint16_t tuple_count = get_tuple_count(page_src, page_size, tpl_d);

	// copy is not possible if
	// start_index is greater than last_index or the last_index in the tuple
	if((start_index > last_index) || (start_index >= tuple_count))
		return 0;

	if(last_index >= tuple_count)
		last_index = tuple_count - 1;

	uint16_t tuples_copied = 0;

	for(uint16_t index = start_index; index <= last_index; index++, tuples_copied++)
	{
		if(exists_tuple(page_src, page_size, tpl_d, index))
		{
			const void* tuple = get_nth_tuple(page_src, page_size, tpl_d, index);
			int inserted = insert_tuple(page, page_size, tpl_d, tuple);
			if(!inserted)
				break;
		}
	}

	return tuples_copied;
}

int update_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple);

int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
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

int delete_all_tuples(void* page, uint32_t page_size, const tuple_def* tpl_d)
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

int exists_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
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

int swap_tuples(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2)
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

const void* get_nth_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index)
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

uint32_t get_free_space(const void* page, uint32_t page_size, const tuple_def* tpl_d)
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

uint32_t get_space_occupied_by_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t start_index, uint32_t last_index)
{
	uint16_t tuple_count = get_tuple_count(page, page_size, tpl_d);
	if((start_index > last_index) || (last_index >= tuple_count))
		return 0;

	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
		{
			uint32_t tuples_total_size = 0;
			uint32_t tuples_total_offsets_size = 0;
			for(uint32_t i = start_index; i <= last_index; i++)
			{
				if(exists_tuple(page, page_size, tpl_d, i))
				{
					tuples_total_size += get_tuple_size(tpl_d, get_nth_tuple(page, page_size, tpl_d, i));
					tuples_total_offsets_size += get_value_size_on_page(page_size);
				}
			}
			return tuples_total_size + tuples_total_offsets_size;
		}
		case FIXED_ARRAY_PAGE_LAYOUT :
		{
			uint32_t tuples_existing = 0;
			for(uint32_t i = start_index; i <= last_index; i++)
			{
				if(exists_tuple(page, page_size, tpl_d, i))
					tuples_existing++;
			}
			return tuples_existing * tpl_d->size;
		}
	}
	return 0;
}

uint32_t get_space_occupied_by_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	if(get_tuple_count(page, page_size, tpl_d) == 0)
		return 0;
	return get_space_occupied_by_tuples(page, page_size, tpl_d, 0, get_tuple_count(page, page_size, tpl_d) - 1);
}

uint32_t get_space_allotted_to_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_allotted_to_all_tuples_in_slotted_page(page, page_size);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_allotted_to_all_tuples_in_fixed_array_page(page, page_size, tpl_d);
	}
	return 0;
}

uint32_t get_fragmentation_space(const void* page, uint32_t page_size, const tuple_def* tpl_d)
{
	return 	get_space_allotted_to_all_tuples(page, page_size, tpl_d)
		- ( get_space_occupied_by_all_tuples(page, page_size, tpl_d)
		+   get_free_space(page, page_size, tpl_d));
}

void print_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

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