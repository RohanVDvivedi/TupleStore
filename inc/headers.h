#ifndef PAGE_HDR_H
#define PAGE_HDR_H

#include<stdint.h>
#include<string.h>

// This is the first header at page 0
// this structur is the only data on the first page
// the disk physical page size is assumed to be grater than or equal to the 512 bytes

// each database is stored in a separate file

char* signature = "<- Simple Tuple Storage Model ->";

#define SIGNATURE_LENGTH 32
#define DATABASE_NAME_LENGTH 16

typedef struct file_hdr file_hdr;
struct file_hdr
{
	char signature[SIGNATURE_LENGTH];				// simple signature to identify if it is a SimpleTupleStorageModel
													// Quite dope 

	char database_name[DATABASE_NAME_LENGTH];		// name of the database

	// root pages of various major system tables
	uint32_t tables_list_root_page_id;				// root page to start reading list of tables from
	uint32_t columns_list_root_page_id;				// root page to start reading list of all columns from
	uint32_t indices_list_root_page_id;				// root page to start reading list of all indices from
	uint32_t columns_indices_list_root_page_id;		// root page to start reading list of all columns indices mapping from

	// place to pick the free page from
	uint32_t free_pages_list_root_page_id;			// root page to reach first page of all free pages linkedlist

	uint32_t page_size_in_bytes;					// total size of each page in the database, must be multiple of 512
	uint32_t total_number_of_pages_used;			// total number of pages used by the database file
													// this number is only incremented and stored, when there are not free pages and a new page is requested by the system
};

typedef struct page_hdr page_hdr;
struct page_hdr
{
	// this is the type of the page
	uint8_t page_type;

	uint32_t overflow_page_id;

	uint32_t next_page_id;

	// size of tuple in bytes
	uint16_t tuple_size_in_bytes;

	// number of tuples contained in the page
	uint16_t tuple_count_in_page;
};

void intialize_file_header(file_hdr* hdr, char* database_name, uint32_t page_size_in_bytes);

#endif