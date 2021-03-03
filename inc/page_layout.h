#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

// NONE OF THE FOLLOWING FUNCTIONS CAN BE CALLED ON AN EMPTY TUPLE
// i.e. please check true for (0 == is_empty_tuple_def(const tuple_def* tpl_d))
// prior to calling any of the following functions below

int insert_tuple_in_page();

int remove_tuple_from_page();

void* seek_to_first_tuple();

void* seek_to_next_tuple();

void* seek_to_nth_tuple();

#endif