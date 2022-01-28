#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include<tuple_def.h>

#include<page_header.h>

typedef enum page_layout page_layout;
enum page_layout
{
	SLOTTED_PAGE_LAYOUT,
	FIXED_ARRAY_PAGE_LAYOUT,
};

// get the page layout that will be used by the functions below,
// for storing the tuple definition given in the parameter
page_layout get_page_layout_type(const tuple_def* tpl_d);

// INITIALIZATION FUNCTIONS

// this is the smallest sized page that is required to store a tuple_count number of tuples, each having a given tuple definition
uint32_t get_minimum_page_size(uint32_t page_header_size, const tuple_def* tpl_d, uint32_t tuple_count);

// initialize page to start using it
int init_page(void* page, uint32_t page_size, uint8_t page_header_size, const tuple_def* tpl_d);

// returns a value that equals free space when tuple count = 0 
uint32_t get_space_to_be_allotted_for_tuples(uint32_t page_size, uint32_t page_header_size, const tuple_def* tpl_d);


// DEBUG FUNCTIONS

// prints all page_reference_ids andd all tuples in the page including the deleted
void print_page(const void* page, uint32_t page_size, const tuple_def* tpl_d);

// prints page in hexadecimal format
void print_page_in_hex(const void* page, uint32_t page_size);

#endif