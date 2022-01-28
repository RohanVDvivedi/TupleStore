#ifndef PAGE_HEADER_H
#define PAGE_HEADER_H

#include<stdint.h>

// returns size of page header as stored on the page
uint32_t get_page_header_size(void* page, uint32_t page_size);

// returns pointer to the page header on the page
void* get_page_header(void* page, uint32_t page_size);

// returns pointer to the end of page header on the page
// i.e. pointer to the start of data section on the page
void* get_end_of_page_header(void* page, uint32_t page_size);

#endif