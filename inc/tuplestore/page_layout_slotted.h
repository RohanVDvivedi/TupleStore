#ifndef PAGE_LAYOUT_SLOTTED_H
#define PAGE_LAYOUT_SLOTTED_H

#include<tuplestore/tuple.h>

uint32_t get_minimum_page_size_for_slotted_page(uint32_t page_header_size, const tuple_size_def* tpl_sz_d, uint32_t tuple_count);

uint32_t get_maximum_tuple_count_slotted_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d);

uint32_t get_maximum_tuple_size_slotted_page(uint32_t page_header_size, uint32_t page_size);

int init_slotted_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_size_def* tpl_sz_d);

uint32_t get_tuple_count_slotted_page(const void* page, uint32_t page_size);

uint32_t get_tomb_stone_count_slotted_page(const void* page, uint32_t page_size);

int append_tuple_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple);

int can_append_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple);

int insert_tuple_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

int update_tuple_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

int can_update_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

int discard_tuple_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index);

void discard_all_tuples_slotted_page(void* page, uint32_t page_size);

uint32_t discard_trailing_tomb_stones_slotted_page(void* page, uint32_t page_size);

int exists_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index);

int swap_tuples_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t i1, uint32_t i2);

const void* get_nth_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index);

int set_element_in_tuple_in_place_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t tuple_index, positional_accessor element_index, const user_value* value);

int run_page_compaction_slotted_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, int* memory_allocation_error);

int zero_out_free_space_slotted_page(void* page, uint32_t page_size);

uint32_t get_free_space_slotted_page(const void* page, uint32_t page_size);

uint32_t get_space_occupied_by_tuples_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t start_index, uint32_t last_index);

uint32_t get_space_occupied_by_all_tuples_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

uint32_t get_space_occupied_by_all_tomb_stones_slotted_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

uint32_t get_space_to_be_occupied_by_tuple_slotted_page(uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple);

uint32_t get_space_allotted_to_all_tuples_slotted_page(const void* page, uint32_t page_size);

uint32_t get_space_to_be_allotted_to_all_tuples_slotted_page(uint32_t page_header_size, uint32_t page_size);

uint32_t get_additional_space_overhead_per_tuple_slotted_page(uint32_t page_size);

void print_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

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
**			uintN_t		space_occupied_by_tuples;
**
**			uintN_t 	tuple_count;
**
**			uintN_t 	tomb_stone_count;
**
**			uintN_t		end_of_free_space_offset;
**
**			uintN_t 	tuple_offsets [ tuple_count ];
**		}
**
**		here N can be 8, 16, 24 or 32
**
****************************************************************************************/