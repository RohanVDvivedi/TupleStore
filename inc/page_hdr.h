#ifndef PAGE_HDR_H
#define PAGE_HDR_H

typedef struct page_hdr page_hdr;
struct page_hdr
{
	// this is the type of the page
	uint8_t page_type;

	uint32_t precedent_page_id;
	uint32_t subsequent_page_id;

	uint32_t overflow_page_id;

	// size of tuple in bytes
	uint16_t size_of_tuple_in_bytes;

	// number of tuples contained in the page
	uint16_t tuple_count_in_page;
};// __attribute__((packed));

#endif