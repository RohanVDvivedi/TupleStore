#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include<stdint.h>

// NONE OF THE FOLLOWING FUNCTIONS CAN BE CALLED ON AN EMPTY TUPLE
// i.e. please check true for (0 == is_empty_tuple_def(const tuple_def* tpl_d))
// prior to calling any of the following functions below



// returns true if the given page has enough space to accomodate the given tuple
int can_page_accomodate_this_tuple();

// to insert a tuple at the given index in the page
int insert_tuple_in_page();

// to remove a tuple at the given index in the page
int remove_tuple_from_page();

// returns the index of the last tuple in the page
uint64_t get_last_tuple_index();



// ALL SEEK FUNCTIONS FAIL WITH NULL, WHEN THE INDEX OF THE TUPLE IS OUT_OF_BOUNDS 
// tuple is out of bounds, if (index < 0 || index > get_last_tuple_index())

// returns pointer to the first tuple in the page
void* seek_to_first_tuple();

// returns pointer to the next tuple in the page
// i.e. from nth tuple to n+1 th tuple in the page
void* seek_to_next_tuple();

// returns pointer to nth tuple in the page
void* seek_to_nth_tuple();

#endif