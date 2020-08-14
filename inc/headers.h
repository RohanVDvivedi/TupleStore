#ifndef PAGE_HDR_H
#define PAGE_HDR_H

#include<tuple_def.h>

// This is the first header at page 0
// this structur is the only data on the first page
// the disk physical page size is assumed to be grater than or equal to the 512 bytes

// each database is stored in a separate file

extern char* signature;

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

// two page layouts are supported
// TUPLE_ARRAY  => array of fixed sized tuples after the header
// SLOTTED_PAGE => tuples are addressed using array of offsets of tuples in the page, after the main header
typedef enum page_layout page_layout;
enum page_layout
{
	TUPLE_ARRAY,	// to be used only when your tuples are fixed length
	SLOTTED_PAGE	// to be used when your tuples are variable length
};

typedef struct page_hdr page_hdr;
struct page_hdr
{
	// this is the type of the page
	u8 page_type;
	
	u4 next_page_id;

	u4 overflow_page_id;

	// defines how the tuples are layed on the page
	page_layout layout;
};

void init_file_header(void* page, char* database_name, uint32_t page_size_in_bytes);

void init_page_header(void* page, u8 page_type, page_layout layout);

#endif