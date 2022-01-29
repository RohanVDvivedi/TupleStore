#ifndef PAGE_LAYOUT_FIXED_ARRAY_H
#define PAGE_LAYOUT_FIXED_ARRAY_H

#include<tuple_def.h>

uint32_t get_minimum_page_size_for_fixed_array_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

int init_fixed_array_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d);

uint32_t get_tuple_count_fixed_array_page(const void* page, uint32_t page_size);

int insert_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

int can_insert_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

int update_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple);

int delete_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

int delete_all_tuples_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d);

int exists_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

int swap_tuples_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2);

const void* get_nth_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

uint32_t get_free_space_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

uint32_t get_space_allotted_to_all_tuples_in_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

void print_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

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