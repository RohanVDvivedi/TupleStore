#include<page_layout.h>

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
			return get_minimum_page_size_slotted_page(page_header_size, tpl_d, tuple_count);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_minimum_page_size_fixed_array_page(page_header_size, tpl_d, tuple_count);
	}
	return 0;
}

int init_page(void* page, uint32_t page_size, uint8_t page_header_size, const tuple_def* tpl_d)
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

uint32_t get_space_to_be_allotted_for_tuples(uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d)
{
	switch(get_page_layout_type(tpl_d))
	{
		case SLOTTED_PAGE_LAYOUT :
			return get_space_to_be_allotted_for_tuples_slotted_page(page_size, page_header_size, tpl_d);
		case FIXED_ARRAY_PAGE_LAYOUT :
			return get_space_to_be_allotted_for_tuples_fixed_array_page(page_size, page_header_size, tpl_d);
	}
	return 0;
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