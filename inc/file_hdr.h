#ifndef DATABASE_HEADER_H
#define DATABASE_HEADER_H

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
	char signature[SIGNATURE_LENGTH];			// simple signature to identify if it is a SimpleTupleStorageModel
													// Quite dope 

	char database_name[DATABASE_NAME_LENGTH];		// name of the database

	uint32_t tables_list_root_page_id;				// root page to start reading list of tables from
	uint32_t columns_list_root_page_id;				// root page to start reading list of all columns from
	uint32_t indices_list_root_page_id;				// root page to start reading list of all indices from
	uint32_t columns_indices_list_root_page_id;		// root page to start reading list of all columns indices mapping from

	uint32_t free_pages_list_root_page_id;			// root page to reach first page of all free pages linkedlist

	uint32_t page_size_in_bytes;					// total size of each page in the database, must be multiple of 512
	uint32_t total_number_of_pages_used;			// total number of pages used by the database file
													// this number is only incremented and stored, when there are not free pages and a new page is requested by the system
};

void intialize_file_header(file_hdr* hdr, char* database_name, uint32_t page_size_in_bytes);

#endif