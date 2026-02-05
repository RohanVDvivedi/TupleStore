#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include<tuplestore/tuple.h>
#include<tuplestore/page_layout_enum.h>

// get the page layout that will be used by the functions below,
// for storing the tuple definition given in the parameter
page_layout get_page_layout_type(const tuple_size_def* tpl_sz_d);



// PAGE_HEADER public functions they are defined in the page_header.c

// this is the total number of bytes that are required on the page (of given page_size) to store the page header (of given page_header_size)
// this space includes the page_header_size and its prefix that stores the size of the page_header
uint32_t get_space_required_for_page_header(uint32_t page_header_size, uint32_t page_size);

// it just returns page_size >= get_space_required_for_page_header(page_header_size, page_size)
int can_page_header_fit_on_page(uint32_t page_header_size, uint32_t page_size);

// returns size of page header as stored on the page
uint32_t get_page_header_size(const void* page, uint32_t page_size);

// returns pointer to the page header on the page
void* get_page_header(void* page, uint32_t page_size);

// same as above function, but it returns a readonly pointer to the page_header
const void* get_page_header_ua(const void* page, uint32_t page_size);



// INITIALIZATION and BASIC FUNCTIONS

// this is the smallest sized page that is required to store a tuple_count number of tuples, each having a given tuple definition
// this is an overflow unsafe function, please besure to limit tuple_count to range [0, (2GB / min_tuple_size)], 1 or 2 would be still an ideal candidate for most use cases
uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_size_def* tpl_sz_d, uint32_t tuple_count);

// returns maximum tuple_count possible on a page
uint32_t get_maximum_tuple_count_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns maximum size of the tuple that can be accomodated on these pages
// the returned value may not be the actual possible size of the tuple, it just gives you an estimate of what largest tuple accomodatable on this page of given page_header_size and page_size
uint32_t get_maximum_tuple_size_accomodatable_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// initialize page to start using it
int init_page(void* page, uint32_t page_size, uint32_t page_header_size, const tuple_size_def* tpl_sz_d);

// returns the number of tuples in the page (including the deleted ones, i.e. including tomb stones)
uint32_t get_tuple_count_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns the number of deleted tuples (tomb stones) in the page
uint32_t get_tomb_stone_count_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);



// INSERT UPDATE and GET functions for tuples in the page

// to append a tuple at the end in the given page, fails if the page is out of space
// if external_tuple is NULL, then a tombstone is appended
int append_tuple_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple);

// returns 1, if the append_tuple would succeed
int can_append_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple);

// inserts tuple on the page, at the given index, fails if the page is out of space, or if the index is out of bounds i.e. when index > get_tuple_count()
// index to all the tuples and tombstones at indices >= index, will increment by 1
int insert_tuple_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

// synonym of can_append_tuple_on_page, with valid index
int can_insert_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

// update tuple at the specified index, fails if the page is out of space, or if the index is out of bounds i.e. when index >= get_tuple_count()
// if external_tuple is NULL, then a tombstone is placed at the given index
int update_tuple_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

// returns 1, if the update_tuple would succeed
int can_update_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

/*
** To be sure that a update must pass, if possible, try the following sequence instead
** can_update_tuple_on_page(page, index, NULL);
** run_page_compaction(page);
** can_update_tuple_on_page(page, index, external_tuple);
*/

// to discard a tuple (or a tombstone) at the given index in the page, fails if index >= get_tuple_count()
// index to all the tuples and tombstones greater than index will decrement by 1
int discard_tuple_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index);

// discards all the tuple on the page
// for a slotted page it will also reset the end_of_free_space_offset
void discard_all_tuples_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// discards trailing tombstones on the page
// it will return the number of trailing tombstones, that were discarded
uint32_t discard_trailing_tomb_stones_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns number of trailing tomb stones on the page
uint32_t get_trailing_tomb_stones_count_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// to check if a tuple at the given index in the page exists
// returns 0, if the tuple was a tombstone OR if the index is out of bounds i.e. when index >= get_tuple_count())
int exists_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index);

// swap tuples (or tombstones) at given indices i1 and i2
// return 0, if the swap fails
int swap_tuples_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t i1, uint32_t i2);

// returns pointer to nth tuple in the page, else returns NULL if exist_tuple fails
const void* get_nth_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index);



// UPDATE ELEMENT OF TUPLE IN PLACE FUNCTION

// updates an element (at index element_index) in tuple on the page (at index element_index) in place
// this function fails if the given update will increase the tuple_size on the page
// use this generally -> for a fixed sized element update in the tuple OR when you know that the updated variable sized value is smaller than the element in the tuple on page
int set_element_in_tuple_in_place_on_page(void* page, uint32_t page_size, const tuple_def* tpl_d, uint32_t tuple_index, positional_accessor element_index, const datum* value);



// CLONE FUNCTION

// creates a physical copy of page_src into page
void clone_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* page_src);



// PAGE COMPACTION functions (this function will not change the logical contents of the page, it will just defragment the page, to make fragmented space as the free space)

// retuns 1, if the page was compacted, by moving tuples around
int run_page_compaction(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, int* memory_allocation_error);
// the above function is prone to memory_allocation_error, i.e. when malloc fails
// on such an error, return will be 0 implying no page_compaction done and memory_allocation_error set to 1

// returns 1, if there were any bytes that were free and were zeroed out
// returns 0, if there was no free space on the page
// it only zeros out the free space on the page and does not touch fragmented space on the page
int zero_out_free_space_on_page(void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);



// SPACE queries

// returns total free space left inside a given page in its free space area, it may shrink as result of inserts, appends and updates
uint32_t get_free_space_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns space_occupied by tuples on the page from start_index to last_index, including the space occupied by tomb_stones at those places. tomb_stones are just NULL tuples
uint32_t get_space_occupied_by_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t start_index, uint32_t last_index);

// equivalent to get_space_occupied_by_tuples[0, tuple_count - 1)
uint32_t get_space_occupied_by_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns space_occupied by deleted tuples (i.e. tomb_stones) on the page
uint32_t get_space_occupied_by_all_tomb_stones_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this is the space that this external_tuple(OR a tomb_stone (if external_tuple = NULL)) will occupy (OR is occupying, if external_tuple is on the page) on the page
// this includes the space required by tuple data and the additional space required for space management, for this tuple on the page
uint32_t get_space_to_be_occupied_by_tuple_on_page(uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple);

// this is equivalent to free_space when the tuple_count = 0
uint32_t get_space_allotted_to_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this is a version 2 of the above method, it must return the same result
uint32_t get_space_to_be_allotted_to_all_tuples_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this is equivalent to get_space_allotted_to_all_tuples_on_page() - ( get_free_space_on_page() + get_space_occupied_by_all_tuples_on_page() )
uint32_t get_fragmentation_space_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// space_allotted_to_all_tuples = space_occupied_by_all_tuples + free_space + fragmented_space

// this the additional space in the "space_allotted_to_all_tuples" that will be used per tuple
// the total space occupied by a (non tomb_stone) tuple on the page is equal to the tuple_size (when tuple is not a tomb_stone) + get_additional_space_overhead_per_tuple
uint32_t get_additional_space_overhead_per_tuple_on_page(uint32_t page_size, const tuple_size_def* tpl_sz_d);

// for a tuple (non tomb_stone) => space_occupied_by_tuple = tuple_size + additional_space_overhead



// DEBUG FUNCTIONS

// prints all space composition variables, page_reference_ids and all tuples in the page including the tombstones, all in order
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