#ifndef PAGE_LAYOUT_SLOTTED_H
#define PAGE_LAYOUT_SLOTTED_H

#include<tuple_def.h>

uint32_t get_minimum_page_size_for_slotted_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

int init_slotted_page(void* page, uint32_t page_size, uint8_t page_header_size, const tuple_def* tpl_d);

uint32_t get_tuple_count_slotted_page(const void* page, uint32_t page_size);

#endif

/*****************************************************************************************
**
**					SLOTTED PAGE
**
**		* CASE ::: is_variable_sized_tuple_def(tuple_definition) = 1
**
**		
**		struct page_of_SLOTTED_PAGE
**		{
**			uintN_t 	page_header_size;
**
**			char 		page_header[ page_header_size ];
**
**			uintN_t 	tuple_count;
**
**			uintN_t		end_of_free_space_offset;
**
**			uintN_t 	tuple_offsets [ tuple_count ];
**		}
**
**		here N can be 8, 16 or 32
**
****************************************************************************************/