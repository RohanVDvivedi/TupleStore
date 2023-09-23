#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include<tuple_def.h>

typedef enum page_layout page_layout;
enum page_layout
{
	SLOTTED_PAGE_LAYOUT,
	FIXED_ARRAY_PAGE_LAYOUT,
};

// get the page layout that will be used by the functions below,
// for storing the tuple definition given in the parameter
page_layout get_page_layout_type(const tuple_def* tpl_d);


// PAGE_HEADER public functions they are defined in the page_header.c

// returns size of page header as stored on the page
uint32_t get_page_header_size(const void* page, uint32_t page_size);

// returns pointer to the page header on the page
void* get_page_header(void* page, uint32_t page_size);



// INITIALIZATION FUNCTIONS

// this is the smallest sized page that is required to store a tuple_count number of tuples, each having a given tuple definition
uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

// initialize page to start using it
int init_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d);

// returns the number of tuples in the page (including the deleted ones, i.e. including tomb stones)
uint32_t get_tuple_count_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// returns the number of deleted tuples (tomb stones) in the page
uint32_t get_tomb_stone_count_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);



// INSERT DELETE and GET functions for tuples in the page

// to append a tuple at the end in the given page, fails if the page is out of space
// if external_tuple is NULL, then a tombstone is appended
int append_tuple_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// returns 1, if the append_tuple would succeed
int can_append_tuple_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// update tuple at the specified index, fails if the page is out of space, or if the index is out of bounds i.e. when index >= get_tuple_count()
// if external_tuple is NULL, then a tombstone is placed at the given index
int update_tuple_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple);

// returns 1, if the update_tuple would succeed
int can_update_tuple_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple);

/*
** To be sure that a update must pass, if possible, try the following sequence instead
** can_update_tuple_on_page(page, index, NULL);
** run_page_compaction(page);
** can_update_tuple_on_page(page, index, external_tuple);
*/

// to discard a tuple (or a tombstane) at the given index in the page, fails if index >= get_tuple_count()
int discard_tuple_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

// discards all the tuple on the page
// for a slotted page it will also reset the end_of_free_space_offset
int discard_all_tuples_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d);

// discards trailing tombstones on the page
// it will return the number of trailing tombstones, that were discarded
uint32_t discard_trailing_tomb_stones_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d);

// to check if a tuple at the given index in the page exists
// returns 0, if the tuple was a tombstone OR if the index is out of bounds i.e. when index >= get_tuple_count())
int exists_tuple_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

// swap tuples (or tombstones) at given indices i1 and i2
// return 0, if the swap fails
int swap_tuples_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2);

// returns pointer to nth tuple in the page, else returns NULL if exist_tuple fails
const void* get_nth_tuple_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);



// CLONE FUNCTION

// creates a physical copy of page_src into page
void clone_page(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* page_src);



// PAGE COMPACTION removal functions

void run_page_compaction(void* page, uint32_t page_size, const tuple_def* tpl_d);



// SPACE queries

// returns total free space left inside a given page, that can be used to accomodate tuples
uint32_t get_free_space_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// returns space_occupied by tuples on the page from start_index to last_index, including the tomb_stones at those places
uint32_t get_space_occupied_by_tuples_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t start_index, uint32_t last_index);

// equivalent to get_space_occupied_by_tuples[0, tuple_count - 1)
uint32_t get_space_occupied_by_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// returns space_occupied by deleted tuples (i.e. tomb_stones) on the page from start_index to last_index
uint32_t get_space_occupied_by_all_tomb_stones_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// this is equivalent to free_space when the tuple_count = 0
uint32_t get_space_allotted_to_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// this is a version 2 of the above method, it must return the same result
uint32_t get_space_to_be_allotted_to_all_tuples_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_def* tpl_d);

// this is equivalent to get_space_allotted_to_all_tuples() - ( get_free_space_in_page() + get_space_occupied_by_all_tuples() )
uint32_t get_fragmentation_space_on_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// this the additional space in the "space_allotted_to_all_tuples" that will be used per tuple
// when you insert a tuple 
// the total space occupied by this tuple on the page is equal to the tuple_size + get_additional_space_overhead_per_tuple
uint32_t get_additional_space_overhead_per_tuple_on_page(uint32_t page_size, const tuple_def* tpl_d);



// DEBUG FUNCTIONS

// prints all page_reference_ids andd all tuples in the page including the deleted
void print_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// prints page in hexadecimal format
void print_page_in_hex(const void* page, uint32_t page_size);

#endif

/*
**
*****************************************************************************************
********			PAGE FORMATS
*****************************************************************************************
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
****************************************************************************************
**
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
*****************************************************************************************
**
*/