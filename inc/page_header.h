#ifndef PAGE_DEFINITON_H
#define PAGE_DEFINITON_H

#include<cell_definition.h>

typedef struct page_header page_header;
struct page_header
{
	unsigned char page_type;

	unsigned int row_count;

	unsigned int precedent_page_id;

	unsigned int subsequent_page_id;
};

#endif