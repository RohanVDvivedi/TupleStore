#ifndef DATABASE_HEADER_H
#define DATABASE_HEADER_H

#include<stdint.h>

// This is the first header at page 0
// this structur is the only data on the first page
// the disk physical page size is assumed to be grater than or equal to the 512 bytes

// each database is stored in a separate file

char* database_header_signature = "<- Simple Tuple Storage Model ->";

typedef struct struct database_hdr;
struct database_hdr
{
	char signature[32];								// simple signale to identify if it is a SimpleTupleStorageModel
													// Quite dope 

	char database_name[16];							// name of the database

	uint32_t tables_list_root_page_id;				// root page to start reading list of tables from
	uint32_t columns_list_root_page_id;				// root page to start reading list of all columns from
	uint32_t indices_list_root_page_id;				// root page to start reading list of all indices from
	uint32_t columns_indices_list_root_page_id;		// root page to start reading list of all columns indices mapping from

	uint32_t free_pages_list_root_page_id;			// root page to reach first page of all free pages linkedlist

	uint32_t page_size_in_bytes;					// total size of each page in the database, must be multiple of 512
	uint32_t total_number_of_pages_used;			// total number of pages used by the database file
													// this number is only incremented and stored, when there are not free pages and a new page is requested by the system
}

#endif