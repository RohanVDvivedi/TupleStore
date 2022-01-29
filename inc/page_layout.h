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


// PAHE_HEADER public functions they are defined in the page_header.c

// returns size of page header as stored on the page
uint32_t get_page_header_size(void* page, uint32_t page_size);

// returns pointer to the page header on the page
void* get_page_header(void* page, uint32_t page_size);



// INITIALIZATION FUNCTIONS

// this is the smallest sized page that is required to store a tuple_count number of tuples, each having a given tuple definition
uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

// initialize page to start using it
int init_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d);

// returns the number of tuples in the page (including the deleted ones)
uint32_t get_tuple_count(const void* page, uint32_t page_size, const tuple_def* tpl_d);



// INSERT DELETE and GET functions for tuples in the page

// to insert a tuple at the end in the given page, fails if the page is out of space
int insert_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// returns 1, if the insert_tuple would succeed
int can_insert_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// inserts tuples from page_src starting with start_index and until end_index (or its tuple_count - 1), 
uint32_t insert_tuples_from_page(void* page, uint32_t page_size, const tuple_def* def, const void* page_src, uint32_t start_index, uint32_t last_index, int ignore_deleted);

// update tuple at the specified index, fails if the page is out of space, or if the index is out of bounds i.e. when index >= get_tuple_count()
int update_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index, const void* external_tuple);

// to delete a tuple at the given index in the page, fails if index >= get_tuple_count()
int delete_tuple(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

// deletes all the tuple in the page
// for a slotted page it will also reset the end_of_free_space_offset
int delete_all_tuples(void* page, uint32_t page_size, const tuple_def* tpl_d);

// to check if a tuple at the given index in the page exists
// returns 0, if the tuple was deleted OR if the index is out of bounds i.e. when index >= get_tuple_count())
int exists_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);

// swap tuples at given indices i1 and i2
// return 0, if the swap fails
int swap_tuples(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t i1, uint32_t i2);

// returns pointer to nth tuple in the page, else returns NULL if exist_tuple fails
const void* get_nth_tuple(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t index);



// PAGE COMPACTION and TOMB STONES removal functions

// returns 1, if the page_compaction is not a NOOP
// this function is a NOOP for FIXED_ARRAY_PAGE_LAYOUT
void run_page_compaction(void* page, uint32_t page_size, const tuple_def* tpl_d);

// discards all tomb stones on the page
void discard_all_deleted_tuples(void* page, uint32_t page_size, const tuple_def* tpl_d);



// SPACE queries

// returns total free space left inside a given page, that can be used to accomodate tuples
uint32_t get_free_space(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// returns space_occupied by tuples on the page from start_index to last_index
uint32_t get_space_occupied_by_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t start_index, uint32_t last_index);

// equivalent to get_space_occupied_by_tuples[0, tuple_count - 1)
uint32_t get_space_occupied_by_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// this is equivalent to free_space when the tuple_count = 0
uint32_t get_space_allotted_to_all_tuples(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// this is equivalent to get_space_allotted_to_all_tuples() - ( get_free_space_in_page() + get_space_occupied_by_all_tuples() )
uint32_t get_fragmentation_space_in_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);



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
**			uintN_t 	tuple_count;
**
**			uintN_t		end_of_free_space_offset;
**
**			uintN_t 	tuple_offsets [ tuple_count ];
**		}
**
**		here N can be 8, 16 or 32
**
****************************************************************************************
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
*****************************************************************************************
**
*/