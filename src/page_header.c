#include<page_header.h>

#include<page_layout_util.h>

#include<cutlery_stds.h>

uint32_t get_space_required_for_page_header(uint32_t page_header_size, uint32_t page_size)
{
	return get_value_size_on_page(page_size) + page_header_size;
}

int can_page_header_fit_on_page(uint32_t page_header_size, uint32_t page_size)
{
	return page_size >= get_space_required_for_page_header(page_header_size, page_size);
}

uint32_t get_page_header_size(const void* page, uint32_t page_size)
{
	return get_page_header_size_INLINE(page, page_size);
}

void* get_page_header(void* page, uint32_t page_size)
{
	// page header starts after its size on the page
	return page + get_value_size_on_page(page_size);
}

const void* get_page_header_ua(const void* page, uint32_t page_size)
{
	// page header starts after its size on the page
	return page + get_value_size_on_page(page_size);
}

int init_page_header(void* page, uint32_t page_size, uint32_t page_header_size)
{
	if(page_size < get_value_size_on_page(page_size) + page_header_size)
		return 0;

	// write page_header size to page
	write_value_to_page(page + 0, page_size, page_header_size);

	// memset the header to all 0 bytes
	memory_set(page + get_value_size_on_page(page_size), 0, page_header_size);

	return 1;
}

void* get_end_of_page_header(void* page, uint32_t page_size)
{
	return page + get_offset_to_end_of_page_header(page, page_size);
}