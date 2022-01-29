#ifndef PAGE_LAYOUT_SLOTTED_H
#define PAGE_LAYOUT_SLOTTED_H

#include<tuple_def.h>

uint32_t get_minimum_page_size_for_slotted_page(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

int init_slotted_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d);

uint32_t get_tuple_count_slotted_page(const void* page, uint32_t page_size);

int insert_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

int can_insert_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

int update_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple);

int delete_tuple_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

int delete_all_tuples_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d);

int exists_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

int swap_tuples_slotted_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2);

const void* get_nth_tuple_slotted_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

uint32_t get_free_space_slotted_page(const void* page, uint32_t page_size);

uint32_t get_space_allotted_to_all_tuples_in_slotted_page(const void* page, uint32_t page_size);

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