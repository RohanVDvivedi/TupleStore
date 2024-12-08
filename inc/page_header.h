#ifndef PAGE_HEADER_H
#define PAGE_HEADER_H

#include<stdint.h>

#include<page_header_util.h>

// this is the total number of bytes that are required on the page (of given page_size) to store the page header (of given page_header_size)
// this space includes the page_header_size and its prefix that stores the size of the page_header
uint32_t get_space_required_for_page_header(uint32_t page_header_size, uint32_t page_size);

// it just returns page_size >= get_space_required_for_page_header(page_header_size, page_size)
int can_page_header_fit_on_page(uint32_t page_header_size, uint32_t page_size);

// returns size of page header as stored on the page
uint32_t get_page_header_size(const void* page, uint32_t page_size);

// returns pointer to the page header on the page
void* get_page_header(void* page, uint32_t page_size);

// same as above method, but returns the read only pointer to the page_header on the page
const void* get_page_header_ua(const void* page, uint32_t page_size);

// initializes page_header in the page to have page_header size
// page_header contents are uninitialized
int init_page_header(void* page, uint32_t page_size, uint32_t page_header_size);

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