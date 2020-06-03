#ifndef PAGE_HDR_H
#define PAGE_HDR_H

typedef struct page_hdr page_hdr;
struct page_hdr
{
	// this is the type of the page
	uint8_t page_type;

	// this is the tuple type that is stored in the page
	uint16_t tuple_type;

	// you must need a doubly linkedlist of page_ids, to eventually read all (datapages or index pages in the same row) pages sequentially
	uint32_t precedent_page_id;
	uint32_t subsequent_page_id;

	// all the tuples in the page, are ordered by its first n fields
	uint8_t first_n_fields_order_by;

	// size of tuple in bytes
	uint16_t size_of_tuple_in_bytes;

	// number of tuples contained in the page
	uint16_t tuple_count_in_page;
} __attribute__((packed));

#endif