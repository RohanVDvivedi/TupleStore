#ifndef PAGE_HEADER_H
#define PAGE_HEADER_H

#include<stdint.h>

// returns size of page header as stored on the page
uint32_t get_page_header_size(void* page, uint32_t page_size);

// returns pointer to the page header on the page
void* get_page_header(void* page, uint32_t page_size);

// initializes page_header in the page to have page_header size
// page_header contents are uninitialized
int init_page_header(void* page, uint32_t page_size, uint32_t page_header_size);

// returns offset to the end of page_header in the given page
uint32_t get_offset_to_end_of_page_header(void* page, uint32_t page_size);

// returns pointer to the end of page header on the page
// i.e. pointer to the start of data section on the page
void* get_end_of_page_header(void* page, uint32_t page_size);

#endif

/*
**	PAGE_HEADER
**
**	equivalent to
**
**	struct page_header
**	{
**		uintN_t page_header_size;
**
**		char page_header[ page_header_size ];
**	}
**
**	here N can be 8, 16 or 32
*/