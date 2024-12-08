#ifndef PAGE_HEADER_UTIL_H
#define PAGE_HEADER_UTIL_H

#include<page_layout_util.h>

static inline uint32_t get_page_header_size_INLINE(const void* page, uint32_t page_size)
{
	return read_value_from_page(page + 0, page_size);
}

// returns offset to the end of page_header in the given page
static inline uint32_t get_offset_to_end_of_page_header(const void* page, uint32_t page_size)
{
	return get_value_size_on_page(page_size) + get_page_header_size_INLINE(page, page_size);
}

#endif