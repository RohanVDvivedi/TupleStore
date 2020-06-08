#ifndef PAGE_HDR_H
#define PAGE_HDR_H

#include<stdint.h>

typedef struct page_hdr page_hdr;
struct page_hdr
{
	// this is the type of the page
	uint8_t page_type;

	uint32_t overflow_page_id;

	// size of tuple in bytes
	uint16_t tuple_size_in_bytes;

	// number of tuples contained in the page
	uint16_t tuple_count_in_page;
};

#endif