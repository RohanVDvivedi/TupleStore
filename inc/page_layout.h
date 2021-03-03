#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include<stdint.h>

// NONE OF THE FOLLOWING FUNCTIONS CAN BE CALLED ON AN EMPTY TUPLE
// i.e. please check true for (0 == is_empty_tuple_def(const tuple_def* tpl_d))
// prior to calling any of the following functions below



// to insert a tuple at the given index in the page
int insert_tuple(void* page, uint64_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// to remove a tuple at the given index in the page
int delete_tuple(void* page, uint64_t page_size, const tuple_def* tpl_d, uint64_t index);

// to check if a tuple at the given index in the page has been deleted
int is_deleted_tuple(const void* page, uint64_t page_size, const tuple_def* tpl_d, uint64_t index);



// returns the index of the last tuple in the page
uint64_t get_last_tuple_index(const void* page, uint64_t page_size, const tuple_def* tpl_d);




// SEEK FUNCTIONS FAIL WITH NULL, WHEN THE INDEX OF THE TUPLE IS OUT_OF_BOUNDS 
// tuple is out of bounds, if (index > get_last_tuple_index())

// returns pointer to nth tuple in the page
void* seek_to_nth_tuple(const void* page, uint64_t page_size, const tuple_def* tpl_d, uint64_t index);




// returns true if the given page has enough space to accomodate the given tuple
int can_accomodate_tuple(const void* page, uint64_t page_size, const tuple_def* tpl_d, const void* external_tuple);

// returns total free space inside a given page
// this is the space that will be freed if the page was to be compacted thoroughly
uint64_t get_free_space(const void* page, uint64_t page_size, const tuple_def* tpl_d);




// for a slotted page, compaction of the page is necessary to increase the usable area
// it involves
//    * reducing the extra area given to the VARIABLE_SIZED elements in the tuple
//    * removing tomb stones for deleted records and their records space in both VARIABLE_SIZED and fixed sized tuples
int compact_page(const void* page, uint64_t page_size, const tuple_def* tpl_d);



// prints all tuples in the page including the deleted ones with their tomb stones
void print_all_tuples(const void* page, uint64_t page_size, const tuple_def* tpl_d);

#endif