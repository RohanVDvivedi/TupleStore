#ifndef PAGE_LAYOUT_FIXED_ARRAY_H
#define PAGE_LAYOUT_FIXED_ARRAY_H

#include<tuple_def.h>

uint32_t get_minimum_page_size_for_fixed_array_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

int init_fixed_array_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d);

uint32_t get_tuple_count_fixed_array_page(const void* page, uint32_t page_size);

uint32_t get_tomb_stone_count_fixed_array_page(const void* page, uint32_t page_size);

int append_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

int can_append_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

int update_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple);

int can_update_tuple_fixed_array_page(const void* page, uint32_t page_size, uint32_t index);

int discard_tuple_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

int discard_all_tuples_fixed_array_page(void* page, uint32_t page_size);

uint32_t discard_trailing_tomb_stones_fixed_array_page(void* page, uint32_t page_size);

int exists_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

int swap_tuples_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2);

const void* get_nth_tuple_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

void run_page_compaction_fixed_array_page(void* page, uint32_t page_size, const tuple_def* tpl_d);

uint32_t get_free_space_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

uint32_t get_space_occupied_by_tuples_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t start_index, uint32_t last_index);

uint32_t get_space_occupied_by_all_tuples_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

uint32_t get_space_occupied_by_all_tomb_stones_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

uint32_t get_space_allotted_to_all_tuples_fixed_array_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

uint32_t get_space_to_be_allotted_to_all_tuples_fixed_array_page(uint32_t page_header_size, uint32_t page_size, const tuple_def* tpl_d);

uint32_t get_additional_space_overhead_per_tuple_fixed_array_page();

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
**			uintN_t 	tomb_stone_count;
**
**			char	 	is_valid_bitmap [ ceil_function( tuple_capacity / 8 ) ] ;
**
**			< tuples [ tuple_count ]; >
**		}
**
**		here N can be 8, 16, 24 or 32
**
*****************************************************************************************/