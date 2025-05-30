#ifndef PAGE_LAYOUT_UNALTERED_H
#define PAGE_LAYOUT_UNALTERED_H

/*
  The below declarations of the functions are one ones, then do not modify the contents of the page
*/

#include<tuplestore/tuple_def.h>
#include<tuplestore/page_layout_enum.h>


// get the page layout that will be used by the functions below,
// for storing the tuple definition given in the parameter
page_layout get_page_layout_type(const tuple_size_def* tpl_sz_d);

// readonly version of get_page_header
const void* get_page_header_ua(const void* page, uint32_t page_size);



// PAGE_HEADER public functions they are defined in the page_header.c

// this is the total number of bytes that are required on the page (of given page_size) to store the page header (of given page_header_size)
// this space includes the page_header_size and its prefix that stores the size of the page_header
uint32_t get_space_required_for_page_header(uint32_t page_header_size, uint32_t page_size);

// it just returns page_size >= get_space_required_for_page_header(page_header_size, page_size)
int can_page_header_fit_on_page(uint32_t page_header_size, uint32_t page_size);

// returns size of page header as stored on the page
uint32_t get_page_header_size(const void* page, uint32_t page_size);



// BASIC FUNCTIONS

// this is the smallest sized page that is required to store a tuple_count number of tuples, each having a given tuple definition
uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_size_def* tpl_sz_d, uint32_t tuple_count);

// returns maximum tuple_count possible on a page
uint32_t get_maximum_tuple_count_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns maximum size of the tuple that can be accomodated on these pages
// the returned value may not be the actual possible size of the tuple, it just gives you an estimate of what largest tuple accomodatable on this page of given page_header_size and page_size
uint32_t get_maximum_tuple_size_accomodatable_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns the number of tuples in the page (including the deleted ones, i.e. including tomb stones)
uint32_t get_tuple_count_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns the number of deleted tuples (tomb stones) in the page
uint32_t get_tomb_stone_count_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);



// check for INSERT, check for UPDATE and GET functions for tuples in the page

// returns 1, if the append_tuple would succeed
int can_append_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple);

// returns 1, if the insert_tuple would succeed
int can_insert_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

// returns 1, if the update_tuple would succeed
int can_update_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index, const void* external_tuple);

// to check if a tuple at the given index in the page exists
// returns 0, if the tuple was a tombstone OR if the index is out of bounds i.e. when index >= get_tuple_count())
int exists_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index);

// returns pointer to nth tuple in the page, else returns NULL if exist_tuple fails
const void* get_nth_tuple_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t index);



// SPACE queries

// returns total free space left inside a given page, that can be used to accomodate tuples
uint32_t get_free_space_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns space_occupied by tuples on the page from start_index to last_index, including the tomb_stones at those places
uint32_t get_space_occupied_by_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d, uint32_t start_index, uint32_t last_index);

// equivalent to get_space_occupied_by_tuples[0, tuple_count - 1)
uint32_t get_space_occupied_by_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// returns space_occupied by deleted tuples (i.e. tomb_stones) on the page from start_index to last_index
uint32_t get_space_occupied_by_all_tomb_stones_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this is the space that this external_tuple(OR a tomb_stone (if external_tuple = NULL)) will occupy (OR is occupying, if external_tuple is on the page) on the page
// this includes the space required by tuple data and the additional space required for space management, for this tuple on the page
uint32_t get_space_to_be_occupied_by_tuple_on_page(uint32_t page_size, const tuple_size_def* tpl_sz_d, const void* external_tuple);

// this is equivalent to free_space when the tuple_count = 0
uint32_t get_space_allotted_to_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this is a version 2 of the above method, it must return the same result
uint32_t get_space_to_be_allotted_to_all_tuples_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this is equivalent to get_space_allotted_to_all_tuples() - ( get_free_space_in_page() + get_space_occupied_by_all_tuples() )
uint32_t get_fragmentation_space_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

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