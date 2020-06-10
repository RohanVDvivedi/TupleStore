#ifndef PAGE_CONTEXT_H
#define PAGE_CONTEXT_H

#include<string.h>

#include<headers.h>
#include<data_access_methods.h>

typedef struct page_context page_context;
struct page_context
{
	uint32_t page_id;				// page_id corresponding to the page currently being read
	page_hdr* header;				// this effectively the pointer to the page itself
	const data_access_methods* dam;	// this allows the page context to change pages on its own
};

void* get_page(page_context* pg_cntxt);

// retuns pointer to tuple at index tuple_no in the page
void* get_tuple(page_context* pg_cntxt, uint16_t tuple_no);

// returns the number of tuples that were appended
uint16_t append_tuple(page_context* pg_cntxt, void* tuples_to_insert, uint16_t num_tuples_to_insert);

#endif