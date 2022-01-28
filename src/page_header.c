#include<page_header.h>

#include<page_layout_util.h>

uint32_t get_page_header_size(void* page, uint32_t page_size)
{
	return read_value_from_page(page, page_size);
}

void* get_page_header(void* page, uint32_t page_size)
{
	// page header starts after its size on the page
	return page + get_value_size_on_page(page_size);
}

void* get_end_of_page_header(void* page, uint32_t page_size)
{
	return get_page_header(page, page_size) + get_page_header_size(page, page_size);
}