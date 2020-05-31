#ifndef PAGE_OPERATIONS_H
#define PAGE_OPERATIONS_H

#include<cell_definition.h>

// updates *tuple pointer will point to first byte of the tuple of the next row in the given page
// retuns 0, if the *tuple alredy points to the last row of the page
int read_next_tuple(void** tuple, const void* page, unsigned int tuple_length, cell_definition def[]);

// returns 1, if a tuple gets written to the given position, and points to the 
// returns 0, if the page is full and the next write would not succeed [this does mean that the current write succeeded]
int write_next_tuple(void** tuple, void* page, unsigned int tuple_length, cell_definition def[]);

#endif