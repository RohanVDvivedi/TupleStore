#ifndef PAGE_READER_H
#define PAGE_READER_H

#include<string.h>
#include<page_hdr.h>
#include<data_access_methods.h>

// retuns pointer to tuple at index tuple_no in the page
void* get_tuple(const data_access_methods* dam, void* page, uint16_t tuple_no);

// returns the number of tuples that were appended
uint16_t append_tuple(const data_access_methods* dam, void* page, void* tuples_to_insert, uint16_t num_tuples_to_insert);

#endif