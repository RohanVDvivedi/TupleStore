#ifndef PAGE_DEFINITON_H
#define PAGE_DEFINITON_H

#include<tuple_definition.h>

typedef struct page_definition page_definition;
struct page_definition
{
	// total size of page in bytes
	// must always be multiple of 512 (prefer 4096)
	unsigned int size_in_bytes;

	// the definition of each tuple in the page
	tuple_definition data_definition;
};

#endif