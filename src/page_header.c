#include<page_header.h>

#include<page_layout_util.h>

uint32_t get_page_header_size(void* page, uint32_t page_size)
{
	return read_value_from_page(page + 0, page_size);
}

void* get_page_header(void* page, uint32_t page_size)
{
	// page header starts after its size on the page
	return page + get_value_size_on_page(page_size);
}

int init_page_header(void* page, uint32_t page_size, uint32_t page_header_size)
{
	if(page_size < get_value_size_on_page(page_size) + page_header_size)
		return 0;
	write_value_to_page(page + 0, page_size, page_header_size);
	return 1;
}

uint32_t get_offset_to_end_of_page_header(void* page, uint32_t page_size)
{
	return get_value_size_on_page(page_size) + get_page_header_size(page, page_size);
}

void* get_end_of_page_header(void* page, uint32_t page_size)
{
	return page + get_offset_to_end_of_page_header(page, page_size);
}