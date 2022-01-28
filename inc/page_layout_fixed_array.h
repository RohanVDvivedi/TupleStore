#ifndef PAGE_LAYOUT_FIXED_ARRAY_H
#define PAGE_LAYOUT_FIXED_ARRAY_H

#include<tuple_def.h>

uint32_t get_minimum_page_size_for_fixed_array_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

int init_fixed_array_page(void* page, uint32_t page_size, uint8_t page_header_size, const tuple_def* tpl_d);

#endif

/****************************************************************************************
**
**					FIXED_ARRAY PAGE
**
**		tuple_capacity = (page_size - page_header_size - N) / (tuple_def->size * 8 + 1)
**
**		struct page_of_FIXED_ARRAY_PAGE
**		{
**			uintN_t 	page_header_size;
**
**			char		page_header[ page_header_size ];
**
**			uintN_t 	tuple_count;
**
**			char	 	is_valid_bitmap [ ceil_function( tuple_capacity / 8 ) ] ;
**
**			< tuples [ tuple_count ]; >
**		}
**
**
*****************************************************************************************/