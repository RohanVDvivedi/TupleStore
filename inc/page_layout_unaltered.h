#ifndef PAGE_LAYOUT_UNALTERED_H
#define PAGE_LAYOUT_UNALTERED_H

/*
  The below declarations of the functions are one ones, then do not modify the contents of the page
*/

#include<tuple_def.h>
#include<page_layout_enum.h>


// get the page layout that will be used by the functions below,
// for storing the tuple definition given in the parameter
page_layout get_page_layout_type(const tuple_size_def* tpl_sz_d);

// readonly version of get_page_header
const void* get_page_header_ua(const void* page, uint32_t page_size);



// PAGE_HEADER public functions they are defined in the page_header.c

// returns size of page header as stored on the page
uint32_t get_page_header_size(const void* page, uint32_t page_size);



// BASIC FUNCTIONS

// this is the smallest sized page that is required to store a tuple_count number of tuples, each having a given tuple definition
uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_size_def* tpl_sz_d, uint32_t tuple_count);

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

// this is equivalent to free_space when the tuple_count = 0
uint32_t get_space_allotted_to_all_tuples_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this is a version 2 of the above method, it must return the same result
uint32_t get_space_to_be_allotted_to_all_tuples_on_page(uint32_t page_header_size, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this is equivalent to get_space_allotted_to_all_tuples() - ( get_free_space_in_page() + get_space_occupied_by_all_tuples() )
uint32_t get_fragmentation_space_on_page(const void* page, uint32_t page_size, const tuple_size_def* tpl_sz_d);

// this the additional space in the "space_allotted_to_all_tuples" that will be used per tuple
// when you insert a tuple 
// the total space occupied by this tuple on the page is equal to the tuple_size + get_additional_space_overhead_per_tuple
uint32_t get_additional_space_overhead_per_tuple_on_page(uint32_t page_size, const tuple_size_def* tpl_sz_d);



// DEBUG FUNCTIONS

// prints all space composition variables, page_reference_ids and all tuples in the page including the tombstones, all in order
void print_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// prints page in hexadecimal format
void print_page_in_hex(const void* page, uint32_t page_size);

#endif